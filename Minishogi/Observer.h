#ifndef _OBSERVER_
#define _OBSERVER_
#include "head.h"
using namespace std;

namespace Observer {
	enum DataType {
		searchNum,
		totalNode,
		researchNode,
		quiesNode,
		scoutGeneNums,
		scoutSearchBranch,
		totalTPDepth,
		indexCollisionNums,
		searchTime,
		COUNT
	};

	// 單一盤面搜尋結果
	extern unsigned long long data[DataType::COUNT];
	static clock_t beginTime;

	// 整局結果
	extern unsigned long long game_data[DataType::COUNT];
	extern unsigned int kifuHash;
	extern bool isPlayer1Win;

	// 全部結果
	extern unsigned long long total_data[DataType::COUNT];
	extern unsigned int gameNum;
	extern unsigned int player1WinNum;

	// 設定
	extern int depth;
	extern bool isSaveRecord;

	inline void StartSearching() {
		for (int i = 0; i < DataType::COUNT; i++)
			data[i] = 0;
		beginTime = clock();
	}

	inline void EndSearching() {
		data[DataType::searchNum]++;
		data[DataType::searchTime] = clock() - beginTime;
		for (int i = 0; i < DataType::COUNT; i++)
			game_data[i] += data[i];
	}

	inline void GameStart() {
		for (int i = 0; i < DataType::COUNT; i++)
			game_data[i] = 0;
	}

	inline void GameOver(bool _isPlayer1Win, unsigned int _kifuHash) {
		isPlayer1Win = _isPlayer1Win;
		player1WinNum += isPlayer1Win;
		kifuHash = _kifuHash;
		gameNum++;
		for (int i = 0; i < DataType::COUNT; i++)
			total_data[i] += game_data[i];
	}

	inline void PrintSearchReport(std::ostream &os) {
		if (data[DataType::searchNum] == 0) return;
		if (data[DataType::scoutGeneNums] == 0) data[DataType::scoutGeneNums] = 1;
		os << setiosflags(ios::fixed) << setprecision(2);
		os << "Search Report :\n";
		os << " Total search nodes      : " << setw(10) << data[DataType::totalNode] << "\n";
		os << " Research nodes          : " << setw(10) << data[DataType::researchNode] << "\n";
		os << " Quies search nodes      : " << setw(10) << data[DataType::quiesNode] << "\n";
		os << " Avg scout search branch : " << setw(13) << (float)data[DataType::scoutSearchBranch] / data[DataType::scoutGeneNums] << "\n";
		os << " Total TP Depth          : " << setw(10) << data[DataType::totalTPDepth] << "\n";
		os << " Index Collision nums    : " << setw(10) << data[DataType::indexCollisionNums] << "\n";
		os << " Search time             : " << setw(13) << (float)data[DataType::searchTime] / 1000 << "\n";
		os << endl;
	}


	inline void PrintGameReport(ostream &os) {
		os << "Game" << setw(3) << gameNum - 1 << " : " << (isPlayer1Win ? "Player 1" : "Player 2") << " Win!\n";
		os << "Game Result :\n";
		os << " Kifu hashcode           : " << setw(10) << hex << kifuHash << dec << "\n";
		os << " Search depths           : " << setw(10) << depth << "\n";
		os << " Winner                  : " << setw(10) << (isPlayer1Win ? "Player 1" : "Player 2") << "\n";
		os << " Search nums             : " << setw(10) << game_data[DataType::searchNum] << "\n";

		if (game_data[DataType::searchNum] != 0) {
			os << setiosflags(ios::fixed) << setprecision(2);
			os << "Average Report (per search) :\n";
			os << " Total search nodes      : " << setw(10) << game_data[DataType::totalNode] / game_data[DataType::searchNum] << "\n";
			os << " Research nodes          : " << setw(10) << game_data[DataType::researchNode] / game_data[DataType::searchNum] << "\n";
			os << " Quies search nodes      : " << setw(10) << game_data[DataType::quiesNode] / game_data[DataType::searchNum] << "\n";
			os << " Avg scout search branch : " << setw(13) << (float)game_data[DataType::scoutSearchBranch] / game_data[DataType::scoutGeneNums] << "\n";
			os << " Total TP Depth          : " << setw(10) << game_data[DataType::totalTPDepth] / game_data[DataType::searchNum] << "\n";
			os << " Index Collision nums    : " << setw(10) << game_data[DataType::indexCollisionNums] / game_data[DataType::searchNum] << "\n";
			os << " Search time             : " << setw(13) << (float)game_data[DataType::searchTime] / game_data[DataType::searchNum] / 1000 << "\n";
		}
		os << endl;
	}

	inline void PrintTotalReport(ostream &os) {
		os << "Total Result :\n";
		os << " Game play nums          : " << setw(10) << gameNum << "\n";
		os << " Player 1 win rate       : " << setw(10) << player1WinNum * 100 / gameNum << "%\n";
		os << " Player 2 win rate       : " << setw(10) << (gameNum - player1WinNum) * 100 / gameNum << "%\n";
		os << " Search nums             : " << setw(10) << total_data[DataType::searchNum] << "\n";

		if (total_data[DataType::searchNum] != 0) {
			os << setiosflags(ios::fixed) << setprecision(2);
			os << "Average Report (per search) :\n";
			os << " Total search nodes      : " << setw(10) << total_data[DataType::totalNode] / total_data[DataType::searchNum] << "\n";
			os << " Research nodes          : " << setw(10) << total_data[DataType::researchNode] / total_data[DataType::searchNum] << "\n";
			os << " Quies search nodes      : " << setw(10) << total_data[DataType::quiesNode] / total_data[DataType::searchNum] << "\n";
			os << " Avg scout search branch : " << setw(13) << (float)total_data[DataType::scoutSearchBranch] / total_data[DataType::scoutGeneNums] << "\n";
			os << " Total TP Depth          : " << setw(10) << total_data[DataType::totalTPDepth] / total_data[DataType::searchNum] << "\n";
			os << " Index Collision nums    : " << setw(10) << total_data[DataType::indexCollisionNums] / total_data[DataType::searchNum] << "\n";
			os << " Search time             : " << setw(13) << (float)total_data[DataType::searchTime] / total_data[DataType::searchNum] / 1000 << "\n";
		}
		os << endl;
	}
}
#endif
