#include <iostream>
#include <fstream>
#include <windows.h>

#include "TimeTest.h"
#include "Thread.h"
#include "Minishogi.h"
#include "Search.h"
#include "Observer.h"
#include "Transposition.h"
using namespace std;
#define CUSTOM_BOARD_FILE "timetest_board.txt"
#define TIMETEST_PATH     "timetest_output//"

namespace TimeTest {
	string logPathStr;
	vector<RootMove> rms;

	void Testing() {
		Minishogi pos(nullptr);
		Thread *thread;
		streamoff readBoardOffset = 0;
		fstream file;

		while (pos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
			rms.emplace_back();
			thread = new Thread(USI::Options["HashEntry"]);
			cout << "---------- Board " << Observer::game_data[Observer::searchNum] << " ----------" << endl;
			pos.PrintChessBoard();
			cout << "Evaluate : " << setw(15) << pos.GetEvaluate() << endl;

			Observer::StartSearching();
			thread->IDAS(rms.back(), USI::Options["HashEntry"]);
			Observer::EndSearching();

			//cout << rms.back().PV() << endl;
			Observer::PrintSearchReport(cout);
			file.open(logPathStr, ios::app);
			file << "---------- Board " << Observer::game_data[Observer::searchNum] - 1 << " ----------" << endl;
			file << pos << "\n";
			file << "Evaluate : " << setw(15) << pos.GetEvaluate() << "\n";
			if (file) Observer::PrintSearchReport(file);
			file.close();

			delete thread;
		}
	}

	void TimeTest() {
		fstream file;

		cout << "AI Version : " << AI_VERSION << "\n" << Observer::GetSettingStr() << endl;
		SetConsoleTitle("Nyanpass " AI_VERSION " - TimeTest");
		logPathStr = TIMETEST_PATH + Observer::GetTimeStamp() + "_TimeTest.txt";

		CreateDirectory(TIMETEST_PATH, NULL);
		file.open(logPathStr, ios::app);
		if (file) file << "AI Version : " << AI_VERSION << "\n" << Observer::GetSettingStr() << endl;
		file.close();

		Zobrist::Initialize();
		Observer::GameStart();

		Testing();

		Observer::GameOver(0, 0, 0, 0);
		Observer::PrintGameReport(cout);
		file.open(logPathStr, ios::app);
		if (file) Observer::PrintGameReport(file);
		file << "num|  eval | move\n";
		for (int i = 0; i < rms.size(); i++) {
			file << setw(3) << i << "|" << setw(6) << rms[i].value << " | " << rms[i].pv[0] << "\n";
		}
		file.close();

		SetConsoleTitle("Nyanpass " AI_VERSION " - TimeTest : Stop");
	}
}