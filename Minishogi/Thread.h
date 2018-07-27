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

typedef std::mutex Mutex;
typedef std::condition_variable ConditionVariable;

/// Threshold used for countermoves based pruning
constexpr int CounterMovePruneThreshold = 0;

struct RootMove {
	Move enemyMove = MOVE_ILLEGAL;
	Move pv[MAX_PLY + 1];
	Value value = VALUE_NONE;
	int depth = 0;

	string PV() {
		stringstream ss;
		ss << "Depth " << setw(2) << depth << ", ";
		ss << "Value " << setw(6) << value << ",\nPV : ";
		for (int i = 0; pv[i] != MOVE_NULL; i++)
			ss << pv[i] << " ";
		return ss.str();
	}

	static string PV(int depth, Value value, Move *pv) {
		stringstream ss;
		ss << "Depth " << setw(2) << depth << ",";
		ss << "Value " << setw(6) << value << ",PV ";
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
};

class Thread {
private:
	Mutex mutex;
	ConditionVariable cv;
	thread *stdThread; 
	Stack stack[MAX_PLY + 7], *ss;

	Color us;
	Minishogi rootPos;
	RootMove bestMove;
	vector<RootMove> rootMoves;
	bool isExit = false;
	int beginTime = 0;
	bool finishDepth = false;

public:
	ButterflyHistory mainHistory;
	CapturePieceToHistory captureHistory;
	PieceToHistory contHistory[CHESS_NB][SQUARE_NB];
	CounterMoveHistory counterMoves;

	std::atomic_bool isStop = false, isReject = false; //private

	Thread(const Minishogi &m, Color c);
	~Thread();
	void Start();

	void IDAS(RootMove &rm, int depth, bool isCompleteSearch);
	void PreIDAS();

	inline bool IsStop() { return isStop || isReject || isExit; }
	inline unsigned __int64 GetSearchDuration() { return beginTime ? clock() - beginTime : 0; }
	bool CheckStop(Move em);
	void SetEnemyMove(Move m);
	RootMove GetBestMove();

	void IdleLoop();
};


#endif