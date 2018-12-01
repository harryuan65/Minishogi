#include <iostream>
#include <fstream>
#include <windows.h>

#include "Observer.h"
#include "usi.h"

using namespace std;
using namespace USI;

namespace Observer {
	struct Tie : public streambuf {
		Tie(streambuf* buf_, streambuf* log_) : buf(buf_), log(log_) {}

		int sync() { return log->pubsync(), buf->pubsync(); }
		int overflow(int c) { return write(buf->sputc((char)c), "<< "); }
		int underflow() { return buf->sgetc(); }
		int uflow() { return write(buf->sbumpc(), ">> "); }

		int write(int c, const char* prefix) {
			static int last = '\n';
			if (last == '\n')
				log->sputn(prefix, 3);
			return last = log->sputc((char)c);
		}

		streambuf *buf, *log;
	};

	struct Logger {
		static void start(bool b) {
			static Logger log;

			if (b && !log.file.is_open()) {
				CreateDirectory("log", NULL);
				log.file.open("log/" + GetTimeStamp() + "_io_log.txt", ifstream::out);
				cin.rdbuf(&log.in);
				cout.rdbuf(&log.out);
				cout << "start logger \"log/" + GetTimeStamp() + "_io_log.txt\"" << endl;
			}
			else if (!b && log.file.is_open()) {
				cout << "end logger \"log/" + GetTimeStamp() + "_io_log.txt\"" << endl;
				cout.rdbuf(log.out.buf);
				cin.rdbuf(log.in.buf);
				log.file.close();
			}
		}

	private:
		Tie in, out;
		ofstream file;

		Logger() : in(cin.rdbuf(), file.rdbuf()), out(cout.rdbuf(), file.rdbuf()) {}
		~Logger() { start(false); }
	};
	void startLogger(bool b) { Logger::start(b); }

	// 單一盤面搜尋結果
	uint64_t data[DataType::COUNT];

	// 整局結果
	uint64_t game_data[DataType::COUNT];

	Winner winner;
	TimePoint start_time;

	// 全部結果
	uint64_t total_data[DataType::COUNT] = { 0 };
	uint32_t gameNum = 0;
	uint32_t player1WinNum = 0;
	uint32_t player2WinNum = 0;
	vector<Winner> winnerTable1;
	vector<Winner> winnerTable2;
	vector<uint64_t> initKey;
	vector<uint32_t> kifuHash1;
	vector<uint32_t> kifuHash2;


	void StartSearching() {
		for (int i = 0; i < COUNT; i++)
			data[i] = 0;
		start_time = now();
	}

	void EndSearching() {
		data[searchNum]++;
		data[searchTime] += now() - start_time;
		for (int i = 0; i < COUNT; i++)
			game_data[i] += data[i];
	}

	void GameStart() {
		for (int i = 0; i < COUNT; i++)
			game_data[i] = 0;
	}

	void GameOver(bool _winner, bool isSwap, uint32_t _kifuHash) {
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

	void PrintData(std::ostream &os, uint64_t *d) {
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

	void PrintSearchReport(ostream &os) {
		if (!os) return;
		if (data[scoutGeneNums] == 0) data[scoutGeneNums] = 1;
		os << "Search Deapth            : " << setw(10) << Options["Depth"] << "\n";
		PrintData(os, data);
	}

	void PrintGameReport(ostream &os) {
		if (!os) return;
		if (!game_data[searchNum]) return;
		os << "Game" << setw(3) << gameNum - 1 << "\n";
		os << "Game Result :\n";
		os << " Winner                  : " << setw(10) << (winner ? "Player2" : "Player1") << "\n";
		//os << " Kifu hashcode           : " << setw(10) << hex << kifuHash.back() << dec << "\n";
		os << " Search depths           : " << setw(10) << Options["Depth"] << "\n";
		os << " Search nums             : " << setw(10) << game_data[searchNum] << "\n";
		PrintData(os, game_data);
		os << endl;
	}

	void PrintTotalReport(ostream &os) {
		if (!os) return;
		if (gameNum == 0) return;
		os << "Game Result :\n";
		os << " Search depths           : " << setw(10) << Options["Depth"] << "\n";
		os << " Search nums             : " << setw(10) << total_data[searchNum] << "\n";
		PrintData(os, total_data);
		os << endl;
	}

	void PrintWinnerReport(ostream &os) {
		if (!os) return;
		if (gameNum == 0) return;
		os << "Total Result :\n";
		os << " Game play nums          : " << setw(10) << gameNum << "\n";
		os << " Player 1 win nums       : " << setw(10) << player1WinNum << "\n";
		os << " Player 2 win nums       : " << setw(10) << player2WinNum << "\n";
		os << "Game | A | B |  A kifu  |  B kifu  | Game\n";
		for (int i = 0; i < winnerTable1.size(); i++) {
			os << setw(4) << i << " | " << (winnerTable1[i] ? "-" : "+") << " | ";
			if (i < winnerTable2.size()) {
				os << (winnerTable2[i] ? "-" : "+") << " | ";
			}
			os << hex << setw(8) << kifuHash1[i] << dec << " | ";
			if (i < winnerTable2.size()) {
				os << hex << setw(8) << kifuHash2[i] << dec << " | ";
				os << setw(4) << i + winnerTable1.size();
			}
			os << "\n";
		}
		os << "(+:Player 1 win,-:Player 2 Win)\n";
		os << endl;
	}

	string GetSettingStr() {
		stringstream ss;
		ss << "AI Version          : " << AI_NAME << "\n";
		ss << "Main Depth          : " << Options["Depth"] << "\n";
		//ss << "Time Limit          : " << (limitTime ? to_string(limitTime) + " ms" : "Disable") << "\n";
#ifndef BACKGROUND_SEARCH_DISABLE
		ss << "Background Search   : Enable\n";
#else
		ss << "Background Search   : Disable\n";
#endif
#ifdef KPPT_DISABLE
		ss << "Evaluater           : Material, Pinner\n";
#elif defined PIN_DISABLE
		ss << "Evaluater           : KPPT(" << (string)Options["EvalDir"] << "), Material\n";
#else
		ss << "Evaluater           : KPPT(" << (string)Options["EvalDir"] << "), Material, Pin\n";
#endif
#ifdef TRANSPOSITION_DISABLE
		ss << "Transposition Table : Enable\n";
#else
#ifdef ENEMY_ISO_TT
		ss << "Transposition Table : Enemy Isomorphism\n";
#else
		ss << "Transposition Table : Single Hashcode\n";
#endif
		ss << "Transposition Size  : 2^" << Options["HashEntry"] << "\n";
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