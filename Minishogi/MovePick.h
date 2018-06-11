#ifndef _MOVEPICK_
#define _MOVEPICK_

#include <array>
#include "Minishogi.h"

//#define MOVEPICK_DISABLE
#define REFUTATION_DISABLE

/// StatsEntry stores the stat table value. It is usually a number but could
/// be a move or even a nested history. We use a class instead of naked value
/// to directly call history update operator<<() on the entry so to use stats
/// tables at caller sites as simple multi-dim arrays.
template<typename T, int D>
class StatsEntry {

	static const bool IsInt = std::is_integral<T>::value;
	typedef typename std::conditional<IsInt, int, T>::type TT;

	T entry;

public:
	T * get() { return &entry; }
	void operator=(const T& v) { entry = v; }
	operator TT() const { return entry; }

	void operator<<(int bonus) {
		assert(abs(bonus) <= D);   // Ensure range is [-D, D]
		//static_assert(D <= std::numeric_limits<T>::max(), "D overflows T");

		entry += bonus - entry * abs(bonus) / D;

		assert(abs(entry) <= D);
	}
};

/// Stats is a generic N-dimensional array used to store various statistics.
/// The first template parameter T is the base type of the array, the second
/// template parameter D limits the range of updates in [-D, D] when we update
/// values with the << operator, while the last parameters (Size and Sizes)
/// encode the dimensions of the array.
template <typename T, int D, int Size, int... Sizes>
struct Stats : public std::array<Stats<T, D, Sizes...>, Size> {
	T* get() { return this->at(0).get(); }

	void fill(const T& v) {
		T* p = get();
		std::fill(p, p + sizeof(*this) / sizeof(*p), v);
	}
};

template <typename T, int D, int Size>
struct Stats<T, D, Size> : public std::array<StatsEntry<T, D>, Size> {
	T* get() { return this->at(0).get(); }
};

/// ButterflyHistory records how often quiet moves have been successful or
/// unsuccessful during the current search, and is used for reduction and move
/// ordering decisions. It uses 2 tables (one for each color) indexed by
/// the move's from and to squares, see chessprogramming.wikispaces.com/Butterfly+Boards
/// [color][srcIndex][dstIndex]
typedef Stats<int16_t, 10368, COLOR_NB, SQUARE_NB, BOARD_NB> ButterflyHistory;

/// CapturePieceToHistory is addressed by a move's [piece][to][captured piece type]
/// [Chese][dstIndex][dstChess type]
typedef Stats<int16_t, 10368, CHESS_NB, BOARD_NB, CHESS_TYPE_NB> CapturePieceToHistory;

/// PieceToHistory is like ButterflyHistory but is addressed by a move's [piece][to]
typedef Stats<int16_t, 29952, CHESS_NB, BOARD_NB> PieceToHistory;

/// CounterMoveHistory stores counter moves indexed by [piece][to] of the previous
/// move, see chessprogramming.wikispaces.com/Countermove+Heuristic
/// [現在在"前一步的dstIndex"位置上的Chess][前一步的dstIndex]
typedef Stats<Move, 0, CHESS_NB, SQUARE_NB> CounterMoveHistory;

enum GenType {
	CAPTURES,
	QUIETS,
	QUIET_CHECKS,
	EVASIONS,
	NON_EVASIONS,
	LEGAL
};

class Minishogi;

class MovePicker{

	enum PickType { Next, Best };

public:
	MovePicker(MovePicker&) = delete;
	MovePicker& operator=(const MovePicker&) = delete;
	//MovePicker(Minishogi&, Move, Value, const CapturePieceToHistory*);
	MovePicker(Minishogi&, Move, int depth, const ButterflyHistory*,  const CapturePieceToHistory*, Square);
	MovePicker(Minishogi&, Move, int depth, const ButterflyHistory*, const CapturePieceToHistory*, const PieceToHistory**, Move, Move*);
	Move GetNextMove(bool skipQuiets = false);

private:
	template<PickType T, typename Pred> Move select(Pred);
	void score(GenType type);
	ExtMove* begin() { return cur; }
	ExtMove* end() { return endMoves; }

	Minishogi& pos;
	const ButterflyHistory* mainHistory;
	const CapturePieceToHistory* captureHistory;
	const PieceToHistory** contHistory;
	Move ttMove;
	ExtMove refutations[3], *cur, *endMoves, *endBadCaptures;
	Move move;
	Square recaptureSquare;
	Value threshold;
	int depth;
	int stage;
	ExtMove moves[MAX_MOVES];
};

#endif