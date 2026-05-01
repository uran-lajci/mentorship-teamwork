#!/bin/bash
# ============================================================
# chmod +x run_all.sh
# ./run_all.sh
# Compiles all solvers (patching NS to fit all instances),
# runs each solver ONCE per instance, saves every solution,
# and produces results.csv.
# Time limit per run: 10 minutes. Timed-out runs score 0.
# ============================================================

set -euo pipefail

TIME_LIMIT=600   # seconds (10 minutes)

# ── Paths ────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTANCES_DIR="$SCRIPT_DIR/instances"
SOLUTIONS_DIR="$SCRIPT_DIR/solutions"
SOLVERS_DIR="$SCRIPT_DIR/solvers"
BINS_DIR="$SCRIPT_DIR/bins"
PATCHED_DIR="$SCRIPT_DIR/bins/patched_src"
CSV="$SCRIPT_DIR/results.csv"

mkdir -p "$SOLUTIONS_DIR" "$BINS_DIR" "$PATCHED_DIR"

# ── Detect max skill count across all instances ───────────────
echo "=== Scanning instances for skill count ==="
MAX_SKILLS=0
for f in "$INSTANCES_DIR"/*.in; do
    cnt=$(awk 'NR>1 && /[A-Za-z]/{for(i=1;i<=NF;i++) if($i~/^[A-Za-z]/) skills[$i]=1} END{print length(skills)}' "$f")
    echo "  $(basename "$f"): $cnt distinct skills"
    if (( cnt > MAX_SKILLS )); then MAX_SKILLS=$cnt; fi
done
NS_SAFE=$(( (MAX_SKILLS * 12 / 10 + 99) / 100 * 100 ))
echo "  → Using NS = $NS_SAFE for all solvers"

# ── Compile ───────────────────────────────────────────────────
echo ""
echo "=== Compiling solvers (patching NS=$NS_SAFE) ==="
for solver in ab c d e f; do
    src="$SOLVERS_DIR/$solver.cpp"
    patched="$PATCHED_DIR/$solver.cpp"
    sed \
        -e "s/const int NS = [0-9]*/const int NS = $NS_SAFE/" \
        -e "s/array<int, [0-9][0-9]*>/array<int, $NS_SAFE>/g" \
        "$src" > "$patched"
    echo -n "  Compiling $solver ... "
    g++ -O2 -std=c++17 -o "$BINS_DIR/$solver" "$patched"
    echo "done"
done

# ── Helpers ───────────────────────────────────────────────────

extract_score() {
    grep -E '^[0-9]+( [0-9]+)?$' "$1" 2>/dev/null | tail -1 | awk '{print $1}'
}

run_with_timeout() {
    local bin="$1" inp="$2" out="$3" err="$4" tlimit="$5"
    timeout "$tlimit" bash -c '"$0" < "$1" > "$2" 2> "$3"' \
        "$bin" "$inp" "$out" "$err"
}

# run_solver_once <solver> <inst_base> <inp>
# Sets global SC to the score (0 on timeout).
run_solver_once() {
    local solver="$1" inst_base="$2" inp="$3"
    local out="$SOLUTIONS_DIR/${inst_base}__${solver}.out"
    local err="$SOLUTIONS_DIR/${inst_base}__${solver}.err"
    local start=$SECONDS

    echo "  [$solver] running ..."
    if run_with_timeout "$BINS_DIR/$solver" "$inp" "$out" "$err" "$TIME_LIMIT"; then
        local elapsed=$(( SECONDS - start ))
        SC="$(extract_score "$err")"
        SC="${SC:-0}"
        echo "    score = $SC  (${elapsed}s)"
    else
        local elapsed=$(( SECONDS - start ))
        echo "    TIMEOUT after ${elapsed}s — score = 0"
        SC=0
    fi
}

# ── Collect instance names ────────────────────────────────────
mapfile -t INSTANCE_FILES < <(ls "$INSTANCES_DIR"/*.in 2>/dev/null | sort)
if [[ ${#INSTANCE_FILES[@]} -eq 0 ]]; then
    echo "ERROR: No .in files found in $INSTANCES_DIR"
    exit 1
fi

declare -A INSTANCE_NAMES
for f in "${INSTANCE_FILES[@]}"; do
    base="$(basename "$f" .in)"
    INSTANCE_NAMES["$base"]="$f"
done

# ── Main loop ─────────────────────────────────────────────────
declare -A BEST_SCORE
SC=0

for inst_base in $(echo "${!INSTANCE_NAMES[@]}" | tr ' ' '\n' | sort); do
    inp="${INSTANCE_NAMES[$inst_base]}"
    echo ""
    echo "=== Instance: $inst_base ==="

    for solver in ab c d e f; do
        run_solver_once "$solver" "$inst_base" "$inp"
        BEST_SCORE["$inst_base:$solver"]="$SC"
    done
done

# ── Write CSV ─────────────────────────────────────────────────
echo ""
echo "=== Writing $CSV ==="

{
    printf "Instance\tNanored B\tNanored C\tNanored D\tNanored E\tNanored F\n"
    for inst_base in $(echo "${!INSTANCE_NAMES[@]}" | tr ' ' '\n' | sort); do
        printf "%s\t%s\t%s\t%s\t%s\t%s\n" \
            "$inst_base" \
            "${BEST_SCORE[$inst_base:ab]:-0}" \
            "${BEST_SCORE[$inst_base:c]:-0}"  \
            "${BEST_SCORE[$inst_base:d]:-0}"  \
            "${BEST_SCORE[$inst_base:e]:-0}"  \
            "${BEST_SCORE[$inst_base:f]:-0}"
    done
} > "$CSV"

echo ""
echo "=== Done! ==="
echo ""
echo "Solution files : $SOLUTIONS_DIR/"
echo "Results CSV    : $CSV"
echo ""
column -t -s $'\t' "$CSV"
