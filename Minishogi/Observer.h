#ifndef _OBSERVER_H_
#define _OBSERVER_H_
#include <iomanip>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

//#define KPPT_DISABLE
//#define PIN_DISABLE
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
//#define BACKGROUND_SEARCH_DISABLE

#define AI_NAME "Nyanpass #116 time3"

namespace Observer {
	typedef std::chrono::milliseconds::rep TimePoint;

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

	// 整局結果
	extern uint64_t game_data[COUNT];
	extern Winner winner;

	// 全部結果
	extern uint64_t total_data[COUNT];
	extern uint32_t gameNum;

	//extern uint64_t lmrTest[162];
	//extern uint64_t total_lmrTest[162];
	//extern int aspFail[6];
	//extern int aspTime[6];

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

	void StartSearching();
	void EndSearching(int elpased);
	void GameStart();
	void GameOver(bool _winner, bool isSwap);

	void PrintData(std::ostream &os, uint64_t *d);
	void PrintSearchReport(std::ostream &os);
	void PrintGameReport(std::ostream &os);
	void PrintTotalReport(std::ostream &os);
	std::string GetSettingStr();
}
#endif