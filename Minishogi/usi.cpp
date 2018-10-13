#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "usi.h"
#include "Types.h"
#include "Observer.h"
#include "Thread.h"
#include "Minishogi.h"
#include "EvaluateLearn.h"
using namespace std;
using namespace Evaluate;

namespace USI {
	OptionsMap Options;
	LimitsType Limits;
}

struct TimeTestThread : public Thread {
	string position_path = "board/timetest.sfen";

	TimeTestThread(int ttBit) : Thread(ttBit) {}

	virtual void Run() {
		streamoff readBoardOffset = 0;
		Observer::GameStart();
		while (pos.LoadBoard(position_path, readBoardOffset)) {
			RootMove rm;
			cout << "Position : " << Observer::game_data[Observer::searchNum] << endl;
			cout << pos << endl;
			cout << "Evaluate : " << pos.GetEvaluate() << endl;

			Clean();
			Observer::StartSearching();
			IDAS(rm, USI::Options["Depth"]);
			Observer::EndSearching();

			Observer::PrintSearchReport(cout);
			cout << endl;
		}

		Observer::GameOver(0, 0, 0);
		Observer::PrintGameReport(cout);
	}
};

void USI::position(Minishogi &pos, istringstream &is) {
    string token, sfen;
	Move m;

    is >> token;
    if (token == "startpos") {     
        sfen = Minishogi::START_SFEN;
        is >> token;
    }
    else if (token == "sfen")
        while (is >> token && token != "moves")
            sfen += token + " ";
    else
        return;

    pos.Initialize(sfen);
    while ((is >> token) && (m = usi2move(token, pos.GetTurn()))) {
        pos.DoMove(m);
		pos.GetEvaluate();
    }
	sync_cout << pos << sync_endl;
}

void USI::go(const Minishogi &pos, istringstream& ss_cmd) {
    string token;
    LimitsType limits;
    limits.start_time = now();
	limits.rootKey = pos.GetKey();

    while (ss_cmd >> token) {
		if (token == "searchmoves")
			while (ss_cmd >> token)
				limits.search_moves.push_back(usi2move(token, Color(pos.GetTurn() ^ (limits.search_moves.size() % 2))));
        else if (token == "btime")    ss_cmd >> limits.time[BLACK];
        else if (token == "wtime")    ss_cmd >> limits.time[WHITE];
        else if (token == "binc")     ss_cmd >> limits.inc[BLACK];
        else if (token == "winc")     ss_cmd >> limits.inc[WHITE];
        else if (token == "movetime") ss_cmd >> limits.move_time;
        else if (token == "byoyomi")  ss_cmd >> limits.byoyomi;
        else if (token == "depth")    ss_cmd >> limits.depth;
        else if (token == "nodes")    ss_cmd >> limits.nodes;
        else if (token == "ponder")   limits.ponder   = true;
        else if (token == "infinite") limits.infinite = true;
    }

	GlobalThread->StartSearching(pos, limits);
}

void USI::timetest(istringstream& ss_cmd) {
	TimeTestThread *th = new TimeTestThread(USI::Options["HashEntry"]);
	string token;

	while (ss_cmd >> token) {
		if (token == "position_path") ss_cmd >> th->position_path;
	}

	delete GlobalThread;
	EvaluateLearn::InitGrad();
	USI::Limits.ponder = false;
	GlobalThread = th;
	GlobalThread->StartWorking();
}

void USI::setoption(istringstream& ss_cmd) {
    string token, name, value;

    ss_cmd >> token;
    assert(token == "name");

    ss_cmd >> name;

    while (ss_cmd >> token && token != "value")
        name += " " + token;

    ss_cmd >> value;

    while (ss_cmd >> token)
        value += " " + token;

    if (!Options.IsLegalOption(name))
        cout << "No such option: " << name << endl;

    if (value.empty())
        Options[name] = true;
    else
        Options[name] = value;
}

void USI::loop(int argc, char** argv) {
	Observer::startLogger(true);
	Zobrist::Initialize();
	Evaluate::GlobalEvaluater.Load(USI::Options["EvalDir"]);
	GlobalThread = new Thread();
	cout << Observer::GetSettingStr() << endl;

    Minishogi pos(nullptr);
	ExtMove moveList[SINGLE_GENE_MAX_ACTIONS];
    string cmd, token;

    for (int i = 1; i < argc; i++)
        cmd += string(argv[i]) + " ";

    do {
        if (argc == 1 && !getline(cin, cmd))
            cmd = "quit";

        istringstream ss_cmd(cmd);
        token.clear();

        ss_cmd >> skipws >> token;

        // usi command
		if (token == "quit" ||
			token == "gameover") {
			GlobalThread->Stop();
			Observer::GameOver(~pos.GetTurn(), false, pos.GetKifuHash());
			Observer::PrintGameReport(cout);
		}
		else if (token == "stop") {
			GlobalThread->Stop();
        }
        else if (token == "usi") {
            sync_cout << "id name " << AI_NAME
                << "\nid author KKK nya"
                << "\n" << Options
                << "\nusiok" << sync_endl;
        }
        else if (token == "ponderhit") { 
			Limits.ponder = false;
            if (Limits.byoyomi) {
                Limits.start_time = now();
                //Time.reset();
            }
			GlobalThread->StartSearching(pos, Limits);
        }
        else if (token == "usinewgame") {
			Observer::GameOver(~pos.GetTurn(), false, pos.GetKifuHash());
			Observer::PrintGameReport(cout);
			delete GlobalThread;
			GlobalThread = new Thread(USI::Options["HashEntry"]);
			pos.Initialize();
			Observer::GameStart();
		}
        else if (token == "isready") { sync_cout << "readyok" << sync_endl; }
        else if (token == "setoption") { setoption(ss_cmd); }
        else if (token == "go") { go(pos, ss_cmd); }
        else if (token == "position") { position(pos, ss_cmd); }
		// custom command
        else if (token == "pos") { sync_cout << pos << sync_endl; }
		else if (token == "eval") { sync_cout << "eval = " << pos.GetEvaluate() << sync_endl; }
        else if (token == "atk") { sync_cout << move_list(moveList, pos.AttackGenerator(moveList), pos) << sync_endl; }
		else if (token == "move") { sync_cout << move_list(moveList, pos.MoveGenerator(moveList), pos) << sync_endl; }
		else if (token == "drop") { sync_cout << move_list(moveList, pos.HandGenerator(moveList), pos) << sync_endl; }
        else if (token == "sfen") { sync_cout << pos.Sfen() << sync_endl; }
		else if (token == "log") { Observer::startLogger(true); }
		else if (token == "version") { sync_cout << Observer::GetSettingStr() << sync_endl;	}
		else if (token == "save_kppt") { GlobalEvaluater.Save(KPPT_DIRPATH + "/" + Observer::GetTimeStamp()); }
		else if (token == "kifulearn") { EvaluateLearn::StartKifuLearn(ss_cmd); }
		else if (token == "timetest") { timetest(ss_cmd); }
		else if (token == "perft") {}
        else if (token == "harry") {
            if (!Limits.ponder)
				GlobalThread->Stop();
        }
        else if (token == "resign") {
            sync_cout << "bestmove resign" << sync_endl;
            if (!Limits.ponder)
				GlobalThread->Stop();
        }
		else if (token == "domove") {
			ss_cmd >> token;
			pos.DoMove(token);
			sync_cout << pos << sync_endl;
		}
		else if (token == "an_domove") { 
			ss_cmd >> token;
			pos.DoMove(algebraic2move(token, pos));
			sync_cout << pos << sync_endl;
		}
		else if (token == "fen2sfen") {
			char c;
			ss_cmd >> c >> token;
			sync_cout << fen2sfen(token) << sync_endl;
		}
		else if (token == "results") {
			bool t = 0;
			int win[2] = { 0 };
			ss_cmd >> token;
			for (char c : token)
				if (c == '+' || c == '-')
					win[(t = !t) ^ (c == '+')]++;
			sync_cout << win[0] << " : " << win[1] << sync_endl;
		}
        else { sync_cout << "unknown command : " << cmd << sync_endl; }

        if (argc > 1)
            argc = 1;
    } while (cmd != "quit");

	delete GlobalThread;
}

string USI::value(Value v) {
    assert(-SCORE_INFINITE < v && v < SCORE_INFINITE);
    stringstream ss;

	if (abs(v) < VALUE_MATE_IN_MAX_PLY)
		ss << "cp " << (Options["EvalStandardize"] ? v * 100 / PIECE_SCORE[PAWN] : v);
    else
        ss << "mate " << (v > 0 ? VALUE_MATE - v : -VALUE_MATE - v);

    return ss.str();
}

string USI::pv(const RootMove &rm, const Thread &th, Value alpha, Value beta) {
	stringstream ss;
	int elapsed = int(now() - Observer::start_time + 1);
	uint64_t nodes = Observer::data[Observer::mainNode] + Observer::data[Observer::quiesNode];

	if (ss.rdbuf()->in_avail()) // Not at first line
		ss << "\n";

	ss << "info"
		<< " depth " << rm.depth
		<< " seldepth " << th.selDepth
		<< " multipv " << 1
		<< " score " << USI::value(rm.value)
		<< (rm.value >= beta ? " lowerbound" : rm.value <= alpha ? " upperbound" : "")
		<< " nodes " << nodes
		<< " nps " << nodes * 1000 / elapsed;

	if (elapsed > 1000)
		ss << " hashfull " << th.tt.HashFull();

	ss << " time " << elapsed
		<< " pv";

	for (int i = 0; rm.pv[i] != MOVE_NULL; i++)
		ss << " " << rm.pv[i];

	return ss.str();
}

string USI::move_list(ExtMove *begin, ExtMove *end, Minishogi &pos) {
	stringstream ss;
	ss << "size: " << end - begin << " ";
	for (; begin < end; begin++) {
		if ((from_sq(begin->move) >= BOARD_NB || !pos.IsInCheckedAfter(begin->move)) && type_of(pos.GetChessOn(to_sq(begin->move))) != KING)
			ss << begin->move << " ";
	}
	return ss.str();
}