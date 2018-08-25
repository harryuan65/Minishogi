/*
* 正常流程 : 
* GameLoop : Set enemy move to Thread 0
* GameLoop : Waiting move from Thread 0
* Thread 0 : Stop Presearch
* Thread 0 : IDAS (or) Hit rootMoves
* Thread 0 : Domove
* Thread 0 : Presearching
* GameLoop : Thread 0 Finished
* ......等別人動.......
* 回到第一步
*/

#ifndef _THREAD_H_
#define _THREAD_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

#include "Types.h"
#include "Movepick.h"
#include "Minishogi.h"
#include "Observer.h"
using namespace Evaluate;

typedef std::mutex Mutex;
typedef std::condition_variable ConditionVariable;

/// Threshold used for countermoves based pruning
constexpr int CounterMovePruneThreshold = 0;

struct RootMove {
	Move enemyMove = MOVE_ILLEGAL;
	Move pv[MAX_PLY + 1];
	Value value = VALUE_NONE;
	int depth = 0;
	unsigned int nodes = (1 << 31);
	float effectBranch = 0;

	string PV() {
		stringstream ss;
		ss << setiosflags(ios::fixed) << setprecision(2);
		ss << "Depth " << setw(2) << depth << ", ";
		ss << "Value " << setw(6) << value << ", ";
		ss << "Nodes " << setw(8) << nodes << ", ";
		ss << "Effect Branch " << setw(4) << effectBranch << ",\nPV : ";
		for (int i = 0; pv[i] != MOVE_NULL; i++)
			ss << pv[i] << " ";
		return ss.str();
	}
};

struct Stack {
	Move pv[MAX_PLY + 1];
	PieceToHistory* contHistory;
	int ply;
	Move currentMove; // 改用pos裡的
	//Move excludedMove;
	Move killers[2];
	//Value staticEval;
	//int statScore;
	int moveCount;
	bool nmp_flag;
	bool lmr_flag;
};

class Thread {
public:
	//Evaluater *evaluater;
	ButterflyHistory mainHistory;
	CapturePieceToHistory captureHistory;
	PieceToHistory contHistory[PIECE_NB][SQUARE_NB];
	CounterMoveHistory counterMoves;

	std::atomic_bool isStop = false, isReject = false; //private

	Thread(const Minishogi &m, Color c);
	~Thread();
	void Start();

	void IDAS(RootMove &rm, int depth, bool isCompleteSearch);
	void PreIDAS();

	bool IsStop();
	uint64_t GetSearchDuration();
	bool CheckStop(Move em);
	void SetEnemyMove(Move m);
	RootMove GetBestMove();
	void Dump(ostream &os);

	void IdleLoop();

private:
	Mutex mutex;
	ConditionVariable cv;
	thread *stdThread;
	Stack stack[MAX_PLY + 7], *ss;

	Color us;
	Minishogi rootPos;
	RootMove bestMove;
	vector<RootMove> rootMoves;
	int beginTime = 0;
	bool isExit = false;
	bool finishDepth = false;

	string log = "";
};

inline bool Thread::IsStop() { 
	return isStop || isReject || isExit; 
}

inline uint64_t Thread::GetSearchDuration() { 
	return beginTime ? clock() - beginTime : 0; 
}

#endif