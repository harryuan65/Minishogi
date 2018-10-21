#include <string>

#include "Observer.h"
#include "usi.h"
#include "EvaluateLearn.h"
using namespace std;

struct TimeTestThread : public Thread {
	const Value SHOKIDOKO_SKIP_MIN = Value(399 * PIECE_SCORE[PAWN]);
	string path = "board/timetest.sfen";
	int test_num = 50;

	TimeTestThread(int ttBit) : Thread(ttBit) {}

	void Test_sfen() {
		streamoff readBoardOffset = 0;
		while (pos.LoadBoard(path, readBoardOffset)) {
			if (Observer::game_data[Observer::searchNum] >= test_num)
				break;
			RootMove rm;
			cout << "Position : " << Observer::game_data[Observer::searchNum] << endl;
			cout << pos << endl;
			cout << "Evaluate : " << pos.GetEvaluate() << endl;

			Clean();
			InitSearch();
			Observer::StartSearching();
			IDAS(rm, USI::Options["Depth"]);
			Observer::EndSearching();

			Observer::PrintSearchReport(cout);
			cout << endl;
		}
	}

	void Test_pgn() {
		//vector<TimeTestResult> result;
		vector<pair<Move, Value>> kifu;
		ifstream ifile(path);
		string line, token;
		char c;
		float f;

		if (!ifile)
			return;

		while (getline(ifile, line)) {
			istringstream iss(line);
			iss >> token;
			if (token[0] == '{' || token == "[Annotator") {
				pos.Initialize();
				if (token[0] == '{')
					for (int j = 0; j < 7; j++)
						getline(ifile, line);
				while (true) {
					if (pos.GetTurn() == WHITE && token != "1.") {
						ifile >> token;             // 1.
						if (token[0] == '{')
							break;
					}
					ifile >> token;                 // Gc2
					if (token[0] == '{')
						break;

					kifu.emplace_back();
					kifu.back().first = algebraic2move(token, pos);
					ifile >> noskipws >> c >> skipws;
					if (ifile.peek() == '{') {
						ifile >> c;                 // {
						getline(ifile, token, '/'); // +0.68/
						istringstream ss(token);
						ss >> f;
						kifu.back().second = Value(int(f * PIECE_SCORE[PAWN]));
						getline(ifile, token, '}'); // 18 5:51}
					}
					else {
						// 排除mini-opening上的evaluate
						for (int i = kifu.size() - 1; i >= 0 && kifu[i].first != MOVE_NONE; i--)
							kifu[i].second = VALUE_NONE;
					}
					pos.DoMove(kifu.back().first);
				}
				kifu.emplace_back();
				kifu.back().first = MOVE_NONE;
			}
		}
		ifile.close();

		int moveCorrect = 0, errorCnt = 0;
		double sumError = 0.0;
		pos.Initialize();
		stringstream ss;
		ss << "kifu move my move my eval kifu eval search eval elapsed" << endl;
		for (auto &r : kifu) {
			if (Observer::game_data[Observer::searchNum] >= test_num)
				break;

			if (r.first == MOVE_NONE) {
				pos.Initialize();
				Clean();
				ss << "<Game Over>" << endl;
				continue;
			}

			if (r.second != VALUE_NONE) {
				RootMove rm;

				InitSearch();
				Observer::StartSearching();
				IDAS(rm, USI::Options["Depth"]);
				Observer::EndSearching();

				ss << "     " << r.first
					<< "    " << rm.pv[0]
					<< setw(8) << pos.GetEvaluate()
					<< setw(10) << r.second
					<< setw(12) << rm.value
					<< setw(8) << (float)Observer::data[Observer::searchTime] / 1000 << endl;
				moveCorrect += r.first == rm.pv[0];
				if (r.second != VALUE_ZERO &&
					r.second != VALUE_NONE &&
					r.second < SHOKIDOKO_SKIP_MIN &&
					r.second > -SHOKIDOKO_SKIP_MIN) {
					errorCnt++;
					sumError += int(r.second - rm.value)*int(r.second - rm.value);
				}
			}
			else
				ss << "     " << r.first << setw(16) << r.second << endl;
			    
			pos.DoMove(r.first);
		}
		ss << "correct rate : " << moveCorrect * 100.0 / Observer::game_data[Observer::searchNum] << "%" << endl;
		ss << "mse : " << sqrt(sumError / errorCnt) << endl;
		ss << "node per second : " << (Observer::game_data[Observer::mainNode] + Observer::game_data[Observer::quiesNode]) * 1000 / Observer::game_data[Observer::searchTime] << endl;

		cout << ss.str() << endl;
	}

	virtual void Run() {
		string ext = get_extension(path);
		Observer::GameStart();

		if (ext == "sfen" || ext == "fen") {
			Test_sfen();
		}
		else if (ext == "pgn") {
			Test_pgn();
		}
		else {
			sync_cout << "Error : Wrong file extension \"" << ext << "\"" << sync_endl;
		}

		Observer::GameOver(0, 0, 0);
		Observer::PrintGameReport(cout);
	}
};

void USI::timetest(istringstream& ss_cmd) {
	TimeTestThread *th = new TimeTestThread(USI::Options["HashEntry"]);
	string token;

	while (ss_cmd >> token) {
		if (token == "path")          ss_cmd >> th->path;
		else if (token == "test_num") ss_cmd >> th->test_num;
	}

	delete GlobalThread;
	EvaluateLearn::InitGrad();
	USI::Limits.ponder = false;
	GlobalThread = th;
	GlobalThread->StartWorking();
}

struct PerftResult {
	uint64_t nodes;
	uint64_t captures;
	uint64_t promotions;
	uint64_t checks;
	uint64_t checkmates;

	PerftResult() { memset(this, 0, sizeof(PerftResult)); }
};

void perft(Minishogi &pos, Move m, int depth, PerftResult &r) {
	if (depth == 0) {
		r.nodes++;
		if (pos.GetPrevCapture())
			r.captures++;
		if (is_promote(m))
			r.promotions++;
		if (pos.IsInChecked()) {
			r.checks++;
			if (pos.IsGameOver())
				r.checkmates++;
		}
	}
	else {
		Move moveList[TOTAL_GENE_MAX_ACTIONS], *end;
		end = pos.GetLegalMoves(moveList);
		for (Move* m = moveList; m < end; m++) {
			pos.DoMove(*m);
			perft(pos, *m, depth - 1, r);
			pos.UndoMove();
		}
	}
}

void USI::perft(Minishogi &pos, int depth) {
	cout << "Depth      Nodes   Captures Promotions     Checks Checkmates" << endl;
	for (int i = 0; i <= depth; i++) {
		PerftResult r;
		perft(pos, MOVE_NULL, i, r);

		cout << setw(5) << i << " ";
		cout << setw(10) << r.nodes << " ";
		cout << setw(10) << r.captures << " ";
		cout << setw(10) << r.promotions << " ";
		cout << setw(10) << r.checks << " ";
		cout << setw(10) << r.checkmates << " ";
		cout << endl;
	}
	cout << "perft finished" << endl;
}