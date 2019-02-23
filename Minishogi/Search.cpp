#include <assert.h>
#include <fstream>
//#include <iostream>

#include "Zobrist.h"
#include "Thread.h"
#include "MovePick.h"
#include "Transposition.h"
#include "Observer.h"
#include "usi.h"
#include "TimeManage.h"

using namespace USI;
using namespace std;

namespace {
	Value NegaScout(bool pvNode, Minishogi& pos, Stack *ss, Key rootKey, Value alpha, Value beta, int depth, bool isResearch);
	Value QuietSearch(bool pvNode, Minishogi& pos, Stack *ss, Key rootKey, Value alpha, Value beta, int depth);

	void UpdatePv(Move* pv, Move move, Move* childPv);
	void UpdateAttackHeuristic(const Minishogi &minishogi, Move move, Move *attackMove, int attackCnt, int bouns);
	void UpdateQuietHeuristic(const Minishogi &minishogi, Stack* ss, Move move, Move *quietMove, int quietCnt, int bouns);
	void UpdateContinousHeuristic(Stack* ss, Piece pc, Square to, int bonus);

	Value value_to_tt(int16_t v, Turn turn, int ply);
	Value value_from_tt(int16_t v, Turn turn, int ply);
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
	double pvFactor = 0;
	int rootDepth;

	//selDepth = 0;
#ifndef ITERATIVE_DEEPENING_DISABLE
	rootDepth = 1;
#else
	rootDepth = depth;
#endif
	value = alpha = delta = -VALUE_INFINITE;
	beta = VALUE_INFINITE;

	if (pos.IsChecking()) {
		rm.value = VALUE_MATE;
		rm.depth = 0;
		if (rm.rootKey == Limits.rootKey)
			sync_cout << USI::pv(rm, *this, alpha, beta) << sync_endl;
		return;
	}
	
	if (rm.depth && rm.value != VALUE_NONE) {
		value = rm.value; 
		rootDepth = rm.depth + 1;
		if (value >= VALUE_MATE_IN_MAX_PLY ||
			value <= VALUE_MATED_IN_MAX_PLY)
			return;
	}

	// Iterative Deepening
	while (true) {
		bool isResearch = false;
		pvFactor *= 0.55;
#ifndef ASPIRE_WINDOW_DISABLE
		if (rootDepth >= 5) {
			delta = Value(150);
			alpha = max(value - delta, -VALUE_INFINITE);
			beta = min(value + delta, VALUE_INFINITE);
		}
#endif
		
		for (int retryCnt = 0; !IsStop(); retryCnt++) {
			//Observer::aspTime[retryCnt]++;
			value = NegaScout(true, pos, ss, rm.rootKey, alpha, beta, rootDepth, isResearch);
			
			if (value <= alpha) 
				if (retryCnt > 3) 
					alpha = -VALUE_INFINITE;
				else 
					alpha = max(value - delta, -VALUE_INFINITE);
			else if (value >= beta)
				if (retryCnt > 3)
					beta = VALUE_INFINITE;
				else
					beta = min(value + delta, VALUE_INFINITE);
			else
				break;
			delta = delta * 2;
			isResearch = true;
			//Observer::aspFail[retryCnt]++;
		}
		if (IsStop()) 
			break;

		// Save Result
		pvFactor += rm.pv[0] != ss->pv[0];
		int i = 0;
		rm.depth = rootDepth;
		rm.value = value;
		do {
			rm.pv[i] = ss->pv[i];
		} while (ss->pv[i++] != MOVE_NULL);

		if (rm.rootKey == Limits.rootKey)
			sync_cout << USI::pv(rm, *this, alpha, beta) << sync_endl;

		// Time Management
		if (Limits.IsTimeManagement()) {
			Time.SetTimer(rm.value, pvFactor);
			if (!Limits.ponder) {
				sync_cout << "info string optimum " << Time.GetOptimum() << " maximum " << Time.GetMaximum() << sync_endl;
				if (Time.Elapsed() >= Time.GetOptimum())
					break;
			}
		}

		if (CheckStop(rm.rootKey) ||
			value >= VALUE_SENNI_IN_MAX_COUNT ||
			value <= -VALUE_SENNI_IN_MAX_COUNT ||
			(!Limits.ponder && ss->moveCount == 1))
			break;
		rootDepth++;
	}
}

void Thread::PreIDAS() {
	int depth = USI::Options["Depth"];
	Move ponderMove = pos.GetPrevMove();

	rootMoves.clear();
	isFinishPonder = false;

	if (Options["TotalMovePonder"] && pos.GetPly() > 0) {
		bool ttHit;
		const TTentry *tte = GlobalTT.Probe(pos.GetKey(), ttHit); // Save TT?
		const Move ttMove = ttHit ? tte->move : MOVE_NULL;
		const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
		//const Move counterMove = counterMoves[rootPos.GetChessOn(to_sq((ss - 1)->currentMove))][to_sq((ss - 1)->currentMove)];
		Move move;
		MovePicker mp(pos, ttMove, depth, &mainHistory, &captureHistory, contHist/*, counterMove, nullptr*/);
		
		while (!IsStop() && ((move = mp.GetNextMove()) != MOVE_NULL)) {
			pos.UndoMove();
			pos.DoMove(move);
			(ss - 1)->currentMove = move;
			(ss - 1)->contHistory = &contHistory[pos.GetPiece(move)][to_sq(move)];
			rootMoves.emplace_back(pos.GetKey(), move);
			IDAS(rootMoves.back(), depth);
		}
	}
	else {
		rootMoves.emplace_back(pos.GetKey(), ponderMove);
		IDAS(rootMoves.back(), depth);
	}

	isFinishPonder = true;
	depth++;

	bool isCompleted = false;
	while (!IsStop() && !isCompleted) {
		isCompleted = true;
		for (auto& rm : rootMoves) {
			if (IsStop() ||
				rm.value >= VALUE_SENNI_IN_MAX_COUNT ||
				rm.value <= -VALUE_SENNI_IN_MAX_COUNT)
				continue;

			isCompleted = false;
			if (Options["TotalMovePonder"] && pos.GetPly() > 0) {
				pos.UndoMove();
				pos.DoMove(rm.rootMove);
				(ss - 1)->currentMove = rm.rootMove;
				(ss - 1)->contHistory = &contHistory[pos.GetPiece(rm.rootMove)][to_sq(rm.rootMove)];
			}
			IDAS(rm, depth);
		}
		depth++;
	}

	if (pos.GetPrevMove() != ponderMove) {
		pos.UndoMove();
		pos.DoMove(ponderMove);
	}
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

		//if (pvNode && thisThread->selDepth < ss->ply)
		//	thisThread->selDepth = ss->ply;

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
		tte = GlobalTT.Probe(pos.GetKey(), ttHit);
		ttValue = ttHit ? value_from_tt(tte->value, pos.GetTurn(), ss->ply) : VALUE_NONE;
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
				if (!pos.GetCapture(ttMove))
					UpdateQuietHeuristic(pos, ss, ttMove, nullptr, 0, stat_bonus(depth));
				if ((ss - 1)->moveCount == 1 && pos.GetPrevCapture() != NO_PIECE)
					UpdateContinousHeuristic(ss - 1, pos.GetBoard(prevSq), prevSq, -stat_bonus(depth + 1));
			}
			else if (!pos.GetCapture(ttMove)) {
				int penalty = -stat_bonus(depth);
				thisThread->mainHistory[pos.GetTurn()][from_sq(ttMove)][to_sq(ttMove)] << penalty;
				UpdateContinousHeuristic(ss, pos.GetPiece(ttMove), to_sq(ttMove), penalty);
			}
			return ttValue;
		}

#ifndef NULLMOVE_DISABLE
		// Null move search with verification search
		// Ex-Null Move
		if (!pvNode &&
			pos.GetEvaluate() >= beta &&
			!(ss - 1)->nmp_flag &&
			!pos.IsInChecked()) {
			int R = depth <= 6 ? 3 : 4;
			ss->currentMove = MOVE_NULL;
			ss->contHistory = &thisThread->contHistory[NO_PIECE][0];

			ss->nmp_flag = true;
			pos.DoNullMove();
			Value nullValue = -NegaScout(false, pos, ss + 1, rootKey, -beta, -beta + 1, depth - R - 1, isResearch);
			pos.UndoNullMove();
			ss->nmp_flag = false;

			if (nullValue >= beta) {
				depth -= 4;
				if (depth < 1)
					return QuietSearch(pvNode, pos, ss, rootKey, alpha, beta, 0);
			}
		}
		// Null Move
		/*if (!pvNode &&
			pos.GetEvaluate() >= beta &&
			!(ss - 1)->nmp_flag &&
			!pos.IsInChecked()) {
			int R = (depth <= 6 || (depth <= 8 && abs(pos.GetEvaluate()) < 1000)) ? 2 : 3;

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
		}*/
		// Stockfish
		/*if (!pvNode &&
			pos.GetEvaluate() >= beta &&
			pos.GetEvaluate() >= beta - 36 * depth + 225 &&
			!(ss - 1)->nmp_flag &&
			!pos.IsInChecked()) {
			int R = ((823 + 67 * (depth)) / 256 + min((pos.GetEvaluate() - beta) / VALUE_PAWN, 3));

			ss->currentMove = MOVE_NULL;
			ss->contHistory = &thisThread->contHistory[NO_PIECE][0];
			ss->nmp_flag = true;
			pos.DoNullMove();
			Value nullValue = -NegaScout(false, pos, ss + 1, rootKey, -beta, -beta + 1, depth - R, isResearch);
			pos.UndoNullMove();
			ss->nmp_flag = false;

			if (nullValue >= beta) {
				Observer::data[Observer::nullMoveNum]++;
				if (nullValue >= VALUE_MATE_IN_MAX_PLY)
					nullValue = beta;

				if (depth < 12 && abs(beta) < 10000)
					return nullValue;

				Value nullValue = -NegaScout(false, pos, ss, rootKey, beta - 1, beta, depth - R, isResearch);

				if (nullValue >= beta)
					return nullValue;
			}
		}*/
#endif

		const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
		Move capturesSearched[32], quietsSearched[64]; //, countermove = thisThread->counterMoves[pos.GetBoard(prevSq)][prevSq];
		MovePicker mp(pos, ttMove, depth, &thisThread->mainHistory, &thisThread->captureHistory, contHist/*, countermove, ss->killers*/);
		bool isInChecked = pos.IsInChecked(); // 若此盤面處於被將 解將不考慮千日手
		int captureCount = 0, quietCount = 0;
		
		while ((move = mp.GetNextMove()) != MOVE_NULL) {
			bool isCapture = pos.GetCapture(move) != NO_PIECE;
			int R = 0;

			ss->currentMove = move;
			ss->contHistory = &thisThread->contHistory[pos.GetPiece(move)][to_sq(move)];
			Observer::data[Observer::mainNode]++;
			Observer::data[Observer::researchNode] += isResearch;

			pos.DoMove(move);
			ss->moveCount++;
			if (pos.IsUchifuzume()) {
				value = mated_in(ss->ply);
			}
			else {
				value = pos.GetSennichiteValue();
				if (value == VALUE_NONE) {
					if (pvNode)
						(ss + 1)->pv[0] = MOVE_NULL;

					// Late Move Reduction
#ifndef LMR_DISABLE
					if (!pvNode &&
						depth >= 4 &&
						bestValue > VALUE_MATED_IN_MAX_PLY &&
						ss->moveCount >= 5 &&
						!(ss - 1)->lmr_flag &&
						!(ss - 1)->nmp_flag &&
						!isCapture &&
						!isInChecked &&
						!pos.IsInChecked()) {
						ss->lmr_flag = true;
						R = depth / 3;
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
					value = -NegaScout(pvNode, pos, ss + 1, rootKey, -beta, -alpha, depth - 1, isResearch);
#endif
				}
			}
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
				if (pos.GetCapture(move) && captureCount < 32)
					capturesSearched[captureCount++] = move;

				else if (!pos.GetCapture(move) && quietCount < 64)
					quietsSearched[quietCount++] = move;
			}
		}

		// Debug : null move zugzwangs
		/*if (value < beta && nullValue != VALUE_NONE && nullValue > value)
			Observer::data[Observer::zugzwangsNum]++;*/

		// Debug : move ordering
		/*if (value >= beta &&
			!pos.IsInChecked()) {
			Observer::lmrTest[ss->moveCount]++;
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
			if (!pos.GetCapture(bestMove))
				UpdateQuietHeuristic(pos, ss, bestMove, quietsSearched, quietCount, stat_bonus(depth));
			else
				UpdateAttackHeuristic(pos, bestMove, capturesSearched, captureCount, stat_bonus(depth));

			// Extra penalty for a quiet TT move in previous ply when it gets refuted
			if ((ss - 1)->moveCount == 1 && !pos.GetPrevCapture())
				UpdateContinousHeuristic(ss - 1, pos.GetBoard(prevSq), prevSq, -stat_bonus(depth + 1));
		}
		// Bonus for prior countermove that caused the fail low
		else if ((depth >= 3 || pvNode) &&
			!pos.GetPrevCapture() &&
			IsDoMove((ss - 1)->currentMove)) {
			UpdateContinousHeuristic(ss - 1, pos.GetBoard(prevSq), prevSq, stat_bonus(depth));
		}
		tte->save(pos.GetKey(), depth, value_to_tt(bestValue, pos.GetTurn(), ss->ply), bestMove,
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

		if (thisThread->CheckStop(rootKey))
			return VALUE_ZERO;

		if (pvNode) {
			ss->pv[0] = MOVE_NULL;
			oldAlpha = alpha; // To flag BOUND_EXACT when eval above alpha and no available moves
			//if (thisThread->selDepth < ss->ply)
			//	thisThread->selDepth = ss->ply;
		}

		ss->moveCount = 0;
		(ss + 1)->ply = ss->ply + 1;
		ss->currentMove = bestMove = MOVE_NULL;
		ss->contHistory = &thisThread->contHistory[NO_PIECE][0];

		tte = GlobalTT.Probe(key, ttHit);
		ttValue = ttHit ? value_from_tt(tte->value, pos.GetTurn(), ss->ply) : VALUE_NONE;
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
				//	tte->save(key, DEPTH_NONE, value_to_tt(bestValue, pos.GetTurn(), ss->ply), MOVE_NULL, TTentry::FAILHIGH);
				return bestValue;
			}

			if (pvNode && bestValue > alpha)
				alpha = bestValue;
		}

		const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
		MovePicker mp(pos, ttMove, depth, &thisThread->mainHistory, &thisThread->captureHistory, contHist, to_sq((ss - 1)->currentMove));

		while ((move = mp.GetNextMove()) != MOVE_NULL) {
			ss->currentMove = move;
			ss->contHistory = &thisThread->contHistory[pos.GetPiece(move)][to_sq(move)];
			Observer::data[Observer::quiesNode]++;

			Value value;
			pos.DoMove(move);
			if (pos.IsUchifuzume()) {
				value = mated_in(ss->ply);
			}
			else {
				value = pos.GetSennichiteValue();
				if (value == VALUE_NONE)
					value = -QuietSearch(pvNode, pos, ss + 1, rootKey, -beta, -alpha, depth - 1);
			}
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
						tte->save(key, depth, value_to_tt(bestValue, pos.GetTurn(), ss->ply), move, TTentry::FAILHIGH);
						return value;
					}
				}
			}
		}
		if (isInChecked && bestValue == -VALUE_INFINITE)
			return mated_in(ss->ply);

		tte->save(key, depth, value_to_tt(bestValue, pos.GetTurn(), ss->ply), bestMove, pvNode && bestValue > oldAlpha ? TTentry::EXACT : TTentry::UNKNOWN);
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
		Piece moved_piece = pos.GetPiece(move);
		Piece captured = type_of(pos.GetCapture(move));
		pos.GetThread()->captureHistory[moved_piece][to_sq(move)][captured] << bonus;

		// Decrease all the other played capture moves
		for (int i = 0; i < captureCnt; ++i) {
			moved_piece = pos.GetPiece(captures[i]);
			captured = type_of(pos.GetCapture(captures[i]));
			pos.GetThread()->captureHistory[moved_piece][to_sq(captures[i])][captured] << -bonus;
		}
	}

	// UpdateQuietHeuristic() updates move sorting heuristics when a new quiet best move is found
	void UpdateQuietHeuristic(const Minishogi& pos, Stack* ss, Move move, Move* quiets, int quietsCnt, int bonus) {
		Turn us = pos.GetTurn();
		pos.GetThread()->mainHistory[us][from_sq(move)][to_sq(move)] << bonus;
		UpdateContinousHeuristic(ss, pos.GetPiece(move), to_sq(move), bonus);

#ifndef REFUTATION_DISABLE
		if (ss->killers[0] != move) {
			ss->killers[1] = ss->killers[0];
			ss->killers[0] = move;
		}
		if ((ss - 1)->currentMove != MOVE_NULL) {
			Square prevSq = to_sq((ss - 1)->currentMove);
			pos.GetThread()->counterMoves[pos.GetBoard(prevSq)][prevSq] = move;
		}
#endif

		// Decrease all the other played quiet moves
		for (int i = 0; i < quietsCnt; ++i) {
			pos.GetThread()->mainHistory[us][from_sq(quiets[i])][to_sq(quiets[i])] << -bonus;
			UpdateContinousHeuristic(ss, pos.GetPiece(quiets[i]), to_sq(quiets[i]), -bonus);
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
	inline Value value_to_tt(int16_t v, Turn turn, int ply) {
		assert(v != VALUE_NONE);
		v = v >= VALUE_MATE_IN_MAX_PLY ? v + ply
		  : v <= VALUE_MATED_IN_MAX_PLY ? v - ply : v;
		return Value(turn ? -v : v);
	}


	// value_from_tt() is the inverse of value_to_tt(): It adjusts a mate score
	// from the transposition table (which refers to the plies to mate/be mated
	// from current position) to "plies to mate/be mated from the root".
	inline Value value_from_tt(int16_t v, Turn turn, int ply) {
		v = turn ? -v : v;
		return Value(v == VALUE_NONE ? VALUE_NONE
			: v >= VALUE_MATE_IN_MAX_PLY ? v - ply
			: v <= VALUE_MATED_IN_MAX_PLY ? v + ply : v);
	}

	inline int stat_bonus(int d) {
		return d > 17 ? 0 : 32 * d * d + 64 * d - 64;
	}
}