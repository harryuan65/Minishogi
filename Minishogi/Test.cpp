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
		vector<pair<Move, Value>> kifu;
		string line, token;
		char c;
		float f;

		ifstream ifile(path);
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
						// �ư�mini-opening�W��evaluate
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

		int i = 0, moveCorrect = 0, errorCnt = 0;
		double sumError = 0.0;
		pos.Initialize();
		stringstream ss;
		ss << " Kifu Move    My Move    My Eval  Kifu Eval SearchEval Error Eval    Elapsed" << endl;
		for (auto &r : kifu) {
			if (Observer::game_data[Observer::searchNum] >= test_num || CheckStop())
				break;

			if (r.first == MOVE_NONE) {
				pos.Initialize();
				Clean();
				ss << "<Game Over>" << endl;
				continue;
			}

			if (r.second != VALUE_NONE) {
				RootMove rm;

				cout << "#" << i++ << endl;
				cout << "position sfen " << pos.Sfen() << endl;
				InitSearch();
				Observer::StartSearching();
				IDAS(rm, USI::Options["Depth"]);
				Observer::EndSearching();

				moveCorrect += r.first == rm.pv[0];

				ss << setw(6) << r.first << (is_promote(r.first) ? "" : " ")
					<< setw(7) << rm.pv[0] << (is_promote(rm.pv[0]) ? "" : " ")
					<< setw(11) << pos.GetEvaluate()
					<< setw(11) << r.second
					<< setw(11) << rm.value;
				if (rm.value != VALUE_NONE &&
					r.second != VALUE_ZERO &&
					r.second != VALUE_NONE &&
					r.second < SHOKIDOKO_SKIP_MIN &&
					r.second > -SHOKIDOKO_SKIP_MIN) {
					errorCnt++;
					sumError += int(r.second - rm.value)*int(r.second - rm.value);
					ss << setw(11) << abs(r.second - rm.value);
				}
				else
					ss << setw(11) << " ";
				ss << setiosflags(std::ios::fixed) << setprecision(3) << setw(11) << (float)Observer::data[Observer::searchTime] / 1000 << endl;
			}
			else
				ss << setw(7) << r.first << setw(33) << r.second << endl;
			    
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
		if (token == "path") {
			getline(ss_cmd, token, '\"');
			getline(ss_cmd, token, '\"');
			th->path = token;
		}
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

void USI::make_opening(istringstream& ss_cmd) {
	string path = "D:/Nyanpass Project/Training Kifu/20181023-1.pgn";
	int game_per_file = 50, sample_rate = 20;
	string token;

	while (ss_cmd >> token) {
		if (token == "path") {
			getline(ss_cmd, token, '\"');
			getline(ss_cmd, token, '\"');
			ss_cmd >> path;
		}
		else if (token == "game_per_file") ss_cmd >> game_per_file;
		else if (token == "sample_rate")   ss_cmd >> sample_rate;
	}

	int start = 0, len = 0;
	string line;
	float f;
	char c;
	int ply = 0;
	int openingCnt = 0;
	int nextSamplePly = sample_rate;

	ofstream ofile("opening-0.pgn");
	ifstream ifile(path);
	if (!ifile) {
		cout << "Error : Can't find target" << endl;
		return;
	}

	while (getline(ifile, line)) {
		istringstream iss(line);
		iss >> token;
		if (token[0] == '{' || token == "[Annotator") {
			if (token[0] == '{')
				for (int j = 0; j < 7; j++)
					getline(ifile, line);
			while (true) {
				if (ply % 2 == 0 && token != "1.") {
					ifile >> token;             // 1.
					if (token[0] == '{')
						break;
				}
				ifile >> token;                 // Gc2
				if (token[0] == '{')
					break;

				ifile >> noskipws >> c >> skipws;
				if (ifile.peek() == '{') {
					ifile >> c;                 // {
					getline(ifile, token, '/'); // +0.68/
					istringstream ss(token);
					ss >> f;
					getline(ifile, token, '}'); // 18 5:51}
				}
				if (++ply >= nextSamplePly) {
					len = (int)ifile.tellg() - start;
					ifile.seekg(start);
					for (int i = 0; i < len; i++) {
						ifile >> noskipws >> c >> skipws;
						ofile << c;
						if (c == '\n')
							i++;
					}
					ofile << "\n\n";
					nextSamplePly += sample_rate;
					if (++openingCnt % game_per_file == 0) {
						ofile.close();
						ofile.open("opening-" + to_string(openingCnt / game_per_file) + ".pgn");;
					}
				}
			}
			getline(ifile, token);
			ply = 0;
			nextSamplePly = sample_rate;
			start = ifile.tellg();
		}
	}
	ifile.close();
	ofile.close();
	cout << "make opening finished" << endl;
}