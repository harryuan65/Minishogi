#include <iostream>
#include <filesystem>

#include "Observer.h"
#include "usi.h"

using namespace std;
using namespace USI;
namespace fs = std::experimental::filesystem;

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
				fs::create_directory("log");
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
	vector<uint64_t> initKey;

	//uint64_t lmrTest[162];
	//uint64_t total_lmrTest[162];
	//int aspFail[6];
	//int aspTime[6];

	void StartSearching() {
		for (int i = 0; i < COUNT; i++)
			data[i] = 0;
		//for (int i = 0; i < 70; i++)
		//	lmrTest[i] = 0;
		start_time = now();
	}

	void EndSearching() {
		data[searchNum]++;
		data[searchTime] += now() - start_time;
		for (int i = 0; i < COUNT; i++)
			game_data[i] += data[i];
		//for (int i = 0; i < 70; i++)
		//	total_lmrTest[i] += lmrTest[i];
	}

	void GameStart() {
		for (int i = 0; i < COUNT; i++)
			game_data[i] = 0;
	}

	void GameOver(bool _winner, bool isSwap) {
		gameNum++;
		winner = (Winner)(_winner != isSwap);
		for (int i = 0; i < COUNT; i++)
			total_data[i] += game_data[i];
	}

	void PrintData(std::ostream &os, uint64_t *d) {
		/*uint64_t maxLMR = *max_element(total_lmrTest, total_lmrTest + 162);
		cout << "MAX LMR : " << maxLMR << endl;
		for (int i = 0; i < 162; i++)
			os << (double)Observer::total_lmrTest[i] / maxLMR << endl;*/

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
		os << " Search depths           : " << setw(10) << Options["Depth"] << "\n";
		os << " Search nums             : " << setw(10) << game_data[searchNum] << "\n";
		PrintData(os, game_data);
		//os << "Aspiration failed rate : ";
		//for (int i = 0; i < 6; i++)
		//	 os << (int)ceil((float)Observer::aspFail[i] * 100 / Observer::aspTime[i]) << "% ";
		os << "\n";

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
#ifndef REFUTATION_DISABLE
ss << "Refutation          : Enable\n";
#else
ss << "Refutation          : Disable\n";
#endif
		return ss.str();
	}
}