// Copyright (C) 2026, Coudert--Osmont Yoann
// SPDX-License-Identifier: MIT
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;
int main()
{
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	int C, P;
	cin >> C >> P;
	vector<string> Cnames(C), Pnames(P);
	unordered_map<string, int> s2i;
	vector<array<int, 800>> skill(C);
	vector<int> ts(C, 0);
	vector<vector<pair<int, int>>> req(P);
	vector<int> D(P), S(P), B(P);
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
		}
	}
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
	vector<pair<int, vector<int>>> sol;
	int best_score = 0;
	vector<pair<int, vector<int>>> best_sol;
	const int NUM_RUNS = 50;
	vector<int> av(C, 0);
	vector<int> order(P);
	vector<bool> chosen(C, false);
	iota(order.begin(), order.end(), 0);
	mt19937 mt;
	const auto ckey = [&](int c, int m, int s)
	{ return make_tuple(max(av[c], m), skill[c][s], ts[c]); };
	bool need_lvl = false;
	for (int run = 0; run < NUM_RUNS; ++run)
	{
		auto cur_skill = skill;
		auto cur_ts = ts;
		av.assign(C, 0);
		order = vector<int>(P);
		iota(order.begin(), order.end(), 0);
		shuffle(order.begin(), order.end(), mt);
		need_lvl = false;
		score = 0;
		const int NS = s2i.size();
		const int MAX_LVL = 100; // safe upper bound from problem constraints
		vector<vector<int>> cnt_ge(NS, vector<int>(MAX_LVL + 1, 0));
		for (int s = 0; s < NS; ++s)
		{
			for (int c = 0; c < C; ++c)
			{
				int lvl = cur_skill[c][s];
				if (lvl > 0)
					cnt_ge[s][lvl]++;
			}
			for (int l = MAX_LVL; l > 0; --l)
			{
				cnt_ge[s][l - 1] += cnt_ge[s][l];
			}
		}
		sol.clear();
		chosen.assign(C, false);
		const auto ckey = [&](int c, int m, int s)
		{ return make_tuple(max(av[c], m), cur_skill[c][s], cur_ts[c]); };
		while (true)
		{
			bool bad = true;
			vector<int> fail;
			sort(order.begin(), order.end(), [&](int a, int b)
				 { return B[a] - D[a] < B[b] - D[b]; });
			for (int p : order)
			{
				int mav = 0;
				vector<int> cs(req[p].size(), -1);
				array<int, 800> ms;
				ms.fill(0);
				vector<int> idx(req[p].size());
				iota(idx.begin(), idx.end(), 0);
				sort(idx.begin(), idx.end(), [&](int i, int j)
					 { return make_tuple(cnt_ge[req[p][i].first][req[p][i].second], -req[p][i].second, req[p][i].first) < make_tuple(cnt_ge[req[p][j].first][req[p][j].second], -req[p][j].second, req[p][j].first); });
				bool failed = false;
				for (int k = 0; k < idx.size(); ++k)
				{
					int i = idx[k];
					auto [s, l] = req[p][i];
					if (ms[s] >= l)
						--l;
					int best = -1;
					for (int c = 0; c < C; ++c)
						if (!chosen[c] && cur_skill[c][s] >= l)
							if (best == -1 || ckey(c, mav, s) < ckey(best, mav, s))
								best = c;
					if (best == -1)
					{
						failed = true;
						break;
					}
					cs[i] = best;
					for (int i = 0; i < s2i.size(); ++i)
						ms[i] = max(ms[i], skill[best][i]);
					chosen[best] = true;
					mav = max(mav, av[best]);
				}
				for (int c : cs)
					if (c != -1)
						chosen[c] = false;
				int end = mav + D[p];
				if (failed || (!need_lvl && end >= B[p] + S[p]))
				{
					fail.push_back(p);
					continue;
				}
				for (int i = 1; i < cs.size(); ++i)
					if (cur_skill[cs[i]][req[p][i].first] > req[p][i].second)
						for (int j = 0; j < i; ++j)
							if (cur_skill[cs[j]][req[p][j].first] > req[p][j].second)
								if (cur_skill[cs[i]][req[p][j].first] >= req[p][j].second - 1 && cur_skill[cs[j]][req[p][i].first] >= req[p][i].second - 1)
									swap(cs[i], cs[j]);
				bool impLvl = false;
				for (int i = 0; i < cs.size(); ++i)
				{
					av[cs[i]] = end;
					auto [s, l] = req[p][i];
					if (cur_skill[cs[i]][s] <= l)
					{
						int old_lvl = cur_skill[cs[i]][s];
						++cur_skill[cs[i]][s];
						++cnt_ge[s][old_lvl + 1];
						++cur_ts[cs[i]];
						impLvl = true;
					}
				}
				if (need_lvl && !impLvl && end >= B[p] + S[p])
				{
					fail.push_back(p);
					continue;
				}
				score += max(0, S[p] - max(0, end - B[p]));
				sol.emplace_back(p, cs);
				bad = false;
			}
			if (bad)
			{
				if (need_lvl)
					break;
				need_lvl = true;
			}
			order = fail;
		}
		if (score > best_score)
		{
			best_score = score;
			best_sol = std::move(sol);
		}
	} // end for run
	cerr << best_score << endl;
	cout << best_sol.size() << endl;
	for (const auto &[p, cs] : best_sol)
	{
		cout << Pnames[p] << '\n';
		for (int c : cs)
			cout << Cnames[c] << ' ';
		cout << '\n';
	}
	return 0;
}
