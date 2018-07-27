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
	streamoff readBoardOffset = 0;
	fstream file;
	string playDetailStr = TIMETEST_PATH + GetCurrentTimeString() + "_TimeTest.txt";

	SetConsoleTitle(L"Minishogi (TimeTest) - " AI_VERSION);
	sync_cout << "AI Version : " << AI_VERSION << "\n" << Search::GetSettingStr() << sync_endl;

	sync_cout << "��J�j�M���`��" << sync_endl;
	cin >> Observer::depth;

#ifndef TRANSPOSITION_DISABLE
	int size;
	sync_cout << "��J�P����entry�ƶq(2^n)" << sync_endl;
	cin >> size;
	Transposition::TPSize = 1 << size;
#endif

	sync_cout << "��J�ɶ�����(ms 0���L����)" << sync_endl;
	cin >> Observer::limitTime;

	sync_cout << Search::GetSettingStr() << "�T�w�n�}�l?" << sync_endl;
	system("pause");

	CreateDirectory(CA2W(TIMETEST_PATH), NULL);
	file.open(playDetailStr, ios::app);
	if (file) file << "AI Version : " << AI_VERSION << "\n" << Search::GetSettingStr() << endl;
	file.close();

	Zobrist::Initialize();
	Transposition::Initialize();
	Observer::GameStart();
	while (pos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
		RootMove rm;
		Transposition::Clean();
		thread = new Thread(pos, pos.GetTurn());
		pos.PrintChessBoard();

		Observer::StartSearching();
		thread->IDAS(rm, Observer::depth, false);
		Observer::EndSearching();
		delete thread;

		sync_cout << rm.PV() << sync_endl;
		Observer::PrintSearchReport(cout);
		file.open(playDetailStr, ios::app);
		pos.PrintNoncolorBoard(file);
		file << rm.PV() << endl;
		if (file) Observer::PrintSearchReport(file);
		file.close();
	}
	Observer::GameOver(0, 0, 0, 0);
	Observer::PrintGameReport(cout);
	file.open(playDetailStr, ios::app);
	if (file) Observer::PrintGameReport(file);
	file.close();

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