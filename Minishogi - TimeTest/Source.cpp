#define _CRT_SECURE_NO_WARNINGS
#include <atlstr.h>
#include <fstream>

#include "FileMapping.h"
#include "Thread.h"
#include "Minishogi.h"
#include "Search.h"
#include "Observer.h"
#include "Transposition.h"
using namespace std;

#define CUSTOM_BOARD_FILE "timetest_board.txt"
#define TIMETEST_PATH     "timetest_output//"

string GetCurrentTimeString();

int main() {
	Minishogi pos;
	Thread *thread;
	vector<RootMove> rms;
	streamoff readBoardOffset = 0;
	fstream file;
	string playDetailStr = TIMETEST_PATH + GetCurrentTimeString() + "_TimeTest.txt";

	SetConsoleTitle(L"Minishogi (TimeTest) - " AI_VERSION);
	sync_cout << "AI Version : " << AI_VERSION << "\n" << Search::GetSettingStr() << sync_endl;

	sync_cout << "輸入搜尋的深度" << sync_endl;
	cin >> Observer::depth;

#ifndef TRANSPOSITION_DISABLE
	int size;
	sync_cout << "輸入同型表entry數量(2^n)" << sync_endl;
	cin >> size;
	Transposition::TPSize = 1 << size;
#endif

	sync_cout << "輸入時間限制(ms 0為無限制)" << sync_endl;
	cin >> Observer::limitTime;

	sync_cout << Search::GetSettingStr() << "確定要開始?" << sync_endl;
	system("pause");

	CreateDirectory(CA2W(TIMETEST_PATH), NULL);
	file.open(playDetailStr, ios::app);
	if (file) file << "AI Version : " << AI_VERSION << "\n" << Search::GetSettingStr() << endl;
	file.close();

	Evaluate::Initialize();
	Zobrist::Initialize();
	Transposition::Initialize();
	Observer::GameStart();
	while (pos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
		rms.emplace_back();
		Transposition::Clean();
		thread = new Thread(pos, pos.GetTurn());
		sync_cout << "---------- Board " << Observer::game_data[Observer::searchNum] << " ----------" << sync_endl;
		pos.PrintChessBoard();
		sync_cout << "Evaluate : " << setw(15) << pos.GetEvaluate() << sync_endl;

		Observer::StartSearching();
		thread->IDAS(rms.back(), Observer::depth, false);
		Observer::EndSearching();

		//sync_cout << rms.back().PV() << sync_endl;
		Observer::PrintSearchReport(cout);
		file.open(playDetailStr, ios::app);
		file << "---------- Board " << Observer::game_data[Observer::searchNum] - 1 << " ----------" << endl;
		pos.PrintNoncolorBoard(file);
		file << "Evaluate : " << setw(15) << pos.GetEvaluate() << "\n";
		thread->Dump(file);
		if (file) Observer::PrintSearchReport(file);
		file.close();

		delete thread;
	}
	Observer::GameOver(0, 0, 0, 0);
	Observer::PrintGameReport(cout);
	file.open(playDetailStr, ios::app);
	if (file) Observer::PrintGameReport(file);
	file << "num|  eval | move\n";
	for (int i = 0; i < rms.size(); i++) {
		file << setw(3) << i << "|" << setw(6) << rms[i].value << " | " << rms[i].pv[0] << "\n";
	}
	file.close();

	cout << "\a";
	system("pause");
	return 0;
}

string GetCurrentTimeString() {
	char buffer[80];
	time_t rawtime;
	time(&rawtime);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", localtime(&rawtime));
	return string(buffer);
}