#include <assert.h>
#include <fstream>
#include <windows.h>
//#include <iostream>

#include "Zobrist.h"
#include "Thread.h"
#include "MovePick.h"
#include "Transposition.h"
#include "Observer.h"
#include "usi.h"
using namespace USI;
using namespace std;

namespace {
	Value NegaScout(bool pvNode, Minishogi& pos, Stack *ss, Key rootKey, Value alpha, Value beta, int depth, bool isResearch);
	Value QuietSearch(bool pvNode, Minishogi& pos, Stack *ss, Key rootKey, Value alpha, Value beta, int depth);

	void UpdatePv(Move* pv, Move move, Move* childPv);
	void UpdateAttackHeuristic(const Minishogi &minishogi, Move move, Move *attackMove, int attackCnt, int bouns);
	void UpdateQuietHeuristic(const Minishogi &minishogi, Stack* ss, Move move, Move *quietMove, int quietCnt, int bouns);
	void UpdateContinousHeuristic(Stack* ss, Piece pc, Square to, int bonus);

	Value value_to_tt(Value v, int ply);
	Value value_from_tt(Value v, int ply);
	int stat_bonus(int d);
}

void Thread::Search(RootMove &rm, int depth) {
	int i = 0; 
	rm.value = NegaScout(true, pos, ss, KEY_NULL, -VALUE_INFINITE, VALUE_INFINITE, depth, false);
	rm.depth = depth;
	do {
		rm.pv[i] = ss->pv[i];
	} while (ss->pv[i++] != MOVE_NULL);
}

void Thread::IDAS(RootMove &rm, int depth) {
	fstream file;
	Value value, alpha, beta, delta;
	int rootDepth;
	bool isWin;

	selDepth = 0;
#ifndef ITERATIVE_DEEPENING_DISABLE
	rootDepth = 1;
#else
	rootDepth = depth;
#endif
	isWin = pos.IsChecking();
	value = alpha = delta = -VALUE_INFINITE;
	beta = VALUE_INFINITE;

	if (rm.depth) {
		if (rm.depth < depth) {
			value = rm.value; 
			rootDepth = rm.depth + 1;
		}
	}

	// Iterative Deepening
	for (; rootDepth <= depth && !IsStop() && !isWin; rootDepth++) {
		bool isResearch = false;
#ifndef ASPIRE_WINDOW_DISABLE
		if (rootDepth >= 5) {
			delta = Value(100);
			alpha = max(value - delta, -VALUE_INFINITE);
			beta = min(value + delta, VALUE_INFINITE);
		}
#endif
		while (true) {
			value = NegaScout(true, pos, ss, rm.rootKey, alpha, beta, rootDepth, isResearch);
			
			if (IsStop())
				break;
			if (value <= alpha || value >= beta)	{
				alpha = max(value - delta, -VALUE_INFINITE);
				beta = min(value + delta, VALUE_INFINITE);
			}
			else
				break;
			delta += delta;
			isResearch = true;
		}
		if (IsStop()) {
			//sync_cout << "readyok" << sync_endl;
			break;
		}

		int i = 0;
		rm.depth = rootDepth;
		rm.value = value;
		do {
			rm.pv[i] = ss->pv[i];
		} while (ss->pv[i++] != MOVE_NULL);

		if (rm.rootKey == Limits.rootKey)
			sync_cout << USI::pv(rm, *this, alpha, beta) << sync_endl;

		// value => 必勝或必輸, ss->moveCount => 只有一個合法步
		if (CheckStop(rm.rootKey) ||
			value >= VALUE_MATE_IN_MAX_PLY || 
			value <= VALUE_MATED_IN_MAX_PLY ||
			(!Limits.ponder && ss->moveCount == 1))
			break;
	}
	if (isWin) { // Debug Usage
		sync_cout << "info depth 0 score " << USI::value(VALUE_MATE) << "\nbestmove win" << sync_endl;
	}

	if (!Limits.ponder || isStop) {
		if (isWin) {
			sync_cout << "bestmove win" << sync_endl;
		}
		else if (value <= -VALUE_MATE + 1) {
			sync_cout << "bestmove resign" << sync_endl;
		}
		else {
			sync_cout << "bestmove " << rm.pv[0];
			if (Options["USI_Ponder"] && rm.pv[1] != MOVE_NULL)
				cout << " ponder " << rm.pv[1];
			cout << sync_endl;
		}
	}
}

void Thread::PreIDAS() {
	rootMoves.clear();

	bool isTerminal = false;
#ifndef BACKGROUND_SEARCH_DISABLE
	Move move;
	int depth = USI::Options["Depth"];
	bool ttHit;
	const TTentry *tte = tt.Probe(pos.GetKey(), pos.GetTurn(), ttHit); // Save TT?
	const Move ttMove = ttHit ? tte->move : MOVE_NULL;
	const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
	const Move counterMove = MOVE_NULL;//counterMoves[rootPos.GetChessOn(to_sq((ss - 1)->currentMove))][to_sq((ss - 1)->currentMove)];

	finishDepth = false;
	MovePicker mp(pos, ttMove, depth, &mainHistory, &captureHistory, contHist, counterMove, nullptr);
	sync_cout << "Thread : Preseaching Depth " << depth << sync_endl;
	while (!IsStop() && ((move = mp.GetNextMove(false)) != MOVE_NULL)) {
		rootMoves.emplace_back();
		pos.DoMove(move);
		rootMoves.back().rootKey = pos.GetKey();
		rootMoves.back().rootMove = move;
		IDAS(rootMoves.back(), depth);
		pos.UndoMove();
	}
	finishDepth = true;
	if (!rootMoves.size()) {
		isExit = true;
		return;
	}
	depth++;

	while (!IsStop() && !isTerminal) {
		isTerminal = true;
		sync_cout << "Thread : Preseaching Depth " << depth << sync_endl;
		for (int i = 0; !IsStop() && i < rootMoves.size(); i++) {
			if (rootMoves[i].value >= VALUE_MATE_IN_MAX_PLY || 
				rootMoves[i].value <= VALUE_MATED_IN_MAX_PLY)
				continue;
			isTerminal = false;
			pos.DoMove(rootMoves[i].rootMove);
			IDAS(rootMoves[i], depth);
			pos.UndoMove();
		}
		depth++;
	}
#else
	isTerminal = true;
#endif
	while (isTerminal && !CheckStop())
		Sleep(10);
}

namespace {
	Value NegaScout(bool pvNode, Minishogi &pos, Stack *ss, Key rootKey, Value alpha, Value beta, int depth, bool isResearch) {
		if (depth < 1)
#ifndef QUIES_DISABLE
			return QuietSearch(pvNode, pos, ss, rootKey, alpha, beta, 0);
#else
			return pos.GetEvaluate();
#endif

		Thread *thisThread = pos.GetThread();
		Value value, bestValue, ttValue;
		Move move, ttMove, bestMove;
		TTentry *tte;
		Square prevSq;
		bool ttHit;

		if (thisThread->CheckStop(rootKey))
			return VALUE_ZERO;

		if (pvNode && thisThread->selDepth < ss->ply)
			thisThread->selDepth = ss->ply;

		bestValue = -VALUE_INFINITE;
		ss->moveCount = 0;
		(ss + 1)->ply = ss->ply + 1;
		ss->nmp_flag = (ss - 1)->nmp_flag;
		ss->lmr_flag = (ss - 1)->lmr_flag;
		ss->currentMove = bestMove = MOVE_NULL;
		ss->contHistory = &thisThread->contHistory[NO_PIECE][0];
		//(ss + 2)->killers[0] = (ss + 2)->killers[1] = MOVE_NULL;
		prevSq = to_sq((ss - 1)->currentMove);

		// Transposition table lookup
		tte = thisThread->tt.Probe(pos.GetKey(), pos.GetTurn(), ttHit);
		ttValue = ttHit ? value_from_tt((Value)tte->value, ss->ply) : VALUE_NONE;
		ttMove = ttHit ? tte->move : MOVE_NULL;

		// At non-PV nodes we check for an early TT cutoff
		if (!pvNode &&
			ttHit &&
			ttMove &&
			tte->depth >= depth &&
			ttValue != VALUE_NONE &&
		   (ttValue >= beta ? (tte->bound & TTentry::FAILHIGH) : (tte->bound & TTentry::UNKNOWN)) &&
			pos.PseudoLegal(ttMove)) {
			if (ttValue >= beta) {
				if (!pos.GetChessOn(to_sq(ttMove)))
					UpdateQuietHeuristic(pos, ss, ttMove, nullptr, 0, stat_bonus(depth));
				if ((ss - 1)->moveCount == 1 && pos.GetPrevCapture() != NO_PIECE)
					UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, -stat_bonus(depth + 1));
			}
			else if (!pos.GetChessOn(to_sq(ttMove))) {
				int penalty = -stat_bonus(depth);
				thisThread->mainHistory[pos.GetTurn()][from_sq(ttMove)][to_sq(ttMove)] << penalty;
				UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(ttMove)), to_sq(ttMove), penalty);
			}
			return ttValue;
		}

#ifndef NULLMOVE_DISABLE
		// Null move search with verification search
		/*Ex-Null Move
		if (!pvNode &&
			!thisThread->nmp_ply &&
			!pos.IsInChecked()) {
			int R = depth <= 6 ? 4 : 5;
			ss->currentMove = MOVE_NULL;
			ss->contHistory = &thisThread->contHistory[NO_PIECE][0];

			thisThread->nmp_ply = ss->ply;
			pos.DoNullMove();
			Value nullValue = -NegaScout(false, pos, ss + 1, rootMove, -beta, -beta + 1, depth - R, isResearch);
			pos.UndoNullMove();
			thisThread->nmp_ply = 0;

			if (nullValue >= beta) {
				depth -= 4;
				if (depth < 1)
					return QuietSearch(pvNode, pos, ss, rootMove, alpha, beta, 0);
			}
		}*/
		//Value nullValue = VALUE_NONE;
		if (!pvNode &&
			depth > 3 &&
			!(ss - 1)->nmp_flag &&
			!pos.IsInChecked()) {
			int R = (depth <= 6 || (depth <= 8 && abs(pos.GetEvaluate()) < 6000)) ? 1 : 2;

			ss->currentMove = MOVE_NULL;
			ss->contHistory = &thisThread->contHistory[NO_PIECE][0];
			ss->nmp_flag = true;
			pos.DoNullMove();
			Value nullValue = -NegaScout(false, pos, ss + 1, rootKey, -beta, -beta + 1, depth - R - 1, isResearch);
			pos.UndoNullMove();
			ss->nmp_flag = false;

			if (nullValue >= beta) {
				Observer::data[Observer::nullMoveNum]++;
				if (nullValue >= VALUE_MATE_IN_MAX_PLY)
					nullValue = beta;
				return nullValue;
			}
			if (nullValue > alpha) {
				Observer::data[Observer::nullMoveNum]++;
				if (nullValue < VALUE_MATE_IN_MAX_PLY)
					alpha = nullValue;
			}
			// Debug : null move zugzwangs
			/*else {
				nullValue = VALUE_NONE;
			}*/
		}
#endif

		const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
		Move capturesSearched[32], quietsSearched[64], countermove = MOVE_NULL;//thisThread->counterMoves[pos.GetChessOn(prevSq)][prevSq];
		MovePicker mp(pos, ttMove, depth, &thisThread->mainHistory, &thisThread->captureHistory, contHist, countermove, nullptr);
		bool isInChecked = pos.IsInChecked(); // 若此盤面處於被將 解將不考慮千日手
		int captureCount = 0, quietCount = 0;

		while ((move = mp.GetNextMove(false)) != MOVE_NULL) {
			bool isCapture = pos.GetChessOn(to_sq(move)) != NO_PIECE;
			int R = 0;

			ss->currentMove = move;
			ss->contHistory = &thisThread->contHistory[pos.GetChessOn(from_sq(move))][to_sq(move)];
			Observer::data[Observer::mainNode]++;
			Observer::data[Observer::researchNode] += isResearch;

			pos.DoMove(move);
			// 千日手判斷
			if ((pos.GetTurn() == BLACK || pos.IsInChecked()) && pos.IsSennichite()) {
				pos.UndoMove();
				continue;
			}
			ss->moveCount++;
			if (pvNode)
				(ss + 1)->pv[0] = MOVE_NULL;

			// Late Move Reduction
#ifndef LMR_DISABLE
		/*if (!pvNode &&
			 depth >= 3 &&
			 bestValue > VALUE_MATED_IN_MAX_PLY &&
			 ss->moveCount >= 25 &&
			!(ss - 1)->lmr_flag &&
			!isCapture &&
			!pos.IsInChecked() &&
			!pos.IsCheckAfter(move)) {
			ss->lmr_flag = true;
			R = 1;
		}*/
			if (!pvNode &&
				depth >= 3 &&
				bestValue > VALUE_MATED_IN_MAX_PLY &&
				ss->moveCount >= 5 &&
			  !(ss - 1)->lmr_flag &&
			   !isCapture &&
				from_sq(move) < BOARD_NB &&
			   !isInChecked &&
			   !pos.IsInChecked()) {
				ss->lmr_flag = true;
				R = 1;
			}
#endif

#ifndef PVS_DISABLE
			// Principal Variation Search
			if (depth > 3 && ss->moveCount > 1) {
				value = -NegaScout(false, pos, ss + 1, rootKey, -(alpha + 1), -alpha, depth - R - 1, isResearch);
				if (alpha < value && value < beta) {
					value = -NegaScout(pvNode, pos, ss + 1, rootKey, -beta, -value + 1, depth - 1, true);
					// Debug : research Value < null window Value
					/*Value value2 = -NegaScout(pvNode, pos, ss + 1, rootMove, -beta, -value + 1, depth - 1, true);
					if (value2 < value) {
						sync_cout << "v1 " << setw(6) << value
								  << " v2 " << setw(6) << value2
								  << " a " << setw(6) << alpha
								  << " b " << setw(6) << beta
								  << " " << pos.GetKey() << sync_endl;
						pos.PrintChessBoard();
					}
					value = value2;*/
				}
			}
			else {
				value = -NegaScout(pvNode, pos, ss + 1, rootKey, -beta, -alpha, depth - 1, isResearch);
			}
#else
			value = -NegaScout(pvNode, pos, ss + 1, rootMove, -beta, -alpha, depth - 1, isResearch);
#endif
			pos.UndoMove();

			if (value > bestValue) {
				bestValue = value;
				if (value > alpha) {
					bestMove = move;
					if (pvNode) { // Update pv even in fail-high case
						UpdatePv(ss->pv, move, (ss + 1)->pv);
						// Debug : pv lost
						/*int i = 0;
						while (value < 20000 && value > -20000 && value < beta) {
							if (ss->pv[i] == MOVE_NULL) {
								if (i != depth) {
									cout << value << " " << i << " " << depth << " " << alpha << " " << beta << endl;
								}
								break;
							}
							i++;
						}*/
					}
					if (pvNode && value < beta) { // Update alpha! Always alpha < beta
						alpha = value;
					}
					else {
						assert(value >= beta); // Fail high
						break;
					}
				}
			}

			if (move != bestMove) {
				if (pos.GetChessOn(to_sq(move)) && captureCount < 32)
					capturesSearched[captureCount++] = move;

				else if (!pos.GetChessOn(to_sq(move)) && quietCount < 64)
					quietsSearched[quietCount++] = move;
			}
		}

		// Debug : null move zugzwangs
		/*if (value < beta && nullValue != VALUE_NONE && nullValue > value)
			Observer::data[Observer::zugzwangsNum]++;*/

			// Debug : LMR test
			/*if (!pvNode &&
				depth >= 3 &&
				bestValue > VALUE_MATED_IN_MAX_PLY &&
				ss->moveCount > 5 &&
				!(ss - 1)->nmp_flag &&
				!isChecked) {
				Observer::data[Observer::lmrTestNum]++;
			}*/

		Observer::data[Observer::scoutSearchBranch] += ss->moveCount;
		if (ss->moveCount) {
			Observer::data[Observer::scoutGeneNums]++;
		}
		if (!ss->moveCount) {
			bestValue = mated_in(ss->ply);
		}
		else if (bestMove) {
			// Quiet best move: update move sorting heuristics
			if (!pos.GetChessOn(to_sq(bestMove)))
				UpdateQuietHeuristic(pos, ss, bestMove, quietsSearched, quietCount, stat_bonus(depth));
			else
				UpdateAttackHeuristic(pos, bestMove, capturesSearched, captureCount, stat_bonus(depth));

			// Extra penalty for a quiet TT move in previous ply when it gets refuted
			if ((ss - 1)->moveCount == 1 && !pos.GetPrevCapture())
				UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, -stat_bonus(depth + 1));
		}
		// Bonus for prior countermove that caused the fail low
		else if ((depth >= 3 || pvNode) &&
			!pos.GetPrevCapture() &&
			IsDoMove((ss - 1)->currentMove)) {
			UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, stat_bonus(depth));
		}
		tte->save(pos.GetKey(), depth, value_to_tt(bestValue, ss->ply), bestMove,
			bestValue >= beta ? TTentry::FAILHIGH : (pvNode && bestMove) ? TTentry::EXACT : TTentry::UNKNOWN);

		assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);
		return bestValue;
	}

	Value QuietSearch(bool pvNode, Minishogi& pos, Stack *ss, Key rootKey, Value alpha, Value beta, int depth) {
		const bool isInChecked = pos.IsInChecked();
		const Key key = pos.GetKey();
		Thread *thisThread = pos.GetThread();
		Value bestValue, ttValue, oldAlpha;
		Move ttMove, move, bestMove;
		TTentry *tte;
		bool ttHit, ttMoveLegal;
		//int ttDepth = isInChecked || depth >= 0 ? 0 : -1;

		if (thisThread->CheckStop(rootKey))
			return VALUE_ZERO;

		if (pvNode) {
			ss->pv[0] = MOVE_NULL;
			oldAlpha = alpha; // To flag BOUND_EXACT when eval above alpha and no available moves
			if (thisThread->selDepth < ss->ply)
				thisThread->selDepth = ss->ply;
		}

		ss->moveCount = 0;
		(ss + 1)->ply = ss->ply + 1;
		ss->currentMove = bestMove = MOVE_NULL;
		ss->contHistory = &thisThread->contHistory[NO_PIECE][0];

		tte = thisThread->tt.Probe(key, pos.GetTurn(), ttHit);
		ttValue = ttHit ? value_from_tt((Value)tte->value, ss->ply) : VALUE_NONE;
		ttMove = ttHit ? tte->move : MOVE_NULL;
		ttMoveLegal = ttHit ? pos.PseudoLegal(ttMove) : true;

		if (!pvNode &&
			ttHit &&
		  (!ttMove || ttMoveLegal) &&
			tte->depth >= depth &&
			ttValue != VALUE_NONE &&
		   (ttValue >= beta ? (tte->bound & TTentry::FAILHIGH) : (tte->bound & TTentry::UNKNOWN))) {
			return ttValue;
		}

		if (isInChecked) {
			bestValue = -VALUE_INFINITE;
		}
		else {
			bestValue = pos.GetEvaluate();

			// Can ttValue be used as a better position evaluation?
			if (ttHit &&
			  (!ttMove || ttMoveLegal) &&
				ttValue != VALUE_NONE && 
			   (ttValue > bestValue ? (tte->bound & TTentry::FAILHIGH) : (tte->bound & TTentry::UNKNOWN))) {
				bestValue = ttValue;
			}

			// Stand pat. Return immediately if static value is at least beta
			if (bestValue >= beta) {
				//if (!ttHit)
				//	tte->save(key, DEPTH_NULL, value_to_tt(bestValue, ss->ply), MOVE_NULL, TTentry::FAILHIGH);
				return bestValue;
			}

			if (pvNode && bestValue > alpha)
				alpha = bestValue;
		}

		const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
		MovePicker mp(pos, ttMove, depth, &thisThread->mainHistory, &thisThread->captureHistory, contHist, to_sq((ss - 1)->currentMove));

		if (depth < -20) {
			cout << pos.Sfen() << endl;
		}
		while ((move = mp.GetNextMove(false)) != MOVE_NULL) {
			ss->currentMove = move;
			ss->contHistory = &thisThread->contHistory[pos.GetChessOn(from_sq(move))][to_sq(move)];
			Observer::data[Observer::quiesNode]++;
			/*if (depth < -20) {
				cout << pos << endl;
				cout << move << " "<<pos.IsCheckAfter(move)<< endl;
			}*/

			pos.DoMove(move);
			Value value = -QuietSearch(pvNode, pos, ss + 1, rootKey, -beta, -alpha, depth - 1);
			pos.UndoMove();

			if (value > bestValue) {
				bestValue = value;
				if (value > alpha) {
					if (pvNode)
						UpdatePv(ss->pv, move, (ss + 1)->pv);
					if (pvNode && value < beta) {
						alpha = value;
						bestMove = move;
					}
					else {
						tte->save(key, depth, value_to_tt(bestValue, ss->ply), move, TTentry::FAILHIGH);
						return value;
					}
				}
			}
		}
		if (isInChecked && bestValue == -VALUE_INFINITE)
			return mated_in(ss->ply);

		tte->save(key, depth, value_to_tt(bestValue, ss->ply), bestMove, pvNode && bestValue > oldAlpha ? TTentry::EXACT : TTentry::UNKNOWN);
		assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);
		return bestValue;
	}

	void UpdatePv(Move* pv, Move move, Move* childPv) {
		for (*pv++ = move; childPv && *childPv != MOVE_NULL; )
			*pv++ = *childPv++;
		*pv = MOVE_NULL;
	}

	// UpdateAttackHeuristic() updates move sorting heuristics when a new capture best move is found
	void UpdateAttackHeuristic(const Minishogi& pos, Move move, Move* captures, int captureCnt, int bonus) {
		Piece moved_piece = pos.GetChessOn(from_sq(move));
		Piece captured = type_of(pos.GetChessOn(to_sq(move)));
		pos.GetThread()->captureHistory[moved_piece][to_sq(move)][captured] << bonus;

		// Decrease all the other played capture moves
		for (int i = 0; i < captureCnt; ++i) {
			moved_piece = pos.GetChessOn(from_sq(captures[i]));
			captured = type_of(pos.GetChessOn(to_sq(captures[i])));
			pos.GetThread()->captureHistory[moved_piece][to_sq(captures[i])][captured] << -bonus;
		}
	}

	// UpdateQuietHeuristic() updates move sorting heuristics when a new quiet best move is found
	void UpdateQuietHeuristic(const Minishogi& pos, Stack* ss, Move move, Move* quiets, int quietsCnt, int bonus) {
		Color us = pos.GetTurn();
		pos.GetThread()->mainHistory[us][from_sq(move)][to_sq(move)] << bonus;
		UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(move)), to_sq(move), bonus);

#ifndef REFUTATION_DISABLE
		if (ss->killers[0] != move) {
			ss->killers[1] = ss->killers[0];
			ss->killers[0] = move;
		}
		if ((ss - 1)->currentMove != MOVE_NULL) {
			int prevSq = to_sq((ss - 1)->currentMove);
			pos.GetThread()->counterMoves[pos.GetChessOn(prevSq)][prevSq] = move;
		}
#endif

		// Decrease all the other played quiet moves
		for (int i = 0; i < quietsCnt; ++i) {
			pos.GetThread()->mainHistory[us][from_sq(quiets[i])][to_sq(quiets[i])] << -bonus;
			UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(quiets[i])), to_sq(quiets[i]), -bonus);
		}
	}

	void UpdateContinousHeuristic(Stack* ss, Piece pc, Square to, int bonus) {
		for (int i : {1, 2, 4})
			if ((ss - i)->currentMove != MOVE_NULL)
				(*(ss - i)->contHistory)[pc][to] << bonus;
	}


	// value_to_tt() adjusts a mate score from "plies to mate from the root" to
	// "plies to mate from the current position". Non-mate scores are unchanged.
	// The function is called before storing a value in the transposition table.
	inline Value value_to_tt(Value v, int ply) {
		assert(v != VALUE_NONE);
		return  v >= VALUE_MATE_IN_MAX_PLY ? v + ply
			: v <= VALUE_MATED_IN_MAX_PLY ? v - ply : v;
	}


	// value_from_tt() is the inverse of value_to_tt(): It adjusts a mate score
	// from the transposition table (which refers to the plies to mate/be mated
	// from current position) to "plies to mate/be mated from the root".
	inline Value value_from_tt(Value v, int ply) {
		return  v == VALUE_NONE ? VALUE_NONE
			: v >= VALUE_MATE_IN_MAX_PLY ? v - ply
			: v <= VALUE_MATED_IN_MAX_PLY ? v + ply : v;
	}

	inline int stat_bonus(int d) {
		return d > 17 ? 0 : 32 * d * d + 64 * d - 64;
	}
}