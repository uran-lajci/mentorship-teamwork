// Copyright (C) 2026, Coudert--Osmont Yoann
// SPDX-License-Identifier: MIT
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace std;
const int NS = 800;

int main()
{
	ios::sync_with_stdio(false);
	cin.tie(nullptr);

	int C, P;
	cin >> C >> P;
	vector<string> Cnames(C), Pnames(P);
	unordered_map<string, int> s2i;
	vector<array<int, NS>> skill(C);
	vector<int> ts(C, 0);
	vector<vector<pair<int, int>>> req(P);
	vector<int> D(P), S(P), B(P);
	array<vector<int>, NS> s2c;
	vector<vector<int>> c2s(C);

	for (int c = 0; c < C; ++c)
	{
		int n;
		cin >> Cnames[c] >> n;
		skill[c].fill(0);
		while (n--)
		{
			string s;
			int l;
			cin >> s >> l;
			if (!s2i.count(s))
				s2i[s] = s2i.size();
			skill[c][s2i[s]] = l;
			ts[c] += l;
			s2c[s2i[s]].push_back(c);
			c2s[c].push_back(s2i[s]);
		}
	}
	vector<bool> sameR(P);
	for (int p = 0; p < P; ++p)
	{
		int r;
		cin >> Pnames[p] >> D[p] >> S[p] >> B[p] >> r;
		while (r--)
		{
			string s;
			int l;
			cin >> s >> l;
			if (!s2i.count(s))
				s2i[s] = s2i.size();
			req[p].emplace_back(s2i[s], l);
		}
		set<pair<int, int>> sr(req[p].begin(), req[p].end());
		sameR[p] = sr.size() == 1;
	}
	cerr << C << ' ' << P << ' ' << s2i.size() << ' ' << sameR.size() << endl;

	int score = 0;
	vector<pair<int, vector<int>>> sol;

	vector<int> ps(P);
	vector<bool> chosen(C, false);
	vector<int> av(C, 0), subR(C);
	iota(ps.begin(), ps.end(), 0);
	const auto ckey = [&](int c, int m, int s, int l)
	{ return make_tuple(max(av[c], m), -min(av[c], m), skill[c][s], ts[c]); };
	vector<array<int, NS>> skill0 = skill;
	while (true)
	{
		for (int s = 0; s < NS; ++s)
			sort(s2c[s].begin(), s2c[s].end(), [&](int i, int j)
				 { return make_tuple(av[i], skill[i][s], ts[i]) < make_tuple(av[j], skill[j][s], ts[j]); });
		int bestP = -1, bestEnd = -1, bestLvlUp = -1;
		double bestScore = -1.;
		vector<int> bestCS;
		sort(ps.begin(), ps.end(), [&](int i, int j)
			 { return D[i] < D[j] || (D[i] == D[j] && S[i] > S[j]); });
		for (int p : ps)
		{
			vector<int> cs;
			int sav = 0, mav = 0;
			if (sameR[p])
			{
				const auto [s, l] = req[p][0];
				const int r = req[p].size();
				int N = 0;
				for (int c : s2c[s])
					if (skill[c][s] >= l - 1)
						subR[N++] = c;
				if (N < r)
					continue;
				int lastL = -1, i = 0, bestI = -1, bestT = -1;
				double best = -1.;
				int cntLvlUp = 0;
				int bestCntLvlUp = -1;
				for (; i < r - 1; ++i)
				{
					sav += av[subR[i]];
					if (skill[subR[i]][s] >= l)
						lastL = i;
					if (skill[subR[i]][s] <= l)
						++cntLvlUp;
				}
				for (; i < N; ++i)
				{
					sav += av[subR[i]];
					if (skill[subR[i]][s] >= l)
						lastL = i;
					if (skill[subR[i]][s] <= l)
						++cntLvlUp;
					if (i - lastL < r)
					{
						int t = r * av[subR[i]] - sav;
						int end = av[subR[i]] + D[p];
						int sco = max(0, S[p] - max(0, end - B[p]));
						double sc = (sco ? double(sco) / double(t) : double(cntLvlUp) / double(t));
						if (sc > best || (sc == best && cntLvlUp > bestCntLvlUp))
						{
							best = sc;
							bestI = i;
							bestT = t;
							bestCntLvlUp = cntLvlUp;
						}
					}
					int out = subR[i - r + 1];
					sav -= av[out];
					if (skill[out][s] <= l)
						--cntLvlUp;
				}
				if (bestI == -1)
					continue;
				cs.assign(subR.begin() + bestI - r + 1, subR.begin() + bestI + 1);
				mav = av[subR[bestI]];
				sav = r * mav - bestT;
			}
			else
			{
				const int R = req[p].size();
				vector<int> order(R);
				iota(order.begin(), order.end(), 0);
				sort(order.begin(), order.end(), [&](int i, int j)
					 {
					if (req[p][i].second != req[p][j].second)
						return req[p][i].second > req[p][j].second;
					return i < j; });
				array<int, NS> ms;
				ms.fill(0);
				vector<int> assign(R, -1);
				vector<int> assigned;
				bool ok = true;
				for (int idx : order)
				{
					auto [s, l] = req[p][idx];
					const int l0 = l;
					if (ms[s] >= l)
						--l;
					int best = -1;
					if (l)
						for (int c : s2c[s])
							if (!chosen[c] && skill[c][s] >= l)
								if (best == -1 || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
									best = c;
					if (!l)
						for (int c = 0; c < C; ++c)
							if (!chosen[c])
								if (best == -1 || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
									best = c;
					if (best == -1)
					{
						ok = false;
						break;
					}
					assign[idx] = best;
					assigned.push_back(best);
					for (int s : c2s[best])
						ms[s] = max(ms[s], skill[best][s]);
					chosen[best] = true;
					sav += av[best];
					mav = max(mav, av[best]);
				}
				if (!ok)
				{
					for (int c : assigned)
						chosen[c] = false;
					continue;
				}
				cs.resize(R);
				for (int i = 0; i < R; ++i)
					cs[i] = assign[i];
				for (int c : cs)
					chosen[c] = false;
			}
			int end = mav + D[p];
			double time = cs.size() * end - sav;
			int sco = max(0, S[p] - max(0, end - B[p]));
			double score;
			int nblvlup;
			if (!sco)
			{
				for (int i = 1; i < cs.size(); ++i)
					if (skill[cs[i]][req[p][i].first] > req[p][i].second)
						for (int j = 0; j < i; ++j)
							if (skill[cs[j]][req[p][j].first] > req[p][j].second)
								if (skill[cs[i]][req[p][j].first] >= req[p][j].second - 1 && skill[cs[j]][req[p][i].first] >= req[p][i].second - 1)
									if (skill[cs[i]][req[p][j].first] <= req[p][j].second || skill[cs[j]][req[p][i].first] <= req[p][i].second)
									{
										swap(cs[i], cs[j]);
										if (skill[cs[i]][req[p][i].first] <= req[p][i].second)
											break;
										else
											j = -1;
									}
				nblvlup = 0;
				double weightedLvl = 0;
				for (int i = 0; i < cs.size(); ++i)
					if (skill[cs[i]][req[p][i].first] <= req[p][i].second)
					{
						++nblvlup;
						weightedLvl += req[p][i].second;
					}
				score = weightedLvl / time;
			}
			else
			{
				nblvlup = 0;
				for (int i = 0; i < cs.size(); ++i)
					if (skill[cs[i]][req[p][i].first] <= req[p][i].second)
						++nblvlup;
				score = double(sco) / time;
			}
			if (score > bestScore || (score == bestScore && end < bestEnd) || (score == bestScore && end == bestEnd && nblvlup > bestLvlUp))
			{
				bestScore = score;
				bestEnd = end;
				bestLvlUp = nblvlup;
				bestCS = move(cs);
				bestP = p;
			}
		}
		if (bestP == -1)
			break;
		score += max(0, S[bestP] - max(0, bestEnd - B[bestP]));
		cerr << score << endl;
		for (int i = 1; i < bestCS.size(); ++i)
			if (skill[bestCS[i]][req[bestP][i].first] > req[bestP][i].second)
				for (int j = 0; j < i; ++j)
					if (skill[bestCS[j]][req[bestP][j].first] > req[bestP][j].second)
						if (skill[bestCS[i]][req[bestP][j].first] >= req[bestP][j].second - 1 && skill[bestCS[j]][req[bestP][i].first] >= req[bestP][i].second - 1)
							if (skill[bestCS[i]][req[bestP][j].first] <= req[bestP][j].second || skill[bestCS[j]][req[bestP][i].first] <= req[bestP][i].second)
							{
								swap(bestCS[i], bestCS[j]);
								if (skill[bestCS[i]][req[bestP][i].first] <= req[bestP][i].second)
									break;
								else
									j = -1;
							}
		for (int i = 0; i < bestCS.size(); ++i)
		{
			av[bestCS[i]] = bestEnd;
			auto [s, l] = req[bestP][i];
			if (skill[bestCS[i]][s] <= l)
			{
				if (!skill[bestCS[i]][s])
				{
					s2c[s].push_back(bestCS[i]);
					c2s[bestCS[i]].push_back(s);
				}
				++skill[bestCS[i]][s];
				++ts[bestCS[i]];
			}
		}
		sol.emplace_back(bestP, bestCS);
		int i = 0;
		while (ps[i] != bestP)
			++i;
		ps[i] = ps.back();
		ps.pop_back();
	}

	// Post-optimization: 2-opt swaps with hill climbing
	{
		auto simulate = [&](const vector<pair<int, vector<int>>> &seq) -> int
		{
			vector<int> av2(C, 0);
			vector<array<int, NS>> skill2 = skill0;
			int total = 0;
			for (const auto &[p, cs] : seq)
			{
				int start = 0;
				for (int c : cs)
					start = max(start, av2[c]);
				// Check validity
				for (int i = 0; i < cs.size(); ++i)
				{
					auto [s, l] = req[p][i];
					int c = cs[i];
					if (skill2[c][s] >= l)
						continue;
					if (skill2[c][s] == l - 1)
					{
						bool hasMentor = false;
						for (int j = 0; j < cs.size(); ++j)
						{
							if (j != i && skill2[cs[j]][s] >= l)
							{
								hasMentor = true;
								break;
							}
						}
						if (!hasMentor)
							return -1;
					}
					else
						return -1;
				}
				int end = start + D[p];
				for (int c : cs)
					av2[c] = end;
				int sco = max(0, S[p] - max(0, end - B[p]));
				total += sco;
				for (int i = 0; i < cs.size(); ++i)
				{
					auto [s, l] = req[p][i];
					if (skill2[cs[i]][s] <= l)
					{
						++skill2[cs[i]][s];
					}
				}
			}
			return total;
		};
		int bestTotal = score;
		bool improved = true;
		const int MAX_IMPROVEMENTS = 100;
		int improvements = 0;
		while (improved && improvements < MAX_IMPROVEMENTS)
		{
			improved = false;
			for (int i = 0; i < (int)sol.size(); ++i)
			{
				for (int j = i + 1; j < (int)sol.size(); ++j)
				{
					swap(sol[i], sol[j]);
					int newScore = simulate(sol);
					if (newScore > bestTotal)
					{
						bestTotal = newScore;
						improved = true;
						++improvements;
						break;
					}
					else
					{
						swap(sol[i], sol[j]);
					}
				}
				if (improved)
					break;
			}
		}
		score = bestTotal;
	}

	cerr << score << endl;
	cout << sol.size() << endl;
	for (const auto &[p, cs] : sol)
	{
		cout << Pnames[p] << '\n';
		for (int c : cs)
			cout << Cnames[c] << ' ';
		cout << '\n';
	}

	return 0;
}
