#ifndef _OBSERVER_
#define _OBSERVER_
#include <iomanip>
#include <vector>
#include <iostream>
using namespace std;

//#define ITERATIVE_DEEPENING_DISABLE
//#define ASPIRE_WINDOW_DISABLE
//#define PVS_DISABLE
//#define QUIES_DISABLE
//#define TRANSPOSITION_DISABLE
#define ENEMY_ISO_TT
//#define MOVEPICK_DISABLE
#define REFUTATION_DISABLE
#define BACKGROUND_SEARCH_DISABLE

#define AI_VERSION "#99"

namespace Observer {
	enum DataType {
		searchNum,
		totalNode,
		researchNode,
		quiesNode,
		scoutGeneNums,
		scoutSearchBranch,
		ttIsoNum,
		ttProbe,
		ttCollision,
		searchTime,
		COUNT
	};

	enum Winner {
		PLAYER1,
		PLAYER2
	};

	// 單一盤面搜尋結果
	extern unsigned long long data[COUNT];
	static clock_t beginTime = 0;

	// 整局結果
	extern unsigned long long game_data[COUNT];
	extern Winner winner;

	// 全部結果
	extern unsigned long long total_data[COUNT];
	extern unsigned int gameNum;
	extern unsigned int player1WinNum;
	extern unsigned int player2WinNum;
	extern vector<Winner> winnerTable1;
	extern vector<Winner> winnerTable2;
	extern vector<uint64_t> initHash;
	extern vector<uint32_t> kifuHash1;
	extern vector<uint32_t> kifuHash2;

	// 設定
	extern int depth;
	extern int limitTime;
	extern bool isSaveRecord;
	extern string playDetailStr;

	inline void StartSearching() {
		for (int i = 0; i < COUNT; i++)
			data[i] = 0;
		beginTime = clock();
	}

	inline void EndSearching() {
		data[searchNum]++;
		data[searchTime] += clock() - beginTime;
		for (int i = 0; i < COUNT; i++)
			game_data[i] += data[i];
		beginTime = 0;
	}

	inline void GameStart() {
		for (int i = 0; i < COUNT; i++)
			game_data[i] = 0;
	}

	inline void GameOver(bool _winner, bool isSwap, unsigned __int64 _initHash, unsigned int _kifuHash) {
		gameNum++;
		winner = (Winner)(_winner != isSwap);
		if (winner == PLAYER1)
			player1WinNum++;
		else if (winner == PLAYER2)
			player2WinNum++;
		if (!isSwap) {
			winnerTable1.push_back(winner);
			initHash.push_back(_initHash);
			kifuHash1.push_back(_kifuHash);
		}
		else {
			winnerTable2.push_back(winner);
			kifuHash2.push_back(_kifuHash);
		}
		for (int i = 0; i < COUNT; i++)
			total_data[i] += game_data[i];
	}

	inline static void PrintData(ostream &os, unsigned long long *pdata) {
		bool isZero = pdata[searchNum] == 0;
		if (isZero)  pdata[searchNum] = 1;
		os << setiosflags(ios::fixed) << setprecision(2);
		os << "Average Report (per search) :\n";
		os << " Total search nodes      : " << setw(10) << pdata[totalNode] / pdata[searchNum] << "\n";
		os << " Research nodes          : " << setw(10) << pdata[researchNode] / pdata[searchNum] << "\n";
		os << " Quies search nodes      : " << setw(10) << pdata[quiesNode] / pdata[searchNum] << "\n";
		os << " Avg scout search branch : " << setw(13) << (float)pdata[scoutSearchBranch] / pdata[scoutGeneNums] << "\n";
		os << " ttProbe isomorphic nums : " << setw(10) << pdata[ttIsoNum] / pdata[searchNum] << "\n";
		os << " ttProbe isomorphic rate : " << setw(13) << (100.0f * pdata[ttIsoNum] / pdata[ttProbe]) << " %\n";
		os << " ttProbe collision nums  : " << setw(10) << pdata[ttCollision] / pdata[searchNum] << "\n";
		os << " ttPribe collision rate  : " << setw(13) << (100.0f * pdata[ttCollision] / pdata[ttProbe]) << " %\n";
		os << " Search time             : " << setw(13) << (float)pdata[searchTime] / pdata[searchNum] / 1000 << "\n";
		if (isZero)  pdata[searchNum] = 0;
	}

	inline void PrintSearchReport(ostream &os) {
		if (!os) return;
		if (data[scoutGeneNums] == 0) data[scoutGeneNums] = 1;
		os << "Search Deapth            : " << setw(10) << depth << "\n";
		PrintData(os, data);
		os << endl;
	}

	inline void PrintGameReport(ostream &os) {
		if (!os) return;
		os << "Game" << setw(3) << gameNum - 1 << "\n";
		os << "Game Result :\n";
		os << " Winner                  : " << setw(10) << (winner ? "Player2" : "Player1") << "\n";
		//os << " Kifu hashcode           : " << setw(10) << hex << kifuHash.back() << dec << "\n";
		os << " Search depths           : " << setw(10) << depth << "\n";
		os << " Search nums             : " << setw(10) << game_data[searchNum] << "\n";
		PrintData(os, game_data);
		os << endl;
	}

	inline void PrintTotalReport(ostream &os) {
		if (!os) return;
		if (gameNum == 0) return;
		os << "Game Result :\n";
		os << " Search depths           : " << setw(10) << depth << "\n";
		os << " Search nums             : " << setw(10) << total_data[searchNum] << "\n";
		PrintData(os, total_data);
		os << endl;
	}

	inline void PrintWinnerReport(ostream &os) {
		if (!os) return;
		if (gameNum == 0) return;
		os << "Total Result :\n";
		os << " Game play nums          : " << setw(10) << gameNum << "\n";
		os << " Player 1 win nums       : " << setw(10) << player1WinNum << "\n";
		os << " Player 2 win nums       : " << setw(10) << player2WinNum << "\n";
		os << "Game |    Init Borad    | A | B |  A kifu  |  B kifu  | Game\n";
		for (int i = 0; i < winnerTable1.size(); i++) {
			os << setw(4) << i << " | " << hex << setw(16) << initHash[i] << dec << " | ";
			os << (winnerTable1[i] ? "-" : "+") << " | ";
			if (i < winnerTable2.size()) {
				os << (winnerTable2[i] ? "-" : "+") << " | ";
			}
			os << hex << setw(8) << kifuHash1[i] << dec << " | " ;
			if (i < winnerTable2.size()) {
				os << hex << setw(8) << kifuHash2[i] << dec << " | ";
				os << setw(4) << i + winnerTable1.size();
			}
			os << "\n";
		}
		os << "(+:Player 1 win,-:Player 2 Win)\n";
		os << endl;
	}
}
#endif
