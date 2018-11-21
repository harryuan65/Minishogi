#include <string>
#include <windows.h>

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
						kifu.back().second = (Value)int(f * (int)PIECE_SCORE[PAWN]);
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

	//uint64_t drops;

	PerftResult() { memset(this, 0, sizeof(PerftResult)); }
};

/* perft 7
	START_POS (with sennichite)
Depth      Nodes   Captures Promotions     Checks Checkmates
	1         14          1          0          1          0
	2        181         19          0         12          0
	3       2512        291         28        169          0
	4      35401       4462        364       2548          1
	5     531949      68763       9532      40221        294
	6    8258848    1129440     152940     672343       4049
	7  132212131   18407122    3285268   11052933     140048

	START_POS (without sennichite)
Depth      Nodes   Captures Promotions     Checks Checkmates
	1         14          1          0          1          0
	2        181         19          0         12          0
	3       2512        291         28        169          0
	4      35401       4462        364       2548          1
	5     533203      68763       9532      40221        294
	6    8276188    1131064     152940     673542       4030
	7  132680617   18435252    3287520   11069694     140048
*/
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
		Move moveList[TOTAL_GENE_MAX_MOVES], *end;
		end = pos.GetLegalMoves(moveList);
		for (Move* m = moveList; m < end; m++) {
			pos.DoMove(*m);
			perft(pos, *m, depth - 1, r);
			pos.UndoMove();
		}
	}
}

void USI::perft(Minishogi &pos, std::istringstream& ss_cmd) {
	int depth = 5;
	string token;

	ss_cmd >> depth;

	cout << "Depth      Nodes   Captures Promotions     Checks Checkmates" << endl;
	for (int i = 1; i <= depth; i++) {
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
	string src_dir = "D:/Nyanpass Project/Training Kifu/";
	string dst_dir = "position/";
	string src_file = "20181023-1.pgn";
	int game_per_file = 50, sample_rate = 20;
	string token;

	while (ss_cmd >> token) {
		if (token == "src_dir") {
			getline(ss_cmd, token, '\"');
			getline(ss_cmd, src_dir, '\"');
		}
		else if (token == "dst_path") {
			getline(ss_cmd, token, '\"');
			getline(ss_cmd, dst_dir, '\"');
		}
		else if (token == "src_file")      ss_cmd >> src_file;
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

	cout << "src_path " << src_dir + src_file << " game_per_file " << game_per_file << " sample_rate " << sample_rate << "\n";

	CreateDirectory(dst_dir.c_str(), NULL);
	ifstream ifile(src_dir + src_file);
	ofstream ofile(dst_dir + "opening-1.pgn");
	if (!ifile) {
		cout << "Error : Can't find source pgn file" << endl;
		return;
	}
	if (!ofile) {
		cout << "Error : Can't open destination pgn file" << endl;
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
					if (!ofile) ofile.open(dst_dir + "opening-" + to_string(openingCnt / game_per_file + 1) + ".pgn");;
					bool skipFlag = false;
					len = (int)ifile.tellg() - start;
					ifile.seekg(start);
					for (int i = 0; i < len; i++) {
						ifile >> noskipws >> c >> skipws;
						if (c == '{') skipFlag = true;
						if (!skipFlag) ofile << c;
						if (c == '}') skipFlag = false;
						if (c == '\n') i++;
					}
					ofile << "\n\n";
					nextSamplePly += sample_rate;
					if (++openingCnt % game_per_file == 0) ofile.close();
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

	string src_name = src_file.substr(0, src_file.find('.'));
	if (!ofile) {
		cout << "Error : Can't open destination trn file" << endl;
		return;
	}
	for (int i = 1; i < (float)openingCnt / game_per_file + 1; i++) {
		ofile.open(dst_dir + "Tourney_kifugene_" + to_string(i) + ".trn");
		ofile << "-participants {Shokidoki UEC9+\n"
			"Shokidoki UEC9+\n"
			"}\n"
			"-seedBase 523836227\n"
			"-tourneyType 0\n"
			"-tourneyCycles 1\n"
			"-defaultMatchGames " << game_per_file << "\n"
			"-syncAfterRound false\n"
			"-syncAfterCycle true\n"
			"-saveGameFile \"D:\\Winboard 4.8.0\\WinBoard\\tournament\\" << src_name << " UEC9+-" << i << ".pgn\"\n"
			"-loadGameFile \"D:\\Winboard 4.8.0\\WinBoard\\tournament\\opening-" << i << ".pgn\"\n"
			"-loadGameIndex -1\n"
			"-loadPositionFile \"D:\\Winboard 4.8.0\\WinBoard\\mini.fen\"\n"
			"-loadPositionIndex -1\n"
			"-rewindIndex 0\n"
			"-usePolyglotBook true\n"
			"-polyglotBook \"minibooklet.bin\"\n"
			"-bookDepth 12\n"
			"-bookVariation 50\n"
			"-discourageOwnBooks false\n"
			"-defaultHashSize 64\n"
			"-defaultCacheSizeEGTB 4\n"
			"-ponderNextMove true\n"
			"-smpCores 1\n"
			"-mps 40\n"
			"-tc 30\n"
			"-inc 0.00\n"
			"-results \"\"\n";
		ofile.close();
	}
	cout << "make " << openingCnt << " position success" << endl;
}

/*void fix_opening() {
	string src_path = "D:/Nyanpass Project/Training Kifu/20181023-1.pgn";
	int game_per_file = 20, sample_rate = 30;
	vector<int> split;
	string token;

	int start = 0, len = 0;
	string line;
	float f;
	char c;
	int ply = 0;
	int nextSamplePly = sample_rate;

	ifstream ifile(src_path);
	if (!ifile) {
		cout << "Error : Can't find source file" << endl;
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
					nextSamplePly += sample_rate;
					split.push_back(ply);
				}
			}
			getline(ifile, token);
			ply = 0;
			nextSamplePly = sample_rate;
			start = ifile.tellg();
		}
	}
	ifile.close();
	cout << split.size() << endl;

	string dst_path = "C:/Users/Nyanpass/Desktop/test_kifu/";
	ifile.open(dst_path + "20181023-1 UEC9+1.pgn");
	ofstream oofile(dst_path + "new/20181023-1 UEC9+1.pgn");
	int openingCnt = 1;

	start = ifile.tellg();
	while (split.size()) {
		if (!getline(ifile, line)) {
			if (openingCnt > 5)
				break;
			ifile.close();
			ifile.open(dst_path + "20181023-1 UEC9+" + to_string(openingCnt) + ".pgn");
			ifile.seekg(start);
			while (ifile >> noskipws >> c >> skipws) {
				oofile << c;
			}
			ifile.close();
			oofile.close();
			openingCnt++;
			ifile.open(dst_path + "20181023-1 UEC9+" + to_string(openingCnt) + ".pgn");
			oofile.open(dst_path + "new/20181023-1 UEC9+" + to_string(openingCnt) + ".pgn");
			start = ifile.tellg();
			continue;
		}
		istringstream iss(line);
		iss >> token;
		bool isSkip = false;
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
				if (!isSkip && split.empty())
					cout << "error" << endl;
				if (!isSkip && ++ply == split.front()) {
					split.erase(split.begin());
					len = (int)ifile.tellg() - start;
					ifile.seekg(start);
					for (int i = 0; i < len; i++) {
						ifile >> noskipws >> c >> skipws;
						oofile << c;
						if (c == '\n') i++;
					}
					isSkip = true;
					start = -1;
				}
				if (ifile.peek() == '{') {
					ifile >> c;                 // {
					getline(ifile, token, '/'); // +0.68/
					istringstream ss(token);
					ss >> f;
					getline(ifile, token, '}'); // 18 5:51}
				}
				if (start == -1)
					start = ifile.tellg();
			}
			ply = 0;
		}
	}
	ifile.close();
	ifile.open(dst_path + "20181023-1 UEC9+" + to_string(openingCnt) + ".pgn");
	ifile.seekg(start);
	while (ifile >> noskipws >> c >> skipws) {
		oofile << c;
	}
	ifile.close();
	oofile.close();
	if (split.size())
		cout << "Fix failed " << split.size() << endl;
	else
		cout << "Fix Success" << endl;
}*/