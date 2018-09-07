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

/// Threshold used for countermoves based pruning
constexpr int CounterMovePruneThreshold = 0;

struct RootMove {
	Move enemyMove = MOVE_NONE;
	Move pv[MAX_PLY + 1];
	Value value = VALUE_NONE;
	int depth = 0;
	uint64_t nodes = (1 << 31);
	float effectBranch = 0;

	std::string PV() {
		std::stringstream ss;
		ss << std::setiosflags(std::ios::fixed) << std::setprecision(2);
		ss << "Depth " << std::setw(2) << depth << ", ";
		ss << "Value " << std::setw(6) << value << ", ";
		ss << "Nodes " << std::setw(8) << nodes << ", ";
		ss << "Effect Branch " << std::setw(4) << effectBranch << ",\nPV : ";
		for (int i = 0; pv[i] != MOVE_NULL; i++)
			ss << pv[i] << " ";
		return ss.str();
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
	CounterMoveHistory counterMoves;
	Transposition tt;

	Thread(const Minishogi &m, Color c, int ttBit);
	~Thread();
	void Start();

	void Search(RootMove &rm, int depth);
	// isFullSearch : return Correct value with fix depth
	void IDAS(RootMove &rm, int depth, bool isFullSearch);
	void PreIDAS();

	bool IsStop();
	bool IsExit();
	bool CheckStop(Move em);
	void SetEnemyMove(Move m);
	uint64_t GetSearchDuration();
	RootMove GetBestMove();
	const Minishogi& GetMinishogi() const;

	void DoMove(Move move);
	void UndoMove();
	void Dump(std::ostream &os);
	void IdleLoop();

private:
	Mutex mutex;
	ConditionVariable cv;
	std::thread *stdThread;
	Stack stack[MAX_PLY + 7], *ss;

	Color us;
	Minishogi rootPos;
	RootMove bestMove;
	std::vector<RootMove> rootMoves;
	int beginTime = 0;
	std::atomic_bool isStop = false, isReject = false;
	bool isExit = false;
	bool finishDepth = false;

	std::string resultStr;
};

inline bool Thread::IsStop() { 
	return isStop || isReject || isExit; 
}

inline bool Thread::IsExit() {
	return isExit;
}

inline uint64_t Thread::GetSearchDuration() { 
	return beginTime ? clock() - beginTime : 0; 
}

inline const Minishogi& Thread::GetMinishogi() const {
	return rootPos;
}

inline void Thread::DoMove(Move move) {
	rootPos.DoMove(move);
}

inline void Thread::UndoMove() {
	rootPos.UndoMove();
}

#endif