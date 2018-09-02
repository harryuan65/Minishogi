#ifndef _OBSERVER_H_
#define _OBSERVER_H_
#include <iomanip>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

//#define KPPT_DISABLE
//#define ITERATIVE_DEEPENING_DISABLE
//#define ASPIRE_WINDOW_DISABLE
//#define PVS_DISABLE
//#define NULLMOVE_DISABLE
//#define LMR_DISABLE
//#define QUIES_DISABLE
//#define TRANSPOSITION_DISABLE
#define ENEMY_ISO_TT
//#define MOVEPICK_DISABLE
#define REFUTATION_DISABLE
#define BACKGROUND_SEARCH_DISABLE
#define BACKGROUND_SEARCH_LIMITDEPTH

#define AI_VERSION "#103"

namespace Observer {

	enum DataType {
		searchNum,
		mainNode,
		researchNode,
		quiesNode,
		scoutGeneNums,
		scoutSearchBranch,
		//ttIsoNum,
		ttProbe,
		ttCollision,
		nullMoveNum,
		zugzwangsNum,
		//lmrTestNum,
		searchTime,
		COUNT
	};

	enum Winner {
		PLAYER1,
		PLAYER2
	};

	// 單一盤面搜尋結果
	extern uint64_t data[COUNT];
	static clock_t beginTime = 0;

	// 整局結果
	extern uint64_t game_data[COUNT];
	extern Winner winner;

	// 全部結果
	extern uint64_t total_data[COUNT];
	extern uint32_t gameNum;
	extern uint32_t player1WinNum;
	extern uint32_t player2WinNum;
	extern std::vector<Winner> winnerTable1;
	extern std::vector<Winner> winnerTable2;
	extern std::vector<uint64_t> initKey;
	extern std::vector<uint32_t> kifuHash1;
	extern std::vector<uint32_t> kifuHash2;

	// 設定
	extern int depth;
	extern int limitTime;
	extern bool isSaveRecord;
	extern std::string kpptName;
	extern std::stringstream LearnLog;

	inline std::tm localtime_xp(std::time_t timer) {
		std::tm bt{};
#if defined(__unix__)
		localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
		localtime_s(&bt, &timer);
#else
		static std::mutex mtx;
		std::lock_guard<std::mutex> lock(mtx);
		bt = *std::localtime(&timer);
#endif
		return bt;
	}

	static std::string GetTimeStamp() {
		tm bt = localtime_xp(std::time(0));
		char buffer[64];
		strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", &bt);
		return std::string(buffer);
	}

	inline void LearnLogDump(std::string path) {
		std::ofstream ofLog(path, std::ios::app);
		if (ofLog) {
			ofLog << LearnLog.str();
			LearnLog.str("");
			ofLog.close();
		}
	}

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

	inline void GameOver(bool _winner, bool isSwap, uint64_t key, uint32_t _kifuHash) {
		gameNum++;
		winner = (Winner)(_winner != isSwap);
		if (winner == PLAYER1)
			player1WinNum++;
		else if (winner == PLAYER2)
			player2WinNum++;
		if (!isSwap) {
			winnerTable1.push_back(winner);
			initKey.push_back(key);
			kifuHash1.push_back(_kifuHash);
		}
		else {
			winnerTable2.push_back(winner);
			kifuHash2.push_back(_kifuHash);
		}
		for (int i = 0; i < COUNT; i++)
			total_data[i] += game_data[i];
	}

	inline static void PrintData(std::ostream &os, uint64_t *pdata) {
		bool isZero = pdata[searchNum] == 0;
		if (isZero)  pdata[searchNum] = 1;
		os << std::setiosflags(std::ios::fixed) << std::setprecision(2);
		os << "Average Report (per search) :\n";
		os << " Total node              : " << std::setw(10) << (pdata[mainNode] + pdata[quiesNode]) / pdata[searchNum] << "\n";
		os << " Main search nodes       : " << std::setw(10) << pdata[mainNode] / pdata[searchNum] << "\n";
		os << " Research nodes          : " << std::setw(10) << pdata[researchNode] / pdata[searchNum] << "\n";
		os << " Quies search nodes      : " << std::setw(10) << pdata[quiesNode] / pdata[searchNum] << "\n";
		os << " Avg scout search branch : " << std::setw(13) << (float)pdata[scoutSearchBranch] / pdata[scoutGeneNums] << "\n";
		//os << " ttProbe isomorphic nums : " << std::setw(10) << pdata[ttIsoNum] / pdata[searchNum] << "\n";
		//os << " ttProbe isomorphic rate : " << std::setw(13) << (100.0f * pdata[ttIsoNum] / pdata[ttProbe]) << " %\n";
		os << " ttProbe collision nums  : " << std::setw(10) << pdata[ttCollision] / pdata[searchNum] << "\n";
		os << " ttProbe collision rate  : " << std::setw(13) << (100.0f * pdata[ttCollision] / pdata[ttProbe]) << " %\n";
		//os << " zugzwangs num           : " << std::setw(13) <<  (float)pdata[zugzwangsNum] / pdata[searchNum] << "\n";
		os << " Null Move num           : " << std::setw(10) << pdata[nullMoveNum] / pdata[searchNum] << "\n";
		//os << " LMR Failed node         : " << std::setw(10) << pdata[lmrTestNum] << "\n";
		os << " Search time             : " << std::setw(13) << (float)pdata[searchTime] / pdata[searchNum] / 1000 << "\n";
		if (isZero)  pdata[searchNum] = 0;
	}

	inline void PrintSearchReport(std::ostream &os) {
		if (!os) return;
		if (data[scoutGeneNums] == 0) data[scoutGeneNums] = 1;
		os << "Search Deapth            : " << std::setw(10) << depth << "\n";
		PrintData(os, data);
		os << std::endl;
	}

	inline void PrintGameReport(std::ostream &os) {
		if (!os) return;
		os << "Game" << std::setw(3) << gameNum - 1 << "\n";
		os << "Game Result :\n";
		os << " Winner                  : " << std::setw(10) << (winner ? "Player2" : "Player1") << "\n";
		//os << " Kifu hashcode           : " << std::setw(10) << hex << kifuHash.back() << dec << "\n";
		os << " Search depths           : " << std::setw(10) << depth << "\n";
		os << " Search nums             : " << std::setw(10) << game_data[searchNum] << "\n";
		PrintData(os, game_data);
		os << std::endl;
	}

	inline void PrintTotalReport(std::ostream &os) {
		if (!os) return;
		if (gameNum == 0) return;
		os << "Game Result :\n";
		os << " Search depths           : " << std::setw(10) << depth << "\n";
		os << " Search nums             : " << std::setw(10) << total_data[searchNum] << "\n";
		PrintData(os, total_data);
		os << std::endl;
	}

	inline void PrintWinnerReport(std::ostream &os) {
		if (!os) return;
		if (gameNum == 0) return;
		os << "Total Result :\n";
		os << " Game play nums          : " << std::setw(10) << gameNum << "\n";
		os << " Player 1 win nums       : " << std::setw(10) << player1WinNum << "\n";
		os << " Player 2 win nums       : " << std::setw(10) << player2WinNum << "\n";
		os << "Game |    Init Borad    | A | B |  A kifu  |  B kifu  | Game\n";
		for (int i = 0; i < winnerTable1.size(); i++) {
			os << std::setw(4) << i << " | " << std::hex << std::setw(16) << initKey[i] << std::dec << " | ";
			os << (winnerTable1[i] ? "-" : "+") << " | ";
			if (i < winnerTable2.size()) {
				os << (winnerTable2[i] ? "-" : "+") << " | ";
			}
			os << std::hex << std::setw(8) << kifuHash1[i] << std::dec << " | " ;
			if (i < winnerTable2.size()) {
				os << std::hex << std::setw(8) << kifuHash2[i] << std::dec << " | ";
				os << std::setw(4) << i + winnerTable1.size();
			}
			os << "\n";
		}
		os << "(+:Player 1 win,-:Player 2 Win)\n";
		os << std::endl;
	}
}
#endif
