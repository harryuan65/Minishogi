#include <assert.h>
#include <iostream>
#include <windows.h>

#include "usi.h"
#include "Types.h"
#include "Thread.h"
#include "Observer.h"
using namespace std;
using namespace USI;

Thread *GlobalThread;

Thread::Thread(int ttSize) : pos(this), stdThread(&Thread::IdleLoop, this) {
	//int ttBit = ttSize ? log2(ttSize) + 16 : 1;
	tt.Initialize(ttSize);
	selDepth = 0;
	maxCheckPly = Options["MaxCheckPly"];

	counterMoves.fill(MOVE_NULL);
	mainHistory.fill(0);
	captureHistory.fill(0);
	for (int i = 0; i < PIECE_NB; i++)
		for (int j = 0; j < SQUARE_NB; j++)
			contHistory[i][j].fill(0);
	contHistory[NO_PIECE][0].fill(CounterMovePruneThreshold - 1);

	ss = stack + 4; // To reference from (ss-4) to (ss+2)
	memset(ss - 4, 0, 7 * sizeof(Stack));
	for (int i = 4; i > 0; i--)
		(ss - i)->contHistory = &contHistory[NO_PIECE][0]; // Use as sentinel
}

Thread::~Thread() {
	searchMutex.lock();
	isExit = true;
	isSearching = true;
	searchCV.notify_all();
	searchMutex.unlock();
	stdThread.join();
	sync_cout << "Thread Delete" << sync_endl;
}

void Thread::Clean() {
	tt.Clean();
	selDepth = 0;

	counterMoves.fill(MOVE_NULL);
	mainHistory.fill(0);
	captureHistory.fill(0);
	for (int i = 0; i < PIECE_NB; i++)
		for (int j = 0; j < SQUARE_NB; j++)
			contHistory[i][j].fill(0);
	contHistory[NO_PIECE][0].fill(CounterMovePruneThreshold - 1);

	ss = stack + 4; // To reference from (ss-4) to (ss+2)
	memset(ss - 4, 0, 7 * sizeof(Stack));
	for (int i = 4; i > 0; i--)
		(ss - i)->contHistory = &contHistory[NO_PIECE][0]; // Use as sentinel
}

bool Thread::CheckStop(Key rootKey) {
	if (isStop || isExit ||
		//(Observer::limitTime && GetSearchDuration() > Observer::limitTime) ||
		(!Limits.ponder && (Limits.rootKey != rootKey || finishDepth))) {
		isStop = true;
	}
	return isStop;
}

/*void Thread::SetEnemyMove(Key k) {
	lock_guard<Mutex> lk(moveMutex);
	bestMove.depth = 0;
	Limits.rootKey = k;
	sync_cout << "GameLoop : Set enemy move to Thread " << sync_endl;
}

RootMove Thread::GetBestMove() {
	unique_lock<Mutex> lk(moveMutex);
	if (!bestMove.depth) {
		sync_cout << "GameLoop : Waiting move from Thread " << sync_endl;
		beginTime = clock();
		moveCV.wait(lk, [&] { return bestMove.depth; });
		beginTime = 0;
		lk.unlock();
	}
	sync_cout << "GameLoop : Thread Finished" << sync_endl;
	return bestMove;
}*/

void Thread::InitSearch() {
	int ply = pos.GetPly();

	finishDepth = false;
	ss = stack + 4;
	memset(ss - 4, 0, 7 * sizeof(Stack));
	for (int i = 1; i <= 4 && i <= ply; i++) {
		(ss - i)->currentMove = pos.GetHistMove(ply - i + 1);
		pos.UndoMove();
	}
	for (int i = 4; i >= 1; i--) {
		if ((ss - i)->currentMove) {
			(ss - i)->contHistory = &contHistory[pos.GetPiece((ss - i)->currentMove)][to_sq((ss - i)->currentMove)];
			pos.DoMove((ss - i)->currentMove);
		}
		else {
			(ss - i)->contHistory = &contHistory[NO_PIECE][0];
		}
	}
	//tt.Clean();
}

void Thread::StartSearching(const Minishogi &rootPos, const LimitsType& limits) {
	if (Limits.ponder) {
		Limits.rootKey = limits.rootKey;
		Limits.ponder = false;
	}
	else {
		bestMove.Clean();
	}

	isStop = true;
	lock_guard<Mutex> lk(searchMutex);

	pos.Initialize(rootPos);
	Limits = limits;
	assert(pos.GetKey() == Limits.rootKey);

	isSearching = true;
	searchCV.notify_one(); // Wake up the thread in idle_loop()
}

void Thread::StartWorking() {
	isStop = true;
	lock_guard<Mutex> lk(searchMutex);

	isSearching = true;
	searchCV.notify_one(); // Wake up the thread in idle_loop()
}

void Thread::IdleLoop() {
	while (!isExit) {
		unique_lock<Mutex> lk(searchMutex);

		isSearching = false;

		while (!isSearching && !isExit) {
			searchCV.notify_one();
			searchCV.wait(lk, [&] { return isSearching; });
		}

		lk.unlock();
		isStop = false;

		if (!isExit)
			Run();
	}
}

void Thread::Run() {
	if (Limits.ponder) {
		rootMoves.clear();
		if (Options["FullMovePonder"] && pos.GetPly() > 0) {
			Move ponderMove = pos.GetPrevMove();
			pos.UndoMove();
			InitSearch();
			Observer::StartSearching();
			PreIDAS();
			Observer::EndSearching();
			pos.DoMove(ponderMove);
		}
		else {
			InitSearch();
			rootMoves.emplace_back();
			rootMoves[0].rootKey = pos.GetKey();
			Observer::StartSearching();
			IDAS(rootMoves[0], MAX_PLY);
			Observer::EndSearching();
		}
	}
	else {
		for (int i = 0; i < rootMoves.size(); i++) {
			if (rootMoves[i].rootKey == Limits.rootKey) {
				bestMove = rootMoves[i];
			}
		}
		InitSearch();
		bestMove.rootKey = pos.GetKey();
		Observer::StartSearching();
		IDAS(bestMove, USI::Options["Depth"]);
		Observer::EndSearching();
	}
}