#include <assert.h>
#include <iostream>

#include "Observer.h"
#include "Minishogi.h"
#include "Movepick.h"

using namespace std;

namespace {

	enum Stages {
		MAIN_TT, CAPTURE_INIT, GOOD_CAPTURE, REFUTATION, QUIET_INIT, QUIET, BAD_CAPTURE,
		EVASION_TT             , EVASION_INIT    , EVASION,
		PROBCUT_TT             , PROBCUT_INIT    , PROBCUT,
		QSEARCH_WITH_CHECKS_TT , QCAPTURES_1_INIT, QCAPTURES_1, QCHECKS,
		QSEARCH_NO_CHECKS_TT   , QCAPTURES_2_INIT, QCAPTURES_2,
		QSEARCH_RECAPTURES_INIT, QRECAPTURES,
		NONSORT_INIT           , NONSORT
	};

	// Helper filter used with select()
	const auto Any = []() { return true; };

	// partial_insertion_sort() sorts moves in descending order up to and including
	// a given limit. The order of moves smaller than the limit is left unspecified.
	void partial_insertion_sort(ExtMove* begin, ExtMove* end, int limit) {
		for (ExtMove *sortedEnd = begin, *p = begin + 1; p < end; ++p) {
			if (p->score >= limit) {
				ExtMove tmp = *p, *q;
				*p = *++sortedEnd;
				for (q = sortedEnd; q != begin && *(q - 1) < tmp; --q)
					*q = *(q - 1);
				*q = tmp;
			}
		}
	}

}


  /// Constructors of the MovePicker class. As arguments we pass information
  /// to help it to return the (presumably) good moves first, to decide which
  /// moves to return (in the quiescence search, for instance, we only want to
  /// search captures, promotions, and some checks) and how important good move
  /// ordering is at the current node.

  /// MovePicker constructor for the main search
MovePicker::MovePicker(Minishogi& p, Move ttm, int d, const ButterflyHistory* mh,
	const CapturePieceToHistory* cph, const PieceToHistory** ch/*, Move cm, Move* killers*/)
	: pos(p), depth(d), mainHistory(mh), captureHistory(cph), contHistory(ch)
	/*,refutations{ { killers[0], 0 },{ killers[1], 0 },{ cm, 0 } } */{
	assert(d > 0);

#ifndef MOVEPICK_DISABLE
	stage = pos.IsInChecked() ? EVASION_TT : MAIN_TT;
	ttMove = ttm && pos.PseudoLegal(ttm) ? ttm : MOVE_NULL;
	stage += (ttMove == MOVE_NULL);
#else
	stage = NONSORT_INIT;
	ttMove = MOVE_NULL;
#endif
}

/// MovePicker constructor for quiescence search
MovePicker::MovePicker(Minishogi& p, Move ttm, int d, const ButterflyHistory* mh,
	const CapturePieceToHistory* cph, const PieceToHistory** ch, Square rs)
	: pos(p), mainHistory(mh), captureHistory(cph), contHistory(ch), recaptureSquare(rs), depth(d) {
	assert(d <= 0);

	if (pos.IsInChecked())
		stage = EVASION_TT;
	else if (d > -2)
		stage = QSEARCH_WITH_CHECKS_TT;
	else if (d > -10)
		stage = QSEARCH_NO_CHECKS_TT;
	else {
		stage = QSEARCH_RECAPTURES_INIT;
		return;
	}

	ttMove = ttm && pos.PseudoLegal(ttm) ? ttm : MOVE_NULL;
	stage += (ttMove == MOVE_NULL);
}

/// MovePicker constructor for ProbCut: we generate captures with SEE greater
/// than or equal to the given threshold.
/*
MovePicker::MovePicker(const Minishogi& p, Move ttm, Value th, const CapturePieceToHistory* cph)
: pos(p), captureHistory(cph), threshold(th) {
assert(!pos.checkers());

stage = PROBCUT_TT;
ttMove = ttm
&& pos.pseudo_legal(ttm)
&& pos.capture(ttm)
&& pos.see_ge(ttm, threshold) ? ttm : MOVE_NULL;
stage += (ttMove == MOVE_NULL);
}
*/

/// MovePicker::score() assigns a numerical value to each move in a list, used
/// for sorting. Captures are ordered by Most Valuable Victim (MVV), preferring
/// captures with a good history. Quiets moves are ordered using the histories.
void MovePicker::score(GenType type) {
	for (auto& m : *this) {
		const Square from = from_sq(m), to = to_sq(m);
		const Piece from_pc = pos.GetPiece(m), to_pc = pos.GetCapture(m);

		if (type == CAPTURES) {
			m.score = PIECE_SCORE[type_of(to_pc)]
				+ ((*captureHistory)[from_pc][to][type_of(to_pc)] >> 4);
		}
		else if (type == QUIETS) {
			m.score = (*mainHistory)[pos.GetTurn()][from][to]
					+ (*contHistory[0])[from_pc][to]
					+ (*contHistory[1])[from_pc][to]
					+ (*contHistory[3])[from_pc][to];
		}
		else { // Type == EVASIONS 
			if (to_pc) {
				m.score = PIECE_SCORE[type_of(to_pc)] - Value(type_of(from_pc));
			}
			else {
				m.score = (*mainHistory)[pos.GetTurn()][from][to] - (1 << 28);
			}
			if (type_of(from_pc) == KING) {
				m.score -= (1 << 10);
			}
		}
	}
}

/// MovePicker::select() returns the next move satisfying a predicate function.
/// It never returns the TT move.
template<MovePicker::PickType T, typename Pred>
Move MovePicker::select(Pred filter) {
	while (cur < endMoves) {
		if (T == Best)
			std::swap(*cur, *std::max_element(cur, endMoves));

		move = *cur++;

		if (move &&
			move != ttMove &&
			filter() &&
			(from_sq(move) >= BOARD_NB || !pos.IsInCheckedAfter(move)))
			return move;
	}
	return move = MOVE_NULL;
}

/// MovePicker::next_move() is the most important method of the MovePicker class. It
/// returns a new pseudo legal move every time it is called until there are no more
/// moves left, picking the move with the highest score from a list of generated moves.
Move MovePicker::GetNextMove() {
top:
	switch (stage) {
	case MAIN_TT:
	case EVASION_TT:
	case QSEARCH_WITH_CHECKS_TT:
	case QSEARCH_NO_CHECKS_TT:
	case PROBCUT_TT:
		stage++;
		return ttMove;

	case CAPTURE_INIT:
	case PROBCUT_INIT:
	case QCAPTURES_1_INIT:
	case QCAPTURES_2_INIT:
		cur = endBadCaptures = moves;
		endMoves = pos.AttackGenerator(cur);

		score(CAPTURES);

		stage++;
		goto top;

	case GOOD_CAPTURE:
		if (select<Best>([&]() {
			return pos.SEE(move) ? true : (*endBadCaptures++ = move, false);
		}))
			return move;

#ifndef REFUTATION_DISABLE
		// Prepare the pointers to loop over the refutations array
		cur = std::begin(refutations);
		endMoves = std::end(refutations);

		// If the countermove is the same as a killer, skip it
		if (refutations[0].move == refutations[2].move ||
			refutations[1].move == refutations[2].move)
			--endMoves;

		stage++;

	case REFUTATION:
		if (select<Next>([&]() { 
			return move != MOVE_NULL && !pos.GetCapture(move) && pos.PseudoLegal(move);
		}))
			return move;
		stage++;
#else
		stage += 2;

#endif

	case QUIET_INIT:
		cur = endBadCaptures;
		endMoves = pos.MoveGenerator(cur);
		endMoves = pos.HandGenerator(endMoves);

		score(QUIETS);
		partial_insertion_sort(cur, endMoves, -4000 * depth);

		stage++;

	case QUIET:
		if (select<Next>(
#ifdef REFUTATION_DISABLE
			Any
#else
			[&]() {
			return move != refutations[0] && move != refutations[1] && move != refutations[2];
		}
#endif
		))
			return move;

		// Prepare the pointers to loop over the bad captures
		cur = moves;
		endMoves = endBadCaptures;

		stage++;

	case BAD_CAPTURE:
		return select<Next>(Any);

	case EVASION_INIT:
		cur = moves;
		endMoves = pos.AttackGenerator(cur);
		endMoves = pos.MoveGenerator(endMoves);
		endMoves = pos.HandGenerator(endMoves);

		score(EVASIONS);

		stage++;

	case EVASION:
		return select<Best>(Any);

	case PROBCUT:
		return select<Best>([&]() { return pos.SEE(move, threshold); });

	case QCAPTURES_1:
	case QCAPTURES_2:
		if (select<Best>(Any))
			return move;

		if (stage == QCAPTURES_2)
			break;

		cur = moves;
		endMoves = pos.HandGenerator(cur);
		endMoves = pos.MoveGenerator(endMoves);

		stage++;
	case QCHECKS:
		return select<Next>([&]() { return pos.IsCheckingAfter(move); });

	case QSEARCH_RECAPTURES_INIT:
		cur = moves;
		endMoves = pos.AttackGenerator(cur, 1 << recaptureSquare);

		score(CAPTURES);

		stage++;

	case QRECAPTURES:
		return select<Best>(Any);

	case NONSORT_INIT:
		cur = moves;
		endMoves = pos.AttackGenerator(cur);
		endMoves = pos.MoveGenerator(endMoves);
		endMoves = pos.HandGenerator(endMoves);

		stage++;

	case NONSORT:
		return select<Next>(Any);
	}

	return MOVE_NULL;
}