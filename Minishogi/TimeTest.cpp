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
		Minishogi pos;
		Thread *thread;
		streamoff readBoardOffset = 0;
		fstream file;

		while (pos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
			rms.emplace_back();
			thread = new Thread(pos, pos.GetTurn(), Observer::ttBit);
			cout << "---------- Board " << Observer::game_data[Observer::searchNum] << " ----------" << endl;
			pos.PrintChessBoard();
			cout << "Evaluate : " << setw(15) << pos.GetEvaluate() << endl;

			Observer::StartSearching();
			thread->IDAS(rms.back(), Observer::depth, false);
			Observer::EndSearching();

			//cout << rms.back().PV() << endl;
			Observer::PrintSearchReport(cout);
			file.open(logPathStr, ios::app);
			file << "---------- Board " << Observer::game_data[Observer::searchNum] - 1 << " ----------" << endl;
			pos.PrintNoncolorBoard(file);
			file << "Evaluate : " << setw(15) << pos.GetEvaluate() << "\n";
			thread->Dump(file);
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

		cout << "輸入搜尋的深度" << endl;
		cin >> Observer::depth;

#ifndef KPPT_DISABLE
		cout << "輸入KPP名稱" << endl;
		cin >> Observer::kpptName;
		if (!Evaluate::evaluater.Load(Observer::kpptName)) {
			Observer::kpptName = "";
			Evaluate::evaluater.Clean();
		}
#else
		Evaluate::evaluater.Clean();
#endif

#ifndef TRANSPOSITION_DISABLE
		int size;
		cout << "輸入同型表entry數量(2^n)" << endl;
		cin >> size;
		Observer::ttBit = size;
#endif

		cout << Observer::GetSettingStr() << "確定要開始?" << endl;
		system("pause");

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