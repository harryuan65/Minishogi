#ifndef _OBSERVER_H_
#define _OBSERVER_H_
#include <iomanip>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "usi.h"

//#define KPPT_DISABLE
//#define KPPT_ONLY
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

#define AI_NAME "Nyanpass #109"

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

	void startLogger(bool b);

	// 單一盤面搜尋結果
	extern uint64_t data[COUNT];
	extern TimePoint start_time;

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
	extern std::vector<uint32_t> kifuHash1;
	extern std::vector<uint32_t> kifuHash2;

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

	inline void StartSearching() {
		for (int i = 0; i < COUNT; i++)
			data[i] = 0;
		start_time = now();
	}

	inline void EndSearching() {
		data[searchNum]++;
		data[searchTime] += now() - start_time;
		for (int i = 0; i < COUNT; i++)
			game_data[i] += data[i];
	}

	inline void GameStart() {
		for (int i = 0; i < COUNT; i++)
			game_data[i] = 0;
	}

	inline void GameOver(bool _winner, bool isSwap, uint32_t _kifuHash) {
		gameNum++;
		winner = (Winner)(_winner != isSwap);
		if (winner == PLAYER1)
			player1WinNum++;
		else if (winner == PLAYER2)
			player2WinNum++;
		if (!isSwap) {
			winnerTable1.push_back(winner);
			kifuHash1.push_back(_kifuHash);
		}
		else {
			winnerTable2.push_back(winner);
			kifuHash2.push_back(_kifuHash);
		}
		for (int i = 0; i < COUNT; i++)
			total_data[i] += game_data[i];
	}

	inline static void PrintData(std::ostream &os, uint64_t *d) {
		bool isZero = d[searchNum] == 0;
		if (isZero) d[searchNum] = 1;
		os << std::setiosflags(std::ios::fixed) << std::setprecision(2);
		os << "Average Report (per search) :\n";
		os << " Total node              : " << std::setw(10) << (d[mainNode] + d[quiesNode]) / d[searchNum] << "\n";
		os << " Main search nodes       : " << std::setw(10) << d[mainNode] / d[searchNum] << "\n";
		os << " Research nodes          : " << std::setw(10) << d[researchNode] / d[searchNum] << "\n";
		os << " Quies search nodes      : " << std::setw(10) << d[quiesNode] / d[searchNum] << "\n";
		os << " Avg scout search branch : " << std::setw(13) << (float)d[scoutSearchBranch] / d[scoutGeneNums] << "\n";
		//os << " ttProbe isomorphic nums : " << std::setw(10) << d[ttIsoNum] / d[searchNum] << "\n";
		//os << " ttProbe isomorphic rate : " << std::setw(13) << (100.0f * d[ttIsoNum] / d[ttProbe]) << " %\n";
		os << " ttProbe collision nums  : " << std::setw(10) << d[ttCollision] / d[searchNum] << "\n";
		os << " ttProbe collision rate  : " << std::setw(13) << (100.0f * d[ttCollision] / d[ttProbe]) << " %\n";
		//os << " zugzwangs num           : " << std::setw(13) <<  (float)d[zugzwangsNum] / d[searchNum] << "\n";
		os << " Null Move num           : " << std::setw(10) << d[nullMoveNum] / d[searchNum] << "\n";
		//os << " LMR Failed node         : " << std::setw(10) << d[lmrTestNum] << "\n";
		os << " Search time             : " << std::setw(13) << (float)d[searchTime] / d[searchNum] / 1000 << "\n";
		if (isZero) d[searchNum] = 0;
	}

	inline void PrintSearchReport(std::ostream &os) {
		if (!os) return;
		if (data[scoutGeneNums] == 0) data[scoutGeneNums] = 1;
		os << "Search Deapth            : " << std::setw(10) << USI::Options["Depth"] << "\n";
		PrintData(os, data);
	}

	inline void PrintGameReport(std::ostream &os) {
		if (!os) return;
		if (!game_data[searchNum]) return;
		os << "Game" << std::setw(3) << gameNum - 1 << "\n";
		os << "Game Result :\n";
		os << " Winner                  : " << std::setw(10) << (winner ? "Player2" : "Player1") << "\n";
		//os << " Kifu hashcode           : " << std::setw(10) << hex << kifuHash.back() << dec << "\n";
		os << " Search depths           : " << std::setw(10) << USI::Options["Depth"] << "\n";
		os << " Search nums             : " << std::setw(10) << game_data[searchNum] << "\n";
		PrintData(os, game_data);
		os << std::endl;
	}

	inline void PrintTotalReport(std::ostream &os) {
		if (!os) return;
		if (gameNum == 0) return;
		os << "Game Result :\n";
		os << " Search depths           : " << std::setw(10) << USI::Options["Depth"] << "\n";
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
		os << "Game | A | B |  A kifu  |  B kifu  | Game\n";
		for (int i = 0; i < winnerTable1.size(); i++) {
			os << std::setw(4) << i << " | " << (winnerTable1[i] ? "-" : "+") << " | ";
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

	inline std::string GetSettingStr() {
		std::stringstream ss;
		ss << "AI Version          : " << AI_NAME << "\n";
		ss << "Main Depth          : " << USI::Options["Depth"] << "\n";
		//ss << "Time Limit          : " << (limitTime ? std::to_string(limitTime) + " ms" : "Disable") << "\n";
#ifndef BACKGROUND_SEARCH_DISABLE
		ss << "Background Search   : Enable\n";
#else
		ss << "Background Search   : Disable\n";
#endif
#ifdef KPPT_DISABLE
		ss << "Evaluater           : Material, Pinner\n";
#elif defined KPPT_ONLY
		ss << "Evaluater           : KPPT(" << (std::string)USI::Options["EvalDir"] << ") only\n";
#else
		ss << "Evaluater           : KPPT(" << (std::string)USI::Options["EvalDir"] << "), Material, Pinner\n";
#endif
#ifdef TRANSPOSITION_DISABLE
		ss << "Transposition Table : Enable\n";
#else
#ifdef ENEMY_ISO_TT
		ss << "Transposition Table : Enemy Isomorphism\n";
#else
		ss << "Transposition Table : Single Hashcode\n";
#endif
		ss << "Transposition Entry : 2^" << USI::Options["HashEntry"] << "\n";
#endif

#ifndef ITERATIVE_DEEPENING_DISABLE
		ss << "Iterative Deepening : Enable\n";
#else
		ss << "Iterative Deepening : Disable\n";
#endif
#ifndef ASPIRE_WINDOW_DISABLE
		ss << "Aspire Window       : Enable\n";
#else
		ss << "Aspire Window       : Disable\n";
#endif
#ifndef PVS_DISABLE
		ss << "PVS                 : Enable\n";
#else
		ss << "PVS                 : Disable\n";
#endif 
#ifndef NULLMOVE_DISABLE
		ss << "Null Move Pruning   : Enable\n";
#else
		ss << "Null Move Pruning   : Disable\n";
#endif
#ifndef LMR_DISABLE
		ss << "Late Move Reduction : Enable\n";
#else
		ss << "Late Move Reduction : Disable\n";
#endif
#ifndef QUIES_DISABLE
		ss << "Quiet Search        : Enable\n";
#else
		ss << "Quiet Search        : Disable\n";
#endif
#ifndef MOVEPICK_DISABLE
		ss << "MovePicker          : Enable\n";
#else
		ss << "MovePicker          : Disable\n";
#endif
		return ss.str();
	}
}
#endif