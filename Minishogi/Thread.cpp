#include <assert.h>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#elif __unix__
#include <unistd.h>
#endif

#include "usi.h"
#include "Types.h"
#include "Thread.h"
#include "Observer.h"
#include "TimeManage.h"

using namespace std;
using namespace USI;

Thread *GlobalThread;

Thread::Thread() : pos(this) {
	WaitWorking();

	isExit = false;
	isStop = false;
	isSearching = true;
	isFinishPonder = false;
	Clean();
	stdThread = thread(&Thread::IdleLoop, this);
}

Thread::~Thread() {
	isExit = true;
	StartWorking();
	stdThread.join();
	sync_cout << "Thread Delete" << sync_endl;
}

void Thread::Clean() {
	//selDepth = 0;
	maxCheckPly = Options["MaxCheckPly"];
	GlobalTT.Resize(USI::Options["HashEntry"]);

	//counterMoves.fill(MOVE_NULL);
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
	if (isStop || isExit
		|| (!Limits.ponder && (Limits.rootKey != rootKey || isFinishPonder))) {
		return isStop = true;
	}

	if (Limits.ponder)
		return isStop;

	int elapsed = Time.Elapsed();
	if ((!Limits.move_time && Limits.IsTimeManagement() && elapsed >= Time.GetMaximum())
		|| (Limits.move_time && elapsed >= Limits.move_time)
		|| (Limits.nodes && Observer::data[Observer::mainNode] >= Limits.nodes)) {
		return isStop = true;
	}
	return isStop;
}

void Thread::InitSearch() {
	isFinishPonder = false;
	ss = stack + 4;
	memset(ss - 4, 0, 7 * sizeof(Stack));

	int ply = pos.GetPly();
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
	Time.Init(Limits, pos.GetTurn(), ply);

	GlobalTT.Clean();
}

void Thread::StartWorking() {
	lock_guard<Mutex> lk(mutex);

	isSearching = true;
	cv.notify_one(); // Wake up the thread in idle_loop()
}

void Thread::WaitWorking() {
	unique_lock<Mutex> lk(mutex);
	cv.wait(lk, [&] { return !isSearching; });
}

void Thread::StartSearching(const Minishogi &rootPos, const LimitsType& limits) {
	isStop = true;
	WaitWorking();
	isStop = false;

	pos.Initialize(rootPos);
	Limits = limits;
	assert(pos.GetKey() == Limits.rootKey);

	StartWorking();
}

void Thread::IdleLoop() {
	while (true) {
		unique_lock<Mutex> lk(mutex);
		isSearching = false;
		cv.notify_one();
		cv.wait(lk, [&] { return isSearching; });
		
		if (isExit)
			return;

		lk.unlock();

		Run();
	}
}

void Thread::Run() {
	InitSearch();
	if (Limits.ponder) {
#ifndef BACKGROUND_SEARCH_DISABLE
		Observer::StartSearching();
		PreIDAS();

		// Select ponder result to show 'bestmove'
		for (auto& rm : rootMoves) {
			if (rm.rootKey == Limits.rootKey) {
				bestMove = rm;
				break;
			}
		}
		Observer::EndSearching(Time.Elapsed());
#endif
		// Wait for USI call 'ponderhit' or 'stop'
		while (!CheckStop())
#ifdef _WIN32
			Sleep(10);
#elif __unix__
			delay(10);
#endif
	}
	else {
		Observer::StartSearching();
		// Load ponder result
		for (auto& rm : rootMoves) {
			if (rm.rootKey == Limits.rootKey) {
				bestMove = rm;
				break;
			}
		}
		bestMove.Clean();
		bestMove.rootKey = pos.GetKey();
		IDAS(bestMove, USI::Options["Depth"]);
		Observer::EndSearching(Time.Elapsed());
	}

	// Show 'bestmove'
	if (bestMove.value == VALUE_MATE) {
		sync_cout << "bestmove win" << sync_endl;
	}
	// Debug : when checkmated position ponder miss UCI2WB will not send bestmove to winboard, then the engine can't quit (winboard think engine is pondering)
	else if (VALUE_MATE + bestMove.value == 2) { 
		sync_cout << endl << "bestmove resign" << sync_endl;
	}
	else {
		sync_cout << endl << "bestmove " << bestMove.pv[0];
		if (Options["USI_Ponder"] && bestMove.pv[0] != MOVE_NULL && bestMove.pv[1] != MOVE_NULL)
			cout << " ponder " << bestMove.pv[1];
		cout << sync_endl;
	}
}