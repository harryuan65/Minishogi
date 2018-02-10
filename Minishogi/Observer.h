#ifndef _OBSERVER_
#define _OBSERVER_

#include "head.h"
using namespace std;

namespace Observer {
	enum DataType {
		totalNode,
		researchNode,
		quiesNode,
		scoutGeneNums,
		scoutSearchBranch,
		totalTPDepth,
		indexCollisionNums,
		evalCollisionNums,
		COUNT
	};

	// 單一盤面搜尋結果
	extern unsigned long long data[DataType::COUNT];
	extern double searchTime;
	static clock_t beginTime;

	// 整局結果
	extern unsigned long long game_data[DataType::COUNT];
	extern double game_searchTime;
	static clock_t game_beginTime;

	extern unsigned long long startZobristHash;
	extern unsigned int kifuHash;
	extern unsigned int searchNum;
	extern bool winner;
	extern double gamePlayTime;

	// 全部
	extern unsigned int gameNum;
	extern unsigned int whiteWinNum;

	// 設定
	extern int depth;
	extern bool isSaveRecord;

	inline void StartSearching() {
		for (int i = 0; i < DataType::COUNT; i++)
			data[i] = 0;
		searchTime = 0;
		beginTime = clock();
	}

	inline void EndSearching() {
		searchTime = double(clock() - beginTime) / 1000;
		searchNum++;

		for (int i = 0; i < DataType::COUNT; i++)
			game_data[i] += data[i];
		game_searchTime += searchTime;
	}

	inline void GameStart(unsigned __int64 _startZobristHash) {
		startZobristHash = _startZobristHash;
		searchNum = 0;

		for (int i = 0; i < DataType::COUNT; i++)
			game_data[i] = 0;
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
		if (data[DataType::scoutGeneNums] == 0) data[DataType::scoutGeneNums] = 1;
		os << "Search Report :\n";
		os << setiosflags(ios::fixed) << setprecision(2);
		os << " Total search nodes      : " << setw(10) << data[DataType::totalNode] << "\n";
		os << " Research nodes          : " << setw(10) << data[DataType::researchNode] << "\n";
		os << " Quies search nodes      : " << setw(10) << data[DataType::quiesNode] << "\n";
		os << " Avg scout search branch : " << setw(13) << (float)data[DataType::scoutSearchBranch] / data[DataType::scoutGeneNums] << "\n";
		os << " Total TP Depth          : " << setw(10) << data[DataType::totalTPDepth] << "\n";
		os << " Index Collision nums    : " << setw(10) << data[DataType::indexCollisionNums] << "\n";
		os << " Eval Collision nums     : " << setw(10) << data[DataType::evalCollisionNums] << "\n";
		os << " Search time             : " << setw(13) << searchTime << "\n";
		os << endl;
	}


	inline void PrintGameReport(ostream &os) {
		if (searchNum == 0) return;
		os << setiosflags(ios::fixed) << setprecision(2);

		os << "Game" << setw(3) << gameNum - 1 << " : " << (winner ? "▼" : "△") << " Win!\n";
		os << "Game Result :\n";
		os << " Init board zobrist      : " << setw(18) << hex << startZobristHash << dec << "\n";
		os << " Kifu hashcode           : " << setw(10) << hex << kifuHash << dec << "\n";
		os << " Search depths           : " << setw(10) << depth << "\n";
		os << " Search nums             : " << setw(10) << searchNum << "\n";
		os << " Game play time          : " << setw(13) << gamePlayTime << "\n";
		os << " Winner                  : " << setw(10) << (winner ? "▼" : "△") << "\n";
		
		os << "Average Report (per search) :\n";
		os << " Total search nodes      : " << setw(10) << game_data[DataType::totalNode] / searchNum << "\n";
		os << " Research nodes          : " << setw(10) << game_data[DataType::researchNode] / searchNum << "\n";
		os << " Quies search nodes      : " << setw(10) << game_data[DataType::quiesNode] / searchNum << "\n";
		os << " Avg scout search branch : " << setw(13) << (float)game_data[DataType::scoutSearchBranch] / game_data[DataType::scoutGeneNums] << "\n";
		os << " Total TP Depth          : " << setw(10) << game_data[DataType::totalTPDepth] / searchNum << "\n";
		os << " Index Collision nums    : " << setw(10) << game_data[DataType::indexCollisionNums] / searchNum << "\n";
		os << " Eval Collision nums     : " << setw(10) << game_data[DataType::evalCollisionNums] / searchNum << "\n";
		os << " Search time             : " << setw(13) << game_searchTime / searchNum << "\n";
		os << endl;
	}

	inline void PrintObserverReport(ostream &os) {
		if (searchNum == 0) return;
		os << "Observer Report :\n";
		os << " Game play nums          : " << setw(10) << gameNum << "\n";
		os << " Player 1 win rate       : " << setw(10) << whiteWinNum * 100 / gameNum << "%\n";
		os << " Player 2 win rate       : " << setw(10) << (gameNum - whiteWinNum) * 100 / gameNum << "%\n";
		os << endl;
	}
}
#endif
