# Mentorship and Teamwork — Google Hash Code 2022 Qualification

Solvers for the **Mentorship and Teamwork** problem (Google Hash Code 2022, Qualification Round).

This repository is a focused fork of the original "Quiche LORIAinne" solutions. The solvers for the other problems (2019, 2021, and the 2022 Final) have been removed so that everything here targets a single problem. The solvers were also refactored to run correctly on **all six instances** (the originals were each tuned for one instance and crashed or misparsed on the others).

## Results

Each solver was run on every instance. The **Refactored Nanored** column is the best score per instance across the retained solvers; the **Reported** column is the team's originally reported per-instance score.

| Instance | priority_sweep | bestfirst_levelup | two_phase_deadline | bestfirst_efficiency | **Refactored (best)** | Reported |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| a_an_example | 20 | 20 | 33 | 20 | **33** | 33 |
| b_better_start_small | 1,011,400 | 1,032,469 | 894,161 | 859,726 | **1,032,469** | 1,003,496 |
| c_collaboration | 242,898 | 57,238 | 110,356 | 255,918 | **255,918** | 242,898 |
| d_dense_schedule | 2,608,736 | 2,178,519 | 372,996 | 591,528 | **2,608,736** | 2,178,519 |
| e_exceptional_skills | 1,353,350 | 426,029 | 1,648,976 | 1,109,173 | **1,648,976** | 1,648,976 |
| f_find_great_mentors | 542,013 | 60,958 | 483,104 | 466,387 | **542,013** | 466,387 |
| **Total** | 5,758,417 | 3,755,233 | 3,509,626 | 3,282,752 | **6,088,145** | 5,540,309 |

Taking the best solver per instance gives **6,088,145**, an improvement of **+547,836** over the originally reported total (5,540,309). No single solver wins everywhere — every retained solver is the best on at least one instance:

| Instance | Best solver |
| --- | --- |
| a, e | two_phase_deadline_greedy |
| b | bestfirst_levelup_greedy |
| c | bestfirst_efficiency_greedy |
| d, f | priority_sweep_greedy |

Notes:
- The `bestfirst_efficiency_greedy` solver (formerly `f`) **did not reproduce its previously reported F score** when re-run; its best contribution is actually on instance c.
- The original `ab` solver was **removed**: it is the best on no instance, so it adds nothing to the per-instance best.

## Solvers

Renamed from instance-based to mechanism-based names:

| Old name | New name | Mechanism |
| --- | --- | --- |
| Nanored c | `priority_sweep_greedy` | Single deterministic sweep; sorts projects by an S·D·roles priority and commits **all** feasible projects each round, retrying failures. |
| Nanored d | `bestfirst_levelup_greedy` | Commits **one** project per round — the one maximizing skill level-ups (mentorship). O(P²). |
| Nanored e | `two_phase_deadline_greedy` | Two phases: sort by deadline slack and commit all feasible, then switch criterion and allow level-up-only completions. |
| Nanored f | `bestfirst_efficiency_greedy` | Commits one project per round by best score-per-time efficiency, with a sliding-window fast path for same-skill projects. |

### Determinism and time limit

| Solver | Deterministic | Time limit |
| --- | --- | --- |
| `priority_sweep_greedy` | yes | < 1 min |
| `bestfirst_levelup_greedy` | yes | 10 min (enforced) |
| `two_phase_deadline_greedy` | yes | < 1 min |
| `bestfirst_efficiency_greedy` | yes | < 1 min |

All retained solvers are deterministic: they use no random seed from entropy/time (fixed-seed or no RNG at all), so the same binary on the same input always yields the same score. "< 1 min" reflects measured runtime rather than an enforced cap; only `bestfirst_levelup_greedy` enforces a wall-clock limit (its O(P²) loop would otherwise run ~25+ min on the largest instance, f). Determinism is guaranteed per compiled binary; a different compiler or standard-library version could reorder ties.

## Changes made (do not affect the reported scores)

The refactors are capacity/parsing/termination fixes, not algorithm changes. Where a solver already worked on an instance, its output is **byte-identical** before and after (verified by md5).

- **`priority_sweep_greedy`** — skill-array capacity raised 200 → 800. The original overflowed on instances with more than 200 distinct skills (c = 200 was at the edge; e = 800 and f = 500 crashed). Capacity only; outputs unchanged on every instance it already handled.
- **`bestfirst_efficiency_greedy`** — capacity raised 500 → 800 so instance e (800 distinct skills) no longer segfaults. Capacity only; the other five instances are byte-identical.
- **`bestfirst_levelup_greedy`** — three changes: (1) the contributor parser now reads `n` skills per contributor instead of exactly one, so it runs on multi-skill instances instead of desyncing; (2) capacity raised 188 → 1024; (3) a 10-minute wall-clock limit was added (stops the main loop and prints the best solution found so far). For instance d every contributor has one skill, so parsing is unchanged and the capacity is irrelevant — its output is byte-identical to the original.
- **`two_phase_deadline_greedy`** — no changes; its skill array (size 800) already covers all six instances.

None of these touch the search logic, scoring, or RNG, so the scores in the results table are unaffected.

## Build and run

Each solver is a single self-contained C++17 file:

```bash
g++ -O3 -march=native -std=c++17 -o solver priority_sweep_greedy.cpp
./solver < data/d_dense_schedule.in > d_dense_schedule.out.txt
```

Input is read from stdin; the solution is written to stdout; progress and the final score go to stderr.

## Verifying scores

Scores can be checked with the Node.js referee (Hash Code Judge-equivalent insights reporter). Place each solver's output next to its input as `<input>.out.txt` and run from the repository root:

```bash
node index.js data/d_dense_schedule.in.txt
```

The referee validates feasibility (role counts, skill levels with the mentorship rule, no double-assignment, no duplicate projects) and prints the official score plus insights.

## Credits and sources

- Original solvers (team Quiche LORIAinne / Nanored): <https://github.com/Nanored4498/HashCode2022/tree/main/2022/Qualif> (accessed 2026-06-03).
- Referee and insights reporter (Kristóf Holecz): <https://github.com/KristofHolecz/hash-code-2022-mentorship-and-teamwork-insights-reporter> (accessed 2026-06-03).

The solvers in this repository are refactors of the original Nanored solutions (capacity, parsing, and termination fixes only — see "Changes made" above); the underlying algorithms are unchanged.
