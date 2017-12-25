#ifndef _OBSERVER_
#define _OBSERVER_
#include "define.h"
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <iomanip>
#include <vector>
#include <functional>
using namespace std;

namespace Observer {
	extern unsigned int searchNum;

	extern unsigned long long totalNode;
	extern unsigned long long researchNode;
	extern unsigned long long quieNode;
	extern unsigned long long scoutGeneNums;
	extern unsigned long long scoutSearchBranch;
	extern unsigned	long long cutIllgalBranch;
	extern double searchTime;

	extern unsigned int gameNum;
	extern unsigned int whiteWinNum;
	extern vector<bool> winner;
	extern vector<unsigned int> kifuHash;

	extern unsigned long long all_totalNode;
	extern unsigned long long all_researchNode;
	extern unsigned long long all_quieNode;
	extern unsigned long long all_scoutGeneNums;
	extern unsigned long long all_scoutSearchBranch;
	extern unsigned	long long all_cutIllgalBranch;
	extern double all_searchTime;

	extern int depth;
	extern bool isAutoSaveKifu;
	extern bool isAutoSaveDetail;
	extern bool isAutoSaveAIReport;

	static clock_t beginTime;

	inline void StartSearching() {
		totalNode = 0;
		researchNode = 0;
		quieNode = 0;
		scoutGeneNums = 0;
		scoutSearchBranch = 0;
		cutIllgalBranch = 0;
		searchTime = 0;

		beginTime = clock();
	}

	inline void EndSearching() {
		searchTime = double(clock() - beginTime) / 1000;

		searchNum++;

		all_totalNode += totalNode;
		all_researchNode += researchNode;
		all_quieNode += quieNode;
		all_scoutGeneNums += scoutGeneNums;
		all_scoutSearchBranch += scoutSearchBranch;
		all_cutIllgalBranch += cutIllgalBranch;
		all_searchTime += searchTime;
	}

	inline unsigned int GetKifuHash(vector<Action> kifu) {
		unsigned int seed = kifu.size();
		for (auto& i : kifu) {
			seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}

	inline void GameOver(bool _winner, vector<Action> kifu) {
		gameNum++;
		whiteWinNum += _winner == 0;
		winner.push_back(_winner);
		kifuHash.push_back(GetKifuHash(kifu));
	}

	inline void PrintReport(std::ostream &os) {
		if (scoutGeneNums == 0)
			scoutGeneNums = 1;
		os << "Search Report :" << endl;
		os << "Total search nodes      : " << setw(10) << totalNode << "\n";
		os << "Research nodes          : " << setw(10) << researchNode << "\n";
		os << "Quie search nodes       : " << setw(10) << quieNode << "\n";
		os << "Avg scout search branch : " << setw(13) << (float)scoutSearchBranch / scoutGeneNums << setiosflags(ios::fixed) << setprecision(2) << "\n";
		os << "Cut illgal branch       : " << setw(10) << cutIllgalBranch << "\n";
		os << "Search time             : " << setw(13) << searchTime << setiosflags(ios::fixed) << setprecision(2) << "\n";
	}


	inline void PrintAvgReport(ostream &os) {
		if (searchNum == 0)
			return;
		os << "Play Result :\n";
		os << "Search depths           : " << setw(10) << depth << "\n";
		os << "Game play nums          : " << setw(10) << gameNum << "\n";
		os << "Search nums             : " << setw(10) << searchNum << "\n";
		os << "White win rate          : " << setw(10) << (int)(whiteWinNum * 100 / gameNum) << "%\n";
		os << "Black win rate          : " << setw(10) << 100 - (int)(whiteWinNum * 100 / gameNum) << "%\n";
		for (int i = 0; i < winner.size(); i++) {
			os << "Round " << setw(3) << i << " : " << (winner[i] ? "¡¿" : "¡µ") << " Win! " << setw(8) << std::hex << kifuHash[i] << std::dec << "\n";
		}

		os << "Average Report (Divide Search nums) :\n";
		os << "Total search nodes      : " << setw(10) << all_totalNode / searchNum << "\n";
		os << "Research nodes          : " << setw(10) << all_researchNode / searchNum << "\n";
		os << "Quie search nodes       : " << setw(10) << all_quieNode / searchNum << "\n";
		os << "Avg scout search branch : " << setw(13) << (float)all_scoutSearchBranch / all_scoutGeneNums << setiosflags(ios::fixed) << setprecision(2) << "\n";
		os << "Cut illgal branch       : " << setw(10) << all_cutIllgalBranch / searchNum << "\n";
		os << "Search time             : " << setw(13) << all_searchTime / searchNum << setiosflags(ios::fixed) << setprecision(2) << "\n";
	}
}
#endif
