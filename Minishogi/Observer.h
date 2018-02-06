#ifndef _OBSERVER_
#define _OBSERVER_

#define REPORT_PATH       "output//"
#define LREPORT_PATH     L"output//"

#include "head.h"
using namespace std;

namespace Observer {
	// 單一盤面搜尋結果
	extern unsigned long long totalNode;
	extern unsigned long long researchNode;
	extern unsigned long long quiesNode;
	extern unsigned long long scoutGeneNums;
	extern unsigned long long scoutSearchBranch;
	extern unsigned	long long cutIllgalBranch;
	extern unsigned	long long collisionNums;
	extern double searchTime;
	static clock_t beginTime;

	// 整局結果
	extern unsigned long long startZobristHash;
	extern unsigned int kifuHash;
	extern unsigned int searchNum;
	extern bool winner;
	extern double gamePlayTime;

	extern unsigned long long game_totalNode;
	extern unsigned long long game_researchNode;
	extern unsigned long long game_quiesNode;
	extern unsigned long long game_scoutGeneNums;
	extern unsigned long long game_scoutSearchBranch;
	extern unsigned	long long game_cutIllgalBranch;
	extern unsigned	long long game_collisionNums;
	extern double game_searchTime;
	static clock_t game_beginTime;

	// 全部
	extern unsigned int gameNum;
	extern unsigned int whiteWinNum;

	// 設定
	extern int depth;
	extern bool isSaveRecord;

	inline void StartSearching() {
		totalNode = 0;
		researchNode = 0;
		quiesNode = 0;
		scoutGeneNums = 0;
		scoutSearchBranch = 0;
		cutIllgalBranch = 0;
		collisionNums = 0;
		searchTime = 0;
		beginTime = clock();
	}

	inline void EndSearching() {
		searchTime = double(clock() - beginTime) / 1000;

		searchNum++;

		game_totalNode += totalNode;
		game_researchNode += researchNode;
		game_quiesNode += quiesNode;
		game_scoutGeneNums += scoutGeneNums;
		game_scoutSearchBranch += scoutSearchBranch;
		game_cutIllgalBranch += cutIllgalBranch;
		game_collisionNums += collisionNums;
		game_searchTime += searchTime;
	}

	inline void GameStart(unsigned __int64 _startZobristHash) {
		startZobristHash = _startZobristHash;
		searchNum = 0;

		game_totalNode = 0;
		game_researchNode = 0;
		game_quiesNode = 0;
		game_scoutGeneNums = 0;
		game_scoutSearchBranch = 0;
		game_cutIllgalBranch = 0;
		game_collisionNums = 0;
		game_searchTime = 0;

		game_beginTime = clock();
	}

	inline void GameOver(bool _winner, unsigned int _kifuHash) {
		winner = _winner;
		kifuHash = _kifuHash;
		gamePlayTime = double(clock() - game_beginTime) / 1000;

		gameNum++;
		whiteWinNum += _winner == 0;
	}

	inline void PrintReport(std::ostream &os) {
		if (searchNum == 0) return;
		if (scoutGeneNums == 0) scoutGeneNums = 1;
		os << "Search Report :\n";
		os << setiosflags(ios::fixed) << setprecision(2);
		os << " Total search nodes      : " << setw(10) << setw(10) << totalNode << "\n";
		os << " Research nodes          : " << setw(10) << setw(10) << researchNode << "\n";
		os << " Quie search nodes       : " << setw(10) << quiesNode << "\n";
		os << " Avg scout search branch : " << setw(13) << (float)scoutSearchBranch / scoutGeneNums << "\n";
		os << " Cut illgal branch       : " << setw(10) << cutIllgalBranch << "\n";
		os << " Collision nums          : " << setw(10) << collisionNums << "\n";
		os << " Search time             : " << setw(13) << searchTime << "\n";
		os << endl;
	}


	inline void PrintGameReport(ostream &os) {
		if (searchNum == 0) return;
		os << setiosflags(ios::fixed) << setprecision(2);

		os << "Game" << setw(3) << gameNum - 1 << " : " << (winner ? "▼" : "△") << " Win!\n";
		os << "Game Result :\n";
		os << " Init board zobrist      : " << setw(10) << hex << startZobristHash << dec << "\n";
		os << " Kifu hashcode           : " << setw(10) << hex << kifuHash << dec << "\n";
		os << " Search depths           : " << setw(10) << depth << "\n";
		os << " Search nums             : " << setw(10) << searchNum << "\n";
		os << " Game play time          : " << setw(13) << gamePlayTime << "\n";
		os << " Winner                  : " << setw(10) << (winner ? "▼" : "△") << "\n";

		os << "Average Report (per search) :\n";
		os << " Total search nodes      : " << setw(10) << game_totalNode / searchNum << "\n";
		os << " Research nodes          : " << setw(10) << game_researchNode / searchNum << "\n";
		os << " Quie search nodes       : " << setw(10) << game_quiesNode / searchNum << "\n";
		os << " Avg scout search branch : " << setw(13) << (float)game_scoutSearchBranch / game_scoutGeneNums  << "\n";
		os << " Cut illgal branch       : " << setw(10) << game_cutIllgalBranch / searchNum << "\n";
		os << " Collision nums          : " << setw(10) << game_collisionNums / searchNum << "\n";
		os << " Search time             : " << setw(13) << game_searchTime / searchNum << "\n";
		os << endl;
	}

	inline void PrintObserverReport(ostream &os) {
		if (searchNum == 0) return;
		os << "Observer Report :\n";
		os << " Game play nums          : " << setw(10) << gameNum << "\n";
		os << " White win rate          : " << setw(10) << whiteWinNum * 100 / gameNum << "%\n";
		os << " Black win rate          : " << setw(10) << (gameNum - whiteWinNum) * 100 / gameNum << "%\n";
		os << endl;
	}
}
#endif
