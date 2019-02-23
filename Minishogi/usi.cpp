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
#include "TimeManage.h"

using namespace std;
using namespace Evaluate;

namespace USI {
	OptionsMap Options;
	LimitsType Limits;
}

void USI::position(Minishogi &pos, istringstream &ss_cmd) {
    string token, sfen;
	Move move;

	ss_cmd >> token;
    if (token == "startpos") {     
        sfen = Minishogi::START_SFEN;
		ss_cmd >> token;
    }
    else if (token == "sfen")
        while (ss_cmd >> token && token != "moves")
            sfen += token + " ";
    else
        return;

    pos.Initialize(sfen);
    while ((ss_cmd >> token) && (move = usi2move(token, pos.GetTurn()))) {
		if (!pos.PseudoLegal(move)) {
			sync_cout << "info string error move " << token << sync_endl;
			break;
		}
        pos.DoMove(move);
		pos.GetEvaluate();
    }
	sync_cout << pos << "\nEvaluate : " << pos.GetEvaluate() << sync_endl;
}

void USI::go(const Minishogi &pos, istringstream& ss_cmd) {
    string token;
    LimitsType limits;
    limits.start_time = now();
	limits.rootKey = pos.GetKey();

    while (ss_cmd >> token) {
		if (token == "searchmoves")
			while (ss_cmd >> token)
				limits.search_moves.push_back(usi2move(token, Turn(pos.GetTurn() ^ (limits.search_moves.size() % 2))));
        else if (token == "btime")    ss_cmd >> limits.time[WHITE];
        else if (token == "wtime")    ss_cmd >> limits.time[BLACK];
        else if (token == "binc")     ss_cmd >> limits.inc[WHITE];
        else if (token == "winc")     ss_cmd >> limits.inc[BLACK];
        else if (token == "movetime") ss_cmd >> limits.move_time;
        else if (token == "byoyomi")  ss_cmd >> limits.byoyomi;
        else if (token == "depth")    ss_cmd >> limits.depth;
        else if (token == "nodes")    ss_cmd >> limits.nodes;
        else if (token == "ponder")   limits.ponder   = true;
        else if (token == "infinite") limits.infinite = true;
    }

	GlobalThread->StartSearching(pos, limits);
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

	cout << Observer::GetSettingStr() << endl;

    Minishogi pos(nullptr);
	ExtMove moveList[SINGLE_GENE_MAX_MOVES];
    string cmd, token;
	pos.Initialize();

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
			if (GlobalThread) GlobalThread->Stop();
			Observer::GameOver(~pos.GetTurn(), false);
			Observer::PrintGameReport(cout);
		}
		else if (token == "stop") {
			if (GlobalThread) GlobalThread->Stop();
        }
        else if (token == "usi") {
            sync_cout << "id name " << AI_NAME
                << "\nid author KKK nya"
                << "\n" << Options
                << "\nusiok" << sync_endl;
        }
        else if (token == "ponderhit") { 
			if (!Limits.ponder) {
				sync_cout << "Error : engine is not pondering" << sync_endl;
			}
			else {
				Limits.ponder = false;
				//if (Limits.byoyomi) {
					Limits.start_time = now();
					Time.Reset();
				//}
			}
        }
        else if (token == "usinewgame") {
			Observer::GameOver(~pos.GetTurn(), false);
			Observer::PrintGameReport(cout);
			if (GlobalThread) delete GlobalThread;
			GlobalThread = new Thread();
			pos.Initialize();
			Observer::GameStart();
		}
        else if (token == "isready") { sync_cout << "readyok" << sync_endl; }
        else if (token == "setoption") { setoption(ss_cmd); }
        else if (token == "go") {
			if (!GlobalThread) GlobalThread = new Thread();
			go(pos, ss_cmd);
		}
        else if (token == "position") { position(pos, ss_cmd); }
		// custom command
        else if (token == "pos") { sync_cout << pos << "\nEvaluate : " << pos.GetEvaluate() << sync_endl; }
		else if (token == "eval") { sync_cout << "Evaluate : " << pos.GetEvaluate() << sync_endl; }
        else if (token == "atk") { sync_cout << move_list(moveList, pos.AttackGenerator(moveList), pos) << sync_endl; }
		else if (token == "move") { sync_cout << move_list(moveList, pos.MoveGenerator(moveList), pos) << sync_endl; }
		else if (token == "drop") { sync_cout << move_list(moveList, pos.HandGenerator(moveList), pos) << sync_endl; }
		else if (token == "legal") {
			ExtMove *end;
			end = pos.AttackGenerator(moveList);
			end = pos.MoveGenerator(end);
			end = pos.HandGenerator(end);
			sync_cout << move_list(moveList, end, pos) << sync_endl;
		}
        else if (token == "sfen") { sync_cout << pos.Sfen() << sync_endl; }
		else if (token == "log") { Observer::startLogger(true); }
		else if (token == "version") { sync_cout << Observer::GetSettingStr() << sync_endl;	}
		else if (token == "save_kppt") { GlobalEvaluater.Save(KPPT_DIRPATH + "/" + Observer::GetTimeStamp()); }
		else if (token == "kifulearn") { EvaluateLearn::StartKifuLearn(ss_cmd); }
		else if (token == "timetest") { timetest(ss_cmd); }
		else if (token == "perft") { perft(pos, ss_cmd); }
        else if (token == "harry") {
            if (GlobalThread && !Limits.ponder)
				GlobalThread->Stop();
        }
        else if (token == "resign") {
            sync_cout << "bestmove resign" << sync_endl;
            if (GlobalThread && !Limits.ponder)
				GlobalThread->Stop();
        }
		else if (token == "do") {
			ss_cmd >> token;
			pos.DoMove(token);
			sync_cout << pos << sync_endl;
		}
		else if (token == "ando") { 
			ss_cmd >> token;
			pos.DoMove(algebraic2move(token, pos));
			sync_cout << pos << sync_endl;
		}
		else if (token == "undo") {
			pos.UndoMove();
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
			for (char c : token) {
				if (c == '\"')
					continue;
				t = !t;
				if (c == '+' || c == '-')
					win[t ^ (c == '+')]++;
			}
			sync_cout << win[0] << " : " << win[1] << "\n"
				<< std::setiosflags(std::ios::fixed) << std::setprecision(1) 
				<< win[0] * 100.0 / (win[0] + win[1]) << "% : "
				<< win[1] * 100.0 / (win[0] + win[1]) << "%" << sync_endl;
		}
		else if (token == "make_opening") {
			make_opening(ss_cmd);
		}
		else if (token == "tt_read") { // TODO : value_from_tt
			bool ttHit;
			TTentry *tte = GlobalTT.Probe(pos.GetKey(), ttHit);
			if (ttHit)
				sync_cout << "Value : " << (pos.GetTurn() ? -tte->value : tte->value)
					<< "\nDepth : " << (int)tte->depth
					<< "\nMove : " << tte->move << sync_endl;
			else 
				sync_cout << "tt not found" << sync_endl;

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

	if (abs(v) < VALUE_SENNI_IN_MAX_COUNT && Options["EvalStandardize"])
		ss << "cp " << v * 100 / PIECE_SCORE[PAWN];
	else if (abs(v) < VALUE_MATE_IN_MAX_PLY)
		ss << "cp " << v;
    else
        ss << "mate " << (v > 0 ? VALUE_MATE - v : -VALUE_MATE - v);

    return ss.str();
}

string USI::pv(const RootMove &rm, const Thread &th, Value alpha, Value beta) {
	stringstream ss;
	int elapsed = Time.Elapsed() + 1;
	uint64_t nodes = Observer::data[Observer::mainNode] + Observer::data[Observer::quiesNode];

	if (ss.rdbuf()->in_avail()) // Not at first line
		ss << "\n";

	ss << "info"
		<< " depth " << rm.depth
		//<< " seldepth " << th.selDepth
		<< " multipv " << 1
		<< " score " << USI::value(rm.value)
		<< (rm.value >= beta ? " lowerbound" : rm.value <= alpha ? " upperbound" : "")
		<< " nodes " << nodes
		<< " nps " << nodes * 1000 / elapsed;

	if (elapsed > 1000)
		ss << " hashfull " << GlobalTT.HashFull();

	ss << " time " << elapsed
		<< " pv";

	for (int i = 0; rm.pv[i] != MOVE_NULL; i++)
		ss << " " << rm.pv[i];

	return ss.str();
}

string USI::move_list(ExtMove *begin, ExtMove *end, Minishogi &pos) {
	stringstream ss;
	int cnt = 0;
	for (; begin < end; begin++)
		if (!pos.IsInCheckedAfter(begin->move)) {
			ss << begin->move << " ";
			cnt++;
		}
	return "size:" + to_string(cnt) + " " + ss.str();
}