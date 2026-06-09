// Copyright (C) 2026, Coudert--Osmont Yoann
// SPDX-License-Identifier: MIT
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace std;
const int NS = 1024;

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
		skill[c].fill(0);
		cin >> Cnames[c] >> n;
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
	cerr << C << ' ' << P << ' ' << s2i.size() << endl;
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
	}

	int score = 0;
	if ((int)s2i.size() > NS)
	{
		cerr << "ERROR: distinct skills (" << s2i.size() << ") exceeds NS=" << NS
			 << "; increase NS and recompile." << endl;
		return 1;
	}
	vector<pair<int, vector<int>>> sol;

	vector<int> av(C, 0);
	unordered_set<int> ps;
	vector<bool> chosen(C, false);
	for (int p = 0; p < P; ++p)
		ps.insert(p);
	mt19937 mt(42);
	const auto t0 = chrono::steady_clock::now();
	const auto ckey = [&](int c, int m, int s, int l)
	{ return make_tuple(max(av[c], m), -skill[c][s], -(int)c2s[c].size()); };
	auto skill_init = skill;
	auto ts_init = ts;
	while (true)
	{
		int bestP = -1, bestlvlup = -1, bestEnd = -1;
		vector<int> bestCS;
		for (int p : ps)
		{
			if (chrono::duration<double>(chrono::steady_clock::now() - t0).count() >= 600.0)
			{
				bestP = -1;
				break;
			}
			int beste = -1, besto = -1;
			vector<int> cs(req[p].size(), 0), cs2;
			vector<int> ro(req[p].size());
			array<int, NS> ms;
			iota(ro.begin(), ro.end(), 0);
			for (int test = 0; test < 50; ++test)
			{
				shuffle(ro.begin(), ro.end(), mt);
				ms.fill(0);
				int mav = 0, oo = 0, nr = req[p].size();
				for (int o : ro)
				{
					auto [s, l] = req[p][o];
					const int l0 = l;
					if (ms[s] >= l)
						--l;
					int best = -1;
					if (l)
						for (int c : s2c[s])
							if (!chosen[c] && skill[c][s] >= l)
								if (best == -1 || (skill[c][s] <= l0 && skill[best][s] > l0) || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
									best = c;
					if (!l)
						for (int c = 0; c < C; ++c)
							if (!chosen[c])
								if (best == -1 || (skill[c][s] <= l0 && skill[best][s] > l0) || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
									best = c;
					if (best == -1)
						break;
					--nr;
					cs[o] = best;
					if (skill[best][s] <= l0)
						++oo;
					for (int s : c2s[best])
						ms[s] = max(ms[s], skill[best][s]);
					chosen[best] = true;
					mav = max(mav, av[best]);
				}
				for (int c : cs)
					chosen[c] = false;
				if (nr)
					continue;
				for (int i = 1; i < cs.size(); ++i)
					if (skill[cs[i]][req[p][i].first] > req[p][i].second)
						for (int j = 0; j < i; ++j)
							if (skill[cs[j]][req[p][j].first] > req[p][j].second)
								if (skill[cs[i]][req[p][j].first] >= req[p][j].second - 1 && skill[cs[j]][req[p][i].first] >= req[p][i].second - 1)
								{
									swap(cs[i], cs[j]);
									oo += 2;
								}
				if (oo > besto)
				{
					besto = oo;
					beste = mav + D[p];
					cs2 = cs;
				}
			}
			if (cs2.empty() && chrono::duration<double>(chrono::steady_clock::now() - t0).count() < 590.0)
			{
				// extra attempts for projects not yet feasible
				for (int test = 50; test < 200; ++test)
				{
					shuffle(ro.begin(), ro.end(), mt);
					ms.fill(0);
					int mav = 0, oo = 0, nr = req[p].size();
					for (int o : ro)
					{
						auto [s, l] = req[p][o];
						const int l0 = l;
						if (ms[s] >= l)
							--l;
						int best = -1;
						if (l)
							for (int c : s2c[s])
								if (!chosen[c] && skill[c][s] >= l)
									if (best == -1 || (skill[c][s] <= l0 && skill[best][s] > l0) || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
										best = c;
						if (!l)
							for (int c = 0; c < C; ++c)
								if (!chosen[c])
									if (best == -1 || (skill[c][s] <= l0 && skill[best][s] > l0) || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
										best = c;
						if (best == -1)
							break;
						--nr;
						cs[o] = best;
						if (skill[best][s] <= l0)
							++oo;
						for (int s : c2s[best])
							ms[s] = max(ms[s], skill[best][s]);
						chosen[best] = true;
						mav = max(mav, av[best]);
					}
					for (int c : cs)
						chosen[c] = false;
					if (nr)
						continue;
					if (oo > besto)
					{
						besto = oo;
						beste = mav + D[p];
						cs2 = cs;
					}
					if (chrono::duration<double>(chrono::steady_clock::now() - t0).count() >= 595.0)
						break;
				}
			}
			if (cs2.empty())
				continue;
			int lvlup = 0;
			for (int i = 0; i < cs2.size(); ++i)
				if (skill[cs2[i]][req[p][i].first] <= req[p][i].second)
					lvlup += 1 + skill[cs2[i]][req[p][i].first];
			if (lvlup > bestlvlup || (lvlup == bestlvlup && bestP != -1 && beste < bestEnd))
			{
				bestlvlup = lvlup;
				bestP = p;
				bestCS = move(cs2);
				bestEnd = beste;
			}
		}
		if (bestP == -1)
			break;
		score += max(0, S[bestP] - max(0, bestEnd - B[bestP]));
		cerr << score << endl;
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
		ps.erase(bestP);
	}

	// post-scheduling local search with adjacent swaps
	auto simulate = [&](const vector<pair<int, vector<int>>> &seq) -> int
	{
		vector<int> av_sim(C, 0);
		auto cur_skill = skill_init;
		int total = 0;
		for (const auto &[p, cs] : seq)
		{
			int s = 0;
			for (int c : cs)
				s = max(s, av_sim[c]);
			for (int i = 0; i < (int)cs.size(); ++i)
			{
				auto [sk, lvl] = req[p][i];
				int clvl = cur_skill[cs[i]][sk];
				if (clvl >= lvl)
					continue;
				if (clvl == lvl - 1)
				{
					bool mentored = false;
					for (int j = 0; j < (int)cs.size(); ++j)
						if (i != j && cur_skill[cs[j]][sk] >= lvl)
						{
							mentored = true;
							break;
						}
					if (mentored)
						continue;
				}
				return -1;
			}
			int end = s + D[p];
			total += max(0, S[p] - max(0, end - B[p]));
			for (int i = 0; i < (int)cs.size(); ++i)
			{
				av_sim[cs[i]] = end;
				auto [sk, lvl] = req[p][i];
				if (cur_skill[cs[i]][sk] <= lvl)
					cur_skill[cs[i]][sk]++;
			}
		}
		return total;
	};
	auto t_now = chrono::steady_clock::now();
	double elapsed = chrono::duration<double>(t_now - t0).count();
	bool changed = true;
	while (changed && elapsed < 595.0)
	{
		changed = false;
		for (int i = 0; i < (int)sol.size() - 1; ++i)
		{
			swap(sol[i], sol[i + 1]);
			int new_score = simulate(sol);
			if (new_score > score)
			{
				score = new_score;
				changed = true;
				break;
			}
			else
			{
				swap(sol[i], sol[i + 1]);
			}
			t_now = chrono::steady_clock::now();
			elapsed = chrono::duration<double>(t_now - t0).count();
			if (elapsed >= 595.0)
				break;
		}
	}

	// 2-swap hill-climbing
	changed = true;
	while (changed && elapsed < 595.0)
	{
		changed = false;
		for (int i = 0; i < (int)sol.size(); ++i)
		{
			for (int j = i + 1; j < (int)sol.size(); ++j)
			{
				swap(sol[i], sol[j]);
				int new_score = simulate(sol);
				if (new_score > score)
				{
					score = new_score;
					changed = true;
					goto swap_done;
				}
				else
				{
					swap(sol[i], sol[j]);
				}
				if (chrono::duration<double>(chrono::steady_clock::now() - t0).count() >= 595.0)
					break;
			}
			if (chrono::duration<double>(chrono::steady_clock::now() - t0).count() >= 595.0)
				break;
		}
	swap_done:;
	}

	// Attempt to schedule remaining projects at the end
	vector<int> remaining(ps.begin(), ps.end());
	sort(remaining.begin(), remaining.end());
	bool added = true;
	while (added && !remaining.empty())
	{
		added = false;
		for (auto it = remaining.begin(); it != remaining.end();)
		{
			int p = *it;
			vector<int> cs(req[p].size(), 0), cs2;
			vector<int> ro(req[p].size());
			iota(ro.begin(), ro.end(), 0);
			array<int, NS> ms;
			int besto = -1, beste = -1;
			for (int test = 0; test < 50; ++test)
			{
				shuffle(ro.begin(), ro.end(), mt);
				ms.fill(0);
				int mav = 0, oo = 0;
				vector<int> tcs(req[p].size());
				vector<bool> chosen_local(C, false);
				bool ok = true;
				for (int o : ro)
				{
					auto [s, l] = req[p][o];
					const int l0 = l;
					if (ms[s] >= l)
						--l;
					int best = -1;
					if (l)
						for (int c : s2c[s])
							if (!chosen_local[c] && skill[c][s] >= l)
								if (best == -1 || (skill[c][s] <= l0 && skill[best][s] > l0) || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
									best = c;
					if (!l)
						for (int c = 0; c < C; ++c)
							if (!chosen_local[c])
								if (best == -1 || (skill[c][s] <= l0 && skill[best][s] > l0) || ckey(c, mav, s, l0) < ckey(best, mav, s, l0))
									best = c;
					if (best == -1)
					{
						ok = false;
						break;
					}
					tcs[o] = best;
					if (skill[best][s] <= l0)
						++oo;
					for (int s2 : c2s[best])
						ms[s2] = max(ms[s2], skill[best][s2]);
					chosen_local[best] = true;
					mav = max(mav, av[best]);
				}
				if (!ok)
					continue;
				if (oo > besto)
				{
					besto = oo;
					beste = mav + D[p];
					cs2 = tcs;
				}
			}
			if (cs2.empty())
			{
				++it;
				continue;
			}
			int projScore = max(0, S[p] - max(0, beste - B[p]));
			if (projScore > 0)
			{
				score += projScore;
				for (int i = 0; i < (int)cs2.size(); ++i)
				{
					av[cs2[i]] = beste;
					auto [s, l] = req[p][i];
					if (skill[cs2[i]][s] <= l)
					{
						if (!skill[cs2[i]][s])
						{
							s2c[s].push_back(cs2[i]);
							c2s[cs2[i]].push_back(s);
						}
						++skill[cs2[i]][s];
					}
				}
				sol.emplace_back(p, cs2);
				it = remaining.erase(it);
				added = true;
			}
			else
			{
				++it;
			}
		}
	}

	// Insertion hill-climbing
	changed = true;
	while (changed && elapsed < 595.0)
	{
		changed = false;
		for (int i = 0; i < (int)sol.size(); ++i)
		{
			auto saved = sol[i];
			sol.erase(sol.begin() + i);
			for (int j = 0; j <= (int)sol.size(); ++j)
			{
				sol.insert(sol.begin() + j, saved);
				int new_score = simulate(sol);
				if (new_score > score)
				{
					score = new_score;
					changed = true;
					goto insert_done;
				}
				else
				{
					sol.erase(sol.begin() + j);
				}
				t_now = chrono::steady_clock::now();
				elapsed = chrono::duration<double>(t_now - t0).count();
				if (elapsed >= 595.0)
					break;
			}
			if (!changed)
			{
				sol.insert(sol.begin() + i, saved);
			}
			else
				break;
			t_now = chrono::steady_clock::now();
			elapsed = chrono::duration<double>(t_now - t0).count();
			if (elapsed >= 595.0)
				break;
		}
	insert_done:;
		t_now = chrono::steady_clock::now();
		elapsed = chrono::duration<double>(t_now - t0).count();
	}

	cerr << ps.size() << endl;
	cout << sol.size() << endl;
	for (const auto &[p, cs] : sol)
	{
		cout << Pnames[p] << '\n';
		for (int c : cs)
			cout << Cnames[c] << ' ';
		cout << '\n';
	}

	cerr << "final score after local search: " << score << endl;
	return 0;
}
