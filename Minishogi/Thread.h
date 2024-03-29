#ifndef _THREAD_H_
#define _THREAD_H_

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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <iomanip>
#include <vector>

#include "Types.h"
#include "Transposition.h"
#include "Movepick.h"
#include "Minishogi.h"

typedef std::mutex Mutex;
typedef std::condition_variable ConditionVariable;

struct LimitsType;

/// Threshold used for countermoves based pruning
constexpr int CounterMovePruneThreshold = 0;

struct RootMove {
	Key rootKey;
	Move rootMove;
	Move pv[MAX_PLY + 1];
	Value value;
	int depth;
	
	RootMove() {
		rootKey = KEY_NULL;
		rootMove = MOVE_NULL;
		pv[0] = MOVE_NULL;
		value = VALUE_NONE;
		depth = 0;
	}

	RootMove(Key key, Move move) {
		rootKey = key;
		rootMove = move;
		pv[0] = MOVE_NULL;
		value = VALUE_NONE;
		depth = 0;
	}

	void Clean() {
		rootKey = KEY_NULL;
		rootMove = MOVE_NULL;
		pv[0] = MOVE_NULL;
		value = VALUE_NONE;
		depth = 0;
	}

	void operator=(const RootMove rm) { 
		int i = 0;
		do {
			pv[i] = rm.pv[i];
		} while (rm.pv[i++] != MOVE_NULL);
		rootKey = rm.rootKey;
		rootMove = rm.rootMove;
		value = rm.value;
		depth = rm.depth;
	}
};

struct Stack {
	Move pv[MAX_PLY + 1];
	PieceToHistory* contHistory;
	int ply;
	Move currentMove;
	//Move excludedMove;
	//Move killers[2];
	//Value staticEval;
	//int statScore;
	int moveCount;
	bool nmp_flag;
	bool lmr_flag;
};

class Thread {
public:
	ButterflyHistory mainHistory;
	CapturePieceToHistory captureHistory;
	PieceToHistory contHistory[PIECE_NB][SQUARE_NB];
	//CounterMoveHistory counterMoves;
	//int selDepth;
	int maxCheckPly;

	Thread();
	~Thread();
	void Clean();

	void Search(RootMove &rm, int depth);
	void IDAS(RootMove &rm, int depth);
	void PreIDAS();

	bool IsStop();
	bool CheckStop(Key rootKey = KEY_NULL);

	void InitSearch();
	void StartWorking();
	void WaitWorking();
	void StartSearching(const Minishogi &rootPos, const LimitsType& limits);
	void IdleLoop();
	void Stop();
	virtual void Run();

protected:
	Minishogi pos;
	bool isExit;

private:
	Mutex mutex;
	ConditionVariable cv;
	std::thread stdThread;
	Stack stack[MAX_PLY + 7], *ss;

	RootMove bestMove;
	std::vector<RootMove> rootMoves;
	bool isSearching, isFinishPonder;
	std::atomic_bool isStop;
};

extern Thread *GlobalThread;

inline bool Thread::IsStop() { 
	return isStop || isExit; 
}

inline void Thread::Stop() {
	isStop = true;
}

#endif