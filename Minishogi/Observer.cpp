#include <iostream>
#include <fstream>
#include <windows.h>

#include "Observer.h"
using namespace std;

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
				cout << "start logger at \"log/" + GetTimeStamp() + "_io_log.txt\"" << endl;
			}
			else if (!b && log.file.is_open()) {
				cout << "end logger at \"log/" + GetTimeStamp() + "_io_log.txt\"" << endl;
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

	uint64_t startZobristHash;
	Winner winner;
	double gamePlayTime;
	TimePoint start_time;

	// 全部結果
	uint64_t total_data[DataType::COUNT] = { 0 };
	uint32_t gameNum = 0;
	uint32_t player1WinNum = 0;
	uint32_t player2WinNum = 0;
	std::vector<Winner> winnerTable1;
	std::vector<Winner> winnerTable2;
	std::vector<uint64_t> initKey;
	std::vector<uint32_t> kifuHash1;
	std::vector<uint32_t> kifuHash2;

	// 設定
	int limitTime = 0;
	bool isSaveRecord = true;
}