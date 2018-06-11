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
		ss << "Depth " << setw(2) << depth << ",";
		ss << "Value " << setw(6) << value << ",PV ";
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
	std::thread stdThread; 
	Stack stack[MAX_PLY + 7], *ss;

	Color us;
	Minishogi rootPos;
	//Move enemyMove = MOVE_ILLEGAL;
	RootMove bestMove;
	vector<RootMove> rootMoves;
	bool isExit = false;
	int beginTime = 0;

public:
	ButterflyHistory mainHistory;
	CapturePieceToHistory captureHistory;
	PieceToHistory contHistory[CHESS_NB][SQUARE_NB];
	CounterMoveHistory counterMoves;

	std::atomic_bool isStop = false, isReject = false; //private

	Thread(const Minishogi &m, Color c);
	~Thread();

	void IDAS(RootMove &rm, int depth);
	void PreIDAS();

	inline bool IsStop() { return isStop || isReject || isExit; }
	inline unsigned __int64 GetSearchDuration() { return beginTime ? clock() - beginTime : 0; }
	bool CheckStop(Move em);
	void SetEnemyMove(Move m);
	RootMove GetBestMove();

	void IdleLoop();
};


#endif