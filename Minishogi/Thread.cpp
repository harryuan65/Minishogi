#include <assert.h>
#include <iostream>
#include <windows.h>
#include "Thread.h"

Thread::Thread(const Minishogi &m, Color c) : us(c) {
	rootPos.Set(m, this);
	counterMoves.fill(MOVE_NULL);
	mainHistory.fill(0);
	captureHistory.fill(0);
	for (int i = 0; i < PIECE_NB; i++)
		for (int j = 0; j < SQUARE_NB; j++)
			contHistory[i][j].fill(0);
	contHistory[NO_PIECE][0].fill(CounterMovePruneThreshold - 1);
	ss = stack + 4; // To reference from (ss-4) to (ss+2)
	std::memset(ss - 4, 0, 7 * sizeof(Stack));
	for (int i = 4; i > 0; i--)
		(ss - i)->contHistory = &contHistory[NO_PIECE][0]; // Use as sentinel
}

Thread::~Thread() {
	isExit = true;
	if (stdThread)
		stdThread->join();
}

void Thread::Start() {
	if (!stdThread)
		stdThread = new thread(&Thread::IdleLoop, this);
}

bool Thread::CheckStop(Move em) {
	if (isStop || isReject || isExit ||
		(Observer::limitTime && GetSearchDuration() > Observer::limitTime) ||
		(bestMove.enemyMove != MOVE_ILLEGAL && (em != bestMove.enemyMove || finishDepth))) {
		isStop = true;
	}
	return isStop;
}

void Thread::SetEnemyMove(Move m) {
	if (m) {
		std::lock_guard<Mutex> lk(mutex);
		bestMove.depth = 0;
		bestMove.enemyMove = m;
		if (m == MOVE_UNDO) {
			isReject = true;
		}
		sync_cout << "GameLoop : Set enemy move to Thread " << us << sync_endl;
	}
}

RootMove Thread::GetBestMove() {
	std::unique_lock<Mutex> lk(mutex);
	if (!bestMove.depth) {
		sync_cout << "GameLoop : Waiting move from Thread " << us << sync_endl;
		beginTime = clock();
		cv.wait(lk, [&] { return bestMove.depth; });
		beginTime = 0;
		lk.unlock();
	}
	sync_cout << "GameLoop : Thread " << us << " Finished" << sync_endl;
	return bestMove;
}

void Thread::Dump(ostream &os) {
	os << log << endl;
	log.clear();
}

void Thread::IdleLoop() {
	while (!isExit) {
		mutex.lock();
		if (bestMove.enemyMove == MOVE_UNDO) {
			rootPos.UndoMove();
			rootPos.UndoMove();
			bestMove.enemyMove = MOVE_ILLEGAL;
		}
		else if (bestMove.enemyMove == MOVE_NULL) {
			isExit = true;
			mutex.unlock();
			break;
		}
		else if (IsDoMove(bestMove.enemyMove)) {
			rootPos.DoMove(bestMove.enemyMove);
		}
		mutex.unlock();

		if (rootPos.IsGameOver()) {
			isExit = true;
			break;
		}
		if (rootPos.GetTurn() == us) {
			mutex.lock();
			Observer::StartSearching();
			bestMove.depth = 0;
			for (int i = 0; i < rootMoves.size(); i++) {
				if (rootMoves[i].enemyMove == bestMove.enemyMove) {
					bestMove = rootMoves[i];
					sync_cout << "Thread " << us << " : Hit rootMoves" << sync_endl;
					break;
				}
			}
			if (!bestMove.depth) {
				RootMove rm;
				rm.enemyMove = bestMove.enemyMove;
				mutex.unlock();
				finishDepth = false;
				sync_cout << "Thread " << us << " : IDAS" << sync_endl;

				IDAS(rm, Observer::depth, false);

				mutex.lock();
				bestMove = rm;
			}
			rootMoves.clear();
			if (bestMove.pv[0])
				rootPos.DoMove(bestMove.pv[0]);
			else
				isExit = true;
			bestMove.enemyMove = MOVE_ILLEGAL;
			sync_cout << "Thread " << us << " : Domove " << bestMove.pv[0] << sync_endl;

			cv.notify_all();
			isStop = false;
			mutex.unlock();
			Observer::EndSearching();
		}
		else {
			PreIDAS();
			sync_cout << "Thread " << us << " : Stop Presearch" << sync_endl;
			if (isReject) {
				rootMoves.clear();
				isReject = false;
			}
			isStop = false;
		}
	}
	sync_cout << "Thread " << us << " : Stoped" << sync_endl;
}