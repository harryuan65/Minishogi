#include <assert.h>

#include "Minishogi.h"
#include "Transposition.h"
#include "Search.h"

using namespace Search;
using namespace Transposition;

namespace Search {
	ButterflyHistory mainHistory;
	CapturePieceToHistory captureHistory;
	PieceToHistory contHistory[CHESS_NB][SQUARE_NB];
	CounterMoveHistory counterMoves;
}

Value value_to_tt(Value v, int ply);
Value value_from_tt(Value v, int ply);

inline int stat_bonus(int d) {
	return d > 17 ? 0 : 32 * d * d + 64 * d - 64;
}

void Search::Initialize() {
	counterMoves.fill(MOVE_NULL);
	mainHistory.fill(0);
	captureHistory.fill(0);
	for (int i = 0; i < CHESS_NB; i++)
		for (int j = 0; j < SQUARE_NB; j++)
			contHistory[i][j].fill(0);
	contHistory[EMPTY][0].fill(Search::CounterMovePruneThreshold - 1);
}

Value Search::IDAS(Minishogi& pos, Move &bestMove, Move *pv) {
	Stack stack[MAX_PLY + 7], *ss = stack + 4; // To reference from (ss-4) to (ss+2)
	Value bestValue, alpha, beta, delta;
#ifdef ITERATIVE_DEEPENING_ENABLE
	int rootDepth = 1;
#else
	int rootDepth = Observer::DEPTH;
#endif

	std::memset(ss - 4, 0, 7 * sizeof(Stack));
	for (int i = 4; i > 0; i--)
		(ss - i)->contHistory = &contHistory[EMPTY][0]; // Use as sentinel
	bestValue = alpha = -VALUE_INFINITE;
	beta = VALUE_INFINITE;
	bestMove = MOVE_NULL;

	for (; rootDepth <= Observer::DEPTH; rootDepth++) {
		cout << "Searching " << rootDepth << " Depth...\n";
#ifdef ASPIRE_WINDOW_ENABLE
		if (rootDepth >= 5) {
			delta = Value(18);
			alpha = max(bestValue - delta, -VALUE_INFINITE);
			beta = min(bestValue + delta, VALUE_INFINITE);
		}
#endif
		while (true) {
			ss->pv[0] = MOVE_NULL;
			bestValue = NegaScout(pos, ss, alpha, beta, rootDepth, false);
			bestMove = ss->pv[0];

			if (bestValue <= alpha)	{
				beta = (alpha + beta) / 2;
				alpha = max(bestValue - delta, -VALUE_INFINITE);
			}
			else if (bestValue >= beta) {
				beta = min(bestValue + delta, VALUE_INFINITE);
			}
			else {
				break;
			}
			delta += delta / 4 + 5;
		}
		PrintPV(cout, ss->pv);
		cout << bestValue << "\n";
		if (bestValue >= VALUE_MATE_IN_MAX_PLY || bestValue <= VALUE_MATED_IN_MAX_PLY) {
			break;
		}
	}
	return bestValue;
}

Value Search::NegaScout(Minishogi &pos, Stack *ss, Value alpha, Value beta, int depth, bool isResearch) {
	Observer::data[Observer::DataType::totalNode]++;
	Observer::data[Observer::DataType::researchNode] += isResearch;
	Observer::data[Observer::DataType::scoutGeneNums]++;

	Value value, bestValue, ttValue;
	Move move, ttMove, bestMove;
	Key key;
	TTnode *ttn;
	bool ttHit;
	const bool rootNode = ss->ply == 0;

	bestValue = -VALUE_INFINITE;
	key = pos.GetKey();
	ss->moveCount = 0;

	ss->pv[0] = MOVE_NULL;
	(ss + 1)->ply = ss->ply + 1;
	ss->currentMove = bestMove = MOVE_NULL;
	ss->contHistory = &contHistory[EMPTY][0];
	(ss + 2)->killers[0] = (ss + 2)->killers[1] = MOVE_NULL;
	Square prevSq = to_sq((ss - 1)->currentMove);

	ttn = Probe(key, ttHit);
	ttValue = ttHit ? value_from_tt((Value)ttn->value, ss->ply) : VALUE_NONE;
	ttMove = ttHit ? ttn->move : MOVE_NULL;
	if (ttHit && ttValue != VALUE_NONE && ttn->depth >= depth) {
		if (ttMove) {
			if (ttValue >= beta) {
				if (pos.GetChessOn(to_sq(ttMove)) == EMPTY)
					UpdateQuietHeuristic(pos, ss, ttMove, nullptr, 0, stat_bonus(depth));
				if ((ss - 1)->moveCount == 1 && pos.GetChessOn(to_sq(ttMove) != EMPTY))
					UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, -stat_bonus(depth + 1));
			}
			else if (!pos.GetChessOn(to_sq(ttMove))) {
				int penalty = -stat_bonus(depth);
				mainHistory[pos.GetTurn()][from_sq(ttMove)][to_sq(ttMove)] << penalty;
				UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(ttMove)), to_sq(ttMove), penalty);
			}
		}
		return ttValue;
	}

	if (depth == 0) {
#ifdef QUIES_DISABLE
		return pos.GetEvaluate();
#else
		return QuietSearch(pos, ss, alpha, beta, 0);
#endif
	}

	const PieceToHistory* contHist[] = { (ss - 1)->contHistory, (ss - 2)->contHistory, nullptr, (ss - 4)->contHistory };
	Move capturesSearched[32], quietsSearched[64], countermove = counterMoves[pos.GetChessOn(prevSq)][prevSq];
	MovePicker mp(pos, ttMove, depth, &mainHistory, &captureHistory, contHist, countermove, ss->killers);
	bool isChecked = pos.IsChecked(); // �Y���L���B��Q�N �ѱN���Ҽ{�d���
	int captureCount = 0, quietCount = 0;

	while ((move = mp.GetNextMove(false)) != MOVE_NULL) {
		ss->moveCount++;

		ss->currentMove = move;
		ss->contHistory = &contHistory[pos.GetBoard(from_sq(move))][to_sq(move)];
		assert(type_of(pos.GetChessOn(to_sq(move))) != KING);
		pos.DoMove(move);
		if (depth > 3 && ss->moveCount > 1) {
			value = -NegaScout(pos, ss + 1, -(alpha + 1), -alpha, depth - 1, isResearch);
		}
		if (ss->moveCount == 1 || depth <= 3 || (value > alpha && value < beta)) {
			value = -NegaScout(pos, ss + 1, -beta, -alpha, depth - 1, ss->moveCount != 1 && depth > 3);
		}
		pos.UndoMove();

		if (value > bestValue) {
			bestValue = value;
			if (value > alpha) {
				bestMove = move;
				UpdatePv(ss->pv, bestMove, (ss + 1)->pv);
				if (value < beta) { // Update alpha! Always alpha < beta
					alpha = value;
				}
			}
			else if (rootNode && ss->moveCount == 0) {
				UpdatePv(ss->pv, bestMove, (ss + 1)->pv);
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
		if ((ss - 1)->moveCount == 1 && !pos.GetChessOn(to_sq(bestMove)))
			UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, -stat_bonus(depth + 1));
	}
	// Bonus for prior countermove that caused the fail low
	else if (depth >= 3 && !pos.GetChessOn(to_sq(bestMove)) && IsDoMove((ss - 1)->currentMove)) {
		UpdateContinousHeuristic(ss - 1, pos.GetChessOn(prevSq), prevSq, stat_bonus(depth));
	}
	ttn->save(pos.GetKey(), depth, value_to_tt(bestValue, ss->ply), bestMove,
		bestValue >= beta ? TTnode::FailHigh : bestValue < alpha ? TTnode::Unknown : TTnode::Exact);
	return bestValue;
}

Value Search::QuietSearch(Minishogi& pos, Stack *ss, Value alpha, Value beta, int depth = 0) {
	Observer::data[Observer::DataType::quiesNode]++;

	const bool isChecked = pos.IsChecked();
	const Key key = pos.GetKey();
	Value bestValue, ttValue, oldAlpha = alpha;
	Move ttMove, move, bestMove;
	TTnode *ttn;
	bool ttHit;
	int ttDepth = isChecked || depth >= 0 ? 0 : -1;

	bestValue = -VALUE_INFINITE;
    bestMove = MOVE_NULL;
	ss->moveCount = 0;

	ss->pv[0] = MOVE_NULL;
	(ss + 1)->ply = ss->ply + 1;
	ttn = Probe(key, ttHit);
	ttValue = ttHit ? value_from_tt((Value)ttn->value, ss->ply) : VALUE_NONE;
	ttMove = ttHit ? ttn->move : MOVE_NULL;
	if (ttHit
		&& ttn->depth >= ttDepth
		&& ttValue != VALUE_NONE
		&& (ttValue >= beta ? (ttn->bound & TTnode::FailHigh)
							: (ttn->bound & TTnode::Unknown))) {
		Observer::data[Observer::DataType::ios_read] += pos.IsIsomorphic();
		return ttValue;
	}

	if (!isChecked) {
		if (ttHit) {
			if (ttValue != VALUE_NONE
				&& (ttn->bound & (ttValue > bestValue ? TTnode::FailHigh : TTnode::Unknown)))
			bestValue = ttValue;
		}
		else {
			bestValue = pos.GetEvaluate();
		}

		if (bestValue >= beta) {
			if (!ttHit)
				ttn->save(key, 0, value_to_tt(bestValue, ss->ply), MOVE_NULL, TTnode::FailHigh);
			return bestValue;
		}

		if (bestValue > alpha)
			alpha = bestValue;
	}
	MovePicker mp(pos, ttMove, depth, &mainHistory, &captureHistory, to_sq((ss - 1)->currentMove));

	while ((move = mp.GetNextMove(false)) != MOVE_NULL) {
		const Square dstIndex = to_sq(move);

		if (!isChecked) {
			if (!pos.GetChessOn(to_sq(move)) && !is_pro(move)) {
				if (depth < 0)
					continue;

				const int dstChess = pos.board[dstIndex];
				pos.board[dstIndex] = pos.GetChessOn(from_sq(move));
				const bool isCheckable = pos.Movable(dstIndex) &
					pos.bitboard[KING | pos.GetTurn() ^ 1];
				pos.board[dstIndex] = dstChess;

				if (!isCheckable)
					continue;
			}

			if (!pos.SEE(move)) {
				continue;
			}
		}

		pos.DoMove(move);
		Value value = -QuietSearch(pos, ss + 1, -beta, -alpha, depth - 1);
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
					ttn->save(key, ttDepth, value_to_tt(bestValue, ss->ply), move, TTnode::FailHigh);
					return bestValue;
				}
			}
		}
	}
	if (isChecked && bestValue == -VALUE_MATE)
		return -VALUE_MATE;

	ttn->save(key, ttDepth, value_to_tt(bestValue, ss->ply), bestMove, bestValue > oldAlpha ? TTnode::Exact : TTnode::Unknown);
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
	captureHistory[moved_piece][to_sq(move)][captured] << bonus;

	// Decrease all the other played capture moves
	for (int i = 0; i < captureCnt; ++i) {
		moved_piece = pos.GetChessOn(from_sq(captures[i]));
		captured = type_of(pos.GetChessOn(to_sq(captures[i])));
		captureHistory[moved_piece][to_sq(captures[i])][captured] << -bonus;
	}
}

// UpdateQuietHeuristic() updates move sorting heuristics when a new quiet best move is found
void Search::UpdateQuietHeuristic(const Minishogi& pos, Stack* ss, Move move, Move* quiets, int quietsCnt, int bonus) {
	if (ss->killers[0] != move) {
		ss->killers[1] = ss->killers[0];
		ss->killers[0] = move;
	}

	Color us = pos.GetTurn();
	mainHistory[us][from_sq(move)][to_sq(move)] << bonus;
	UpdateContinousHeuristic(ss, pos.GetBoard(from_sq(move)), to_sq(move), bonus);

	if ((ss - 1)->currentMove != MOVE_NULL) {
		int prevSq = to_sq((ss - 1)->currentMove);
		counterMoves[pos.GetChessOn(prevSq)][prevSq] = move;
	}

	// Decrease all the other played quiet moves
	for (int i = 0; i < quietsCnt; ++i) {
		mainHistory[us][from_sq(quiets[i])][to_sq(quiets[i])] << -bonus;
		UpdateContinousHeuristic(ss, pos.GetChessOn(from_sq(quiets[i])), to_sq(quiets[i]), -bonus);
	}
}

void Search::UpdateContinousHeuristic(Stack* ss, Chess pc, Square to, int bonus) {
	for (int i : {1, 2, 4})
		if ((ss - i)->currentMove != MOVE_NULL)
			(*(ss - i)->contHistory)[pc][to] << bonus;
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

void Search::PrintPV(ostream &os, Move *move) {
	int i;
	//os << "PV: (depth | turn | action | my evaluate)" << "\n";
	for (i = 0; move[i] != MOVE_NULL; i++) {
		os << i << " : " << move[i] << "\n";
	}
	/*if (leafEvaluate <= -CHECKMATE || CHECKMATE <= leafEvaluate) {
		os << count << " : " << (((turn + count) & 1) ? "��" : "��") << "Lose " << setw(7) << leafEvaluate << "\n";
	}*/
}