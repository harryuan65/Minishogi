#include <assert.h>
#include <fstream>
#include <windows.h>

#include "Search.h"
#include "Thread.h"
#include "MovePick.h"
#include "Transposition.h"

using namespace Search;
using namespace Transposition;

Value value_to_tt(Value v, int ply);
Value value_from_tt(Value v, int ply);

inline int stat_bonus(int d) {
	return d > 17 ? 0 : 32 * d * d + 64 * d - 64;
}

void Thread::IDAS(RootMove &rm, int depth) {
	Value value, alpha, beta, delta;
	fstream file;
	bool isresearch;
	int rootDepth;

#ifndef ITERATIVE_DEEPENING_DISABLE
	rootDepth = 1;
#else
	rootDepth = depth;
#endif
	value = alpha = delta = -VALUE_INFINITE;
	beta = VALUE_INFINITE;
	if (rm.depth) {
		if (rm.depth >= depth) {
			return;
		}
		else {
			value = rm.value; 
			rootDepth = rm.depth + 1;
		}
	}

	for (; rootDepth <= depth && !IsStop(); rootDepth++) {
#ifndef ASPIRE_WINDOW_DISABLE
		if (rootDepth >= 5) {
			delta = Value(100);
			alpha = max(value - delta, -VALUE_INFINITE);
			beta = min(value + delta, VALUE_INFINITE);
		}
#endif
		isresearch = false;
		while (true) {
			ss->pv[0] = MOVE_NULL;
			value = NegaScout(true, rootPos, ss, rm.enemyMove, alpha, beta, rootDepth, isresearch);
			
			if (IsStop())
				break;
			if (value <= alpha)	{
				alpha = max(value - delta, -VALUE_INFINITE);
				beta = min(value + delta, VALUE_INFINITE);
			}
			else if (value >= beta) {
				alpha = max(value - delta, -VALUE_INFINITE);
				beta = min(value + delta, VALUE_INFINITE);
			}
			else {
				break;
			}
			delta += delta;
			isresearch = true;
		}
		if (IsStop())
			break;

		int i = 0;
		rm.depth = rootDepth;
		rm.value = value;
		do {
			rm.pv[i] = ss->pv[i];
		} while (ss->pv[i++] != MOVE_NULL);

		// value => 必勝或必輸, beginTime => 有時間壓力, ss->moveCount => 只有一個合法步
		if (CheckStop(rm.enemyMove) || 
			value >= VALUE_MATE_IN_MAX_PLY || 
			value <= VALUE_MATED_IN_MAX_PLY ||
			(beginTime && ss->moveCount == 1))
			break;
	}
}

void Thread::PreIDAS() {
	Move move;
	int depth = Observer::depth;
	bool ttHit, isWin = false;

	rootMoves.clear();
	const TTnode *ttn = Probe(rootPos.GetKey(), ttHit); // Save TT?
	const Move ttMove = ttHit ? ttn->move : MOVE_NULL;
	const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
	const Move counterMove = counterMoves[rootPos.GetChessOn(to_sq((ss - 1)->currentMove))][to_sq((ss - 1)->currentMove)];
	MovePicker mp(rootPos, ttMove, depth, &mainHistory, &captureHistory, contHist, counterMove, ss->killers);
	sync_cout << "Thread " << us << " : Preseaching Depth " << depth << sync_endl;
	while (!IsStop() && ((move = mp.GetNextMove(false)) != MOVE_NULL)) {
		rootMoves.emplace_back();
		rootMoves.back().enemyMove = move;
		rootPos.DoMove(move);
		IDAS(rootMoves.back(), depth);
		rootPos.UndoMove();
		if (CheckStop(move))
			break;
	}
	if (!rootMoves.size()) {
		isExit = true;
		return;
	}
	depth++;

	while (!IsStop() && !isWin) {
		isWin = true;
		sync_cout << "Thread " << us << " : Preseaching Depth " << depth << sync_endl;
		for (int i = 0; !IsStop() && i < rootMoves.size(); i++) {
			if (rootMoves[i].value >= VALUE_MATE_IN_MAX_PLY) 
				continue;
			isWin = false;
			rootPos.DoMove(rootMoves[i].enemyMove);
			IDAS(rootMoves[i], depth);
			rootPos.UndoMove();
			if (CheckStop(move))
				break;
		}
		depth++;
	}
	while (isWin && !CheckStop(move))
		Sleep(10);
}

Value Search::NegaScout(bool pvNode, Minishogi &pos, Stack *ss, Move rootMove, Value alpha, Value beta, int depth, bool isResearch) {
	Observer::data[Observer::DataType::totalNode]++;
	Observer::data[Observer::DataType::researchNode] += isResearch;

	Thread *thisThread = pos.GetThread();
	Value value, bestValue, ttValue;
	Move move, ttMove, bestMove;
	TTnode *ttn;
	Square prevSq;
	bool ttHit;
	const bool rootNode = ss->ply == 0;

	if (thisThread->CheckStop(rootMove))
		return VALUE_ZERO;

	bestValue = -VALUE_INFINITE;
	ss->moveCount = 0;

	ss->pv[0] = MOVE_NULL;
	(ss + 1)->ply = ss->ply + 1;
	ss->currentMove = bestMove = MOVE_NULL;
	ss->contHistory = &thisThread->contHistory[EMPTY][0];
	(ss + 2)->killers[0] = (ss + 2)->killers[1] = MOVE_NULL;
	prevSq = to_sq((ss - 1)->currentMove);

	ttn = Probe(pos.GetKey(), ttHit);
	ttValue = ttHit ? value_from_tt((Value)ttn->value, ss->ply) : VALUE_NONE;
	ttMove = ttHit ? ttn->move : MOVE_NULL;
	if (!pvNode && ttHit && ttn->depth >= depth && ttValue != VALUE_NONE && (ttValue >= beta ? (ttn->bound & TTnode::FAILHIGH) : (ttn->bound & TTnode::UNKNOWN))) {
		if (ttMove) {
			if (ttValue >= beta) {
				if (!pos.GetChessOn(to_sq(ttMove)))
					UpdateQuietHeuristic(pos, ss, ttMove, nullptr, 0, stat_bonus(depth));
				if ((ss - 1)->moveCount == 1 && pos.GetCapture() != EMPTY)
					UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, -stat_bonus(depth + 1));
			}
			else if (!pos.GetChessOn(to_sq(ttMove))) {
				int penalty = -stat_bonus(depth);
				thisThread->mainHistory[pos.GetTurn()][from_sq(ttMove)][to_sq(ttMove)] << penalty;
				UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(ttMove)), to_sq(ttMove), penalty);
			}
		}
		return ttValue;
	}

	if (depth == 0) {
#ifndef QUIES_DISABLE
		return QuietSearch(pvNode, pos, ss, rootMove, alpha, beta, 0);
#else
		return pos.GetEvaluate();
#endif
	}

	const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
	Move capturesSearched[32], quietsSearched[64], countermove = thisThread->counterMoves[pos.GetChessOn(prevSq)][prevSq];
	MovePicker mp(pos, ttMove, depth, &thisThread->mainHistory, &thisThread->captureHistory, contHist, countermove, ss->killers);
	bool isChecked = pos.IsChecked(); // 若此盤面處於被將 解將不考慮千日手
	int captureCount = 0, quietCount = 0;

	while ((move = mp.GetNextMove(false)) != MOVE_NULL) {
		ss->currentMove = move;
		ss->contHistory = &thisThread->contHistory[pos.GetChessOn(from_sq(move))][to_sq(move)];

		pos.DoMove(move);
		if ((pos.GetTurn() == BLACK || pos.IsChecked()) && pos.IsSennichite()) {
			pos.UndoMove();
			continue;
		}

		ss->moveCount++;
#ifndef PVS_DISABLE
		if (depth > 3 && ss->moveCount > 1) {
			value = -NegaScout(pvNode, pos, ss + 1, rootMove, -(alpha + 1), -alpha, depth - 1, isResearch);
			if (alpha < value && value < beta) {
				ss->moveCount++;
				value = -NegaScout(pvNode, pos, ss + 1, rootMove, -beta, -(value - 1), depth - 1, true);
			}
		}
		else {
			value = -NegaScout(pvNode, pos, ss + 1, rootMove, -beta, -alpha, depth - 1, isResearch);
		}
#else
		value = -NegaScout(pvNode, pos, ss + 1, -beta, -alpha, depth - 1, isResearch);
#endif
		pos.UndoMove();

		if (value > bestValue) {
			bestValue = value;
			if (value > alpha) {
				bestMove = move;
				if (pvNode) {
					UpdatePv(ss->pv, bestMove, (ss + 1)->pv);
				}
				if (pvNode && value < beta) {
					alpha = value;
				}
			}
		}

		if (move != bestMove) {
			if (pos.GetChessOn(to_sq(move)) && captureCount < 32)
				capturesSearched[captureCount++] = move;

			else if (!pos.GetChessOn(to_sq(move)) && quietCount < 64)
				quietsSearched[quietCount++] = move;
		}

		if (bestValue >= beta) {
			break;
		}
	}
	Observer::data[Observer::DataType::scoutSearchBranch] += ss->moveCount;
	if (ss->moveCount) {
		Observer::data[Observer::DataType::scoutGeneNums]++;
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
		if ((ss - 1)->moveCount == 1 && !pos.GetCapture())
			UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, -stat_bonus(depth + 1));
	}
	// Bonus for prior countermove that caused the fail low
	else if (depth >= 3 && !pos.GetCapture() && IsDoMove((ss - 1)->currentMove)) {
		UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, stat_bonus(depth));
	}
	ttn->save(pos.GetKey(), depth, value_to_tt(bestValue, ss->ply), bestMove,
		bestValue >= beta ? TTnode::FAILHIGH : (pvNode && bestMove) ? TTnode::EXACT : TTnode::UNKNOWN);
	assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);
	return bestValue;
}

Value Search::QuietSearch(bool pvNode, Minishogi& pos, Stack *ss, Move rootMove, Value alpha, Value beta, int depth = 0) {
	Observer::data[Observer::DataType::quiesNode]++;

	Thread *thisThread = pos.GetThread();
	const bool isChecked = pos.IsChecked();
	const Key key = pos.GetKey();
	Value bestValue, ttValue, oldAlpha = alpha;
	Move ttMove, move, bestMove = MOVE_NULL;
	TTnode *ttn;
	bool ttHit;
	int ttDepth = isChecked || depth >= 0 ? 0 : -1;

	if (thisThread->CheckStop(rootMove))
		return VALUE_ZERO;

	bestValue = -VALUE_INFINITE;
	ss->moveCount = 0;
	ss->pv[0] = MOVE_NULL;
	(ss + 1)->ply = ss->ply + 1;

	ttn = Probe(key, ttHit);
	ttValue = ttHit ? value_from_tt((Value)ttn->value, ss->ply) : VALUE_NONE;
	ttMove = ttHit ? ttn->move : MOVE_NULL;
	if (ttHit && ttValue != VALUE_NONE && ttMove && !pos.PseudoLegal(ttMove)) {
		ttValue = VALUE_NONE;
		ttMove = MOVE_NULL;
	}
	
	if (!pvNode
		&& ttHit
		&& ttn->depth >= ttDepth
		&& ttValue != VALUE_NONE
		&& (ttValue >= beta ? (ttn->bound & TTnode::FAILHIGH)
							: (ttn->bound & TTnode::UNKNOWN))) {
		Observer::data[Observer::DataType::ios_read] += pos.IsIsomorphic();
		return ttValue;
	}

	if (!isChecked) {
		bestValue = pos.GetEvaluate();

		if (ttHit && ttValue != VALUE_NONE
			&& (ttn->bound & (ttValue > bestValue ? TTnode::FAILHIGH : TTnode::UNKNOWN))) {
			bestValue = ttValue;
		}

		if (bestValue >= beta) {
			if (!ttHit)
				ttn->save(key, 0, value_to_tt(bestValue, ss->ply), MOVE_NULL, TTnode::FAILHIGH);
			return bestValue;
		}

		if (bestValue > alpha)
			alpha = bestValue;
	}

	MovePicker mp(pos, ttMove, depth, &thisThread->mainHistory, &thisThread->captureHistory, to_sq((ss - 1)->currentMove));

	while ((move = mp.GetNextMove(false)) != MOVE_NULL) {
		ss->currentMove = move;

		pos.DoMove(move);
		Value value = -QuietSearch(pvNode, pos, ss + 1, rootMove, -beta, -alpha, depth - 1);
		pos.UndoMove();

		if (value > bestValue) {
			bestValue = value;
			if (value > alpha) {
				UpdatePv(ss->pv, move, (ss + 1)->pv);
				if (value < beta) { // Update alpha! Always alpha < beta
					alpha = value;
					bestMove = move;
				}
				else {
					ttn->save(key, ttDepth, value_to_tt(bestValue, ss->ply), move, TTnode::FAILHIGH);
					return bestValue;
				}
			}
		}
	}
	if (isChecked && bestValue == -VALUE_INFINITE)
		return mated_in(ss->ply);

	ttn->save(key, ttDepth, value_to_tt(bestValue, ss->ply), bestMove, bestValue > oldAlpha ? TTnode::EXACT : TTnode::UNKNOWN);
	assert(bestValue > -VALUE_INFINITE && bestValue < VALUE_INFINITE);
	return bestValue;
}

void Search::UpdatePv(Move* pv, Move move, Move* childPv) {
	for (*pv++ = move; childPv && *childPv != MOVE_NULL; )
		*pv++ = *childPv++;
	*pv = MOVE_NULL;
}

// UpdateAttackHeuristic() updates move sorting heuristics when a new capture best move is found
void Search::UpdateAttackHeuristic(const Minishogi& pos, Move move, Move* captures, int captureCnt, int bonus) {
	Chess moved_piece = pos.GetChessOn(from_sq(move));
	Chess captured = type_of(pos.GetChessOn(to_sq(move)));
	pos.GetThread()->captureHistory[moved_piece][to_sq(move)][captured] << bonus;

	// Decrease all the other played capture moves
	for (int i = 0; i < captureCnt; ++i) {
		moved_piece = pos.GetChessOn(from_sq(captures[i]));
		captured = type_of(pos.GetChessOn(to_sq(captures[i])));
		pos.GetThread()->captureHistory[moved_piece][to_sq(captures[i])][captured] << -bonus;
	}
}

// UpdateQuietHeuristic() updates move sorting heuristics when a new quiet best move is found
void Search::UpdateQuietHeuristic(const Minishogi& pos, Stack* ss, Move move, Move* quiets, int quietsCnt, int bonus) {
	if (ss->killers[0] != move) {
		ss->killers[1] = ss->killers[0];
		ss->killers[0] = move;
	}

	Color us = pos.GetTurn();
	pos.GetThread()->mainHistory[us][from_sq(move)][to_sq(move)] << bonus;
	UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(move)), to_sq(move), bonus);

	if ((ss - 1)->currentMove != MOVE_NULL) {
		int prevSq = to_sq((ss - 1)->currentMove);
		pos.GetThread()->counterMoves[pos.GetChessOn(prevSq)][prevSq] = move;
	}

	// Decrease all the other played quiet moves
	for (int i = 0; i < quietsCnt; ++i) {
		pos.GetThread()->mainHistory[us][from_sq(quiets[i])][to_sq(quiets[i])] << -bonus;
		UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(quiets[i])), to_sq(quiets[i]), -bonus);
	}
}

void Search::UpdateContinousHeuristic(Stack* ss, Chess pc, Square to, int bonus) {
	for (int i : {1, 2, 4})
		if ((ss - i)->currentMove != MOVE_NULL)
			(*(ss - i)->contHistory)[pc][to] << bonus;
}

string Search::GetSettingStr() {
	stringstream ss;
	ss << "Main Depth          : " << Observer::depth << "\n";
	ss << "Time Limit          : " << (Observer::limitTime ? to_string(Observer::limitTime) + " ms" : "Disable") << "\n";
	ss << "Transposition Table : " << (Transposition::IsEnable() ? "Enable" : "Disable") << "\n";
	if (Transposition::IsEnable()) {
#ifndef DOUBLETP
		ss << "Transposition Type  : Single TT\n";
#else
		ss << "Transposition Type  : Double TT\n";
#endif
		ss << "Transposition Size  : " << ((Transposition::TPSize * sizeof(TTnode)) >> 20) << " MiB\n";

#ifndef ITERATIVE_DEEPENING_DISABLE
		ss << "Iterative Deepening : Enable\n";
#else
		ss << "Iterative Deepening : Disable\n";
#endif
#ifndef ASPIRE_WINDOW_DISABLE
		ss << "Aspire Window       : Enable\n";
#else
		ss << "Aspire Window       : Disable\n";
#endif
#ifndef PVS_DISABLE
		ss << "PVS                 : Enable\n";
#else
		ss << "PVS                 : Disable\n";
#endif
#ifndef QUIES_DISABLE
		ss << "Quiet Search        : Enable\n";
#else
		ss << "Quiet Search        : Disable\n";
#endif
#ifndef MOVEPICK_DISABLE
		ss << "MovePicker          : Enable\n";
#else
		ss << "MovePicker          : Disable\n";
#endif
		return ss.str();
	}
}


// value_to_tt() adjusts a mate score from "plies to mate from the root" to
// "plies to mate from the current position". Non-mate scores are unchanged.
// The function is called before storing a value in the transposition table.
Value value_to_tt(Value v, int ply) {
	assert(v != VALUE_NONE);
	return  v >= VALUE_MATE_IN_MAX_PLY ? v + ply
		: v <= VALUE_MATED_IN_MAX_PLY ? v - ply : v;
}


// value_from_tt() is the inverse of value_to_tt(): It adjusts a mate score
// from the transposition table (which refers to the plies to mate/be mated
// from current position) to "plies to mate/be mated from the root".
Value value_from_tt(Value v, int ply) {
	return  v == VALUE_NONE ? VALUE_NONE
		: v >= VALUE_MATE_IN_MAX_PLY ? v - ply
		: v <= VALUE_MATED_IN_MAX_PLY ? v + ply : v;
}