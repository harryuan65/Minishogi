#ifndef _TYPES_
#define _TYPES_
#include <sstream>

using namespace std;

typedef uint64_t Key;

constexpr int SINGLE_GENE_MAX_ACTIONS = 112;
constexpr int TOTAL_GENE_MAX_ACTIONS = 162;  // AtkGene 21, MoveGene 29, HandGene 112
constexpr int DEPTH_QUIET_MAX = -5;
constexpr int DEPTH_QS_CHECKS = 0;
constexpr int DEPTH_QS_RECAPTURES = -5;
constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY = 128;
constexpr int MAX_HISTORY_PLY = 256;

enum Color : int {
	WHITE,
	BLACK,
	COLOR_NB = 2
};

enum Value : int {
	VALUE_ZERO = 0,
	VALUE_MATE = 30000,
	VALUE_INFINITE = 31001,
	VALUE_MATE_IN_MAX_PLY = (int)VALUE_MATE - 2 * MAX_PLY,
	VALUE_MATED_IN_MAX_PLY = (int)-VALUE_MATE + 2 * MAX_PLY,
	VALUE_NONE = 32002
};

enum Square : int {
	SQ_A5, SQ_A4, SQ_A3, SQ_A2, SQ_A1,
	SQ_B5, SQ_B4, SQ_B3, SQ_B2, SQ_B1,
	SQ_C5, SQ_C4, SQ_C3, SQ_C2, SQ_C1,
	SQ_D5, SQ_D4, SQ_D3, SQ_D2, SQ_D1,
	SQ_E5, SQ_E4, SQ_E3, SQ_E2, SQ_E1,
	SQ_F5, SQ_F4, SQ_F3, SQ_F2, SQ_F1,
	SQ_G5, SQ_G4, SQ_G3, SQ_G2, SQ_G1,
	BOARD_NB = 25,
	SQUARE_NB = 35
};

/// | isPro 1bit | srcSq 6bits | dstSq 6bits |
enum Move : int {
	MOVE_NULL,
	MOVE_UNDO = 1 << 13,
	MOVE_SAVEBOARD = 2 << 13,
	MOVE_ILLEGAL = 3 << 13
};

enum Chess : int {
	EMPTY = 0,
	W_PAWN = 1, //步
	W_SILVER = 2, //銀
	W_GOLD = 3, //金
	W_BISHOP = 4, //角
	W_ROOK = 5, //飛
	W_KING = 6, //王
	B_PAWN = 17, //步
	B_SILVER = 18, //銀
	B_GOLD = 19, //金
	B_BISHOP = 20, //角
	B_ROOK = 21, //飛
	B_KING = 22, //王
	CHESS_NB = 30,

	PAWN = 1, //步
	SILVER = 2, //銀
	GOLD = 3, //金
	BISHOP = 4, //角
	ROOK = 5, //飛
	KING = 6, //王
	CHESS_TYPE_NB = 14,

	PROMOTE = 0x08,
	BLACKCHESS = 0x10
};

struct ExtMove {
	Move move;
	int value;

	operator Move() const { return move; }
	void operator=(Move m) { move = m; }

	// Inhibit unwanted implicit conversions to Move
	// with an ambiguity that yields to a compile error.
	operator float() const = delete;
};

inline bool operator<(const ExtMove& f, const ExtMove& s) {
	return f.value < s.value;
}

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, T d2) { return T(int(d1) + int(d2)); } \
constexpr T operator-(T d1, T d2) { return T(int(d1) - int(d2)); } \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; }         \
inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
ENABLE_INCR_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

ENABLE_FULL_OPERATORS_ON(Value)

ENABLE_INCR_OPERATORS_ON(Color)
ENABLE_INCR_OPERATORS_ON(Square)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON

/// Additional operators to add integers to a Value
constexpr Value operator+(Value v, int i) { return Value(int(v) + i); }
constexpr Value operator-(Value v, int i) { return Value(int(v) - i); }
inline Value& operator+=(Value& v, int i) { return v = v + i; }
inline Value& operator-=(Value& v, int i) { return v = v - i; }

constexpr Value mate_in(int ply) {
	return VALUE_MATE - ply;
}

constexpr Value mated_in(int ply) {
	return -VALUE_MATE + ply;
}

constexpr Color operator~(Color c) {
	return Color(c ^ BLACK); // Toggle color
}

constexpr Color color_of(Chess c) {
	return Color(c & 0x10);
}

/// Get srcIndex from move
constexpr Square from_sq(Move m) {
	return Square((m >> 6) & 0x3F);
}

/// Get dstIndex from move
constexpr Square to_sq(Move m) {
	return Square(m & 0x3F);
}

/// Get isPro from move
constexpr bool is_pro(Move m) {
	return (m >> 12) & 0x1;
}

inline Move make_move(Square from, Square to, bool isPro) {
	return Move(((isPro << 12) | from << 6) | to);
}

/// Exclude MOVE_NULL, MOVE_UNDO, MOVE_SAVEBOARD, MOVE_ILLEGAL
constexpr bool IsDoMove(Move m) {
	return !(m >> 13);
}

/// uint32_t -> Move, for Communication
constexpr Move setU32(uint32_t u) {
	if (!u) {
		return MOVE_NULL;
	}
	return make_move(Square(u & 0x000003f), Square((u & 0x0000fc0) >> 6), (u & 0x1000000) >> 24);
}

/// Move -> uint_t, for Communication
constexpr int toU32(Move m) {
	if (!IsDoMove(m)) {
		return 0;
	}
	return (is_pro(m) << 24) | (to_sq(m) << 6) | from_sq(m);
}

static inline Square Input2Index(char row, char col) {
	row = toupper(row);
	if ('A' <= row && row <= 'G' && '1' <= col && col <= '5') {
		return (Square)((row - 'A') * 5 + '5' - col);
	}
	return (Square)-1;
}

static inline string Index2Input(Square index) {
	if (0 <= index && index < 35) {
		string str;
		str.push_back('A' + index / 5);
		str.push_back('5' - index % 5);
		return str;
	}
	return "";
}

static istream& operator>>(istream &is, Move& m) {
	string str;
	is >> str;
	if (str == "SURRENDER" || str == "surrender") {
		m = MOVE_NULL;
	}
	else if (str == "UNDO" || str == "undo") {
		m = MOVE_UNDO;
	}
	else if (str == "SAVEBOARD" || str == "saveboard") {
		m = MOVE_SAVEBOARD;
	}
	else if (str.length() != 4 && (str.length() == 5 && str[4] != '+')) {
		m = MOVE_ILLEGAL;
	}
	else {
		m = make_move(Input2Index(str[0], str[1]),
					  Input2Index(str[2], str[3]),
			          str.length() == 5);
	}
	return is;
}

static ostream& operator<< (ostream& os, const Move& m) {
	switch (m) {
	case MOVE_NULL:
		os << "SURRENDER";
		break;
	case MOVE_UNDO:
		os << "UNDO";
		break;
	case MOVE_SAVEBOARD:
		os << "SAVEBOARD";
		break;
	default:
		os << Index2Input(from_sq(m)) << Index2Input(to_sq(m)) << (is_pro(m) ? "+" : " ");
	}
	return os;
}



static const char CHESS_WORD[][3] = {
	"  ","步","銀","金","角","飛","王","  ",
	"  ","ㄈ","全","  ","馬","龍","  ","  ",
	"  ","步","銀","金","角","飛","玉","  ",
	"  ","ㄈ","全","  ","馬","龍"
};

static const char SAVE_CHESS_WORD[][5] = {
	" ． ","△步","△銀","△金","△角","△飛","△王","    ",
	"    ","△ㄈ","△全","    ","△馬","△龍","    ","    ",
	"    ","▼步","▼銀","▼金","▼角","▼飛","▼玉","    ",
	"    ","▼ㄈ","▼全","    ","▼馬","▼龍"
};

static const int EatToHand[] = {
	0, 25, 26, 27, 28, 29, 0, 0,
	0, 25, 26,  0, 28, 29, 0, 0,
	0, 30, 31, 32, 33, 34, 0, 0,
	0, 30, 31,  0, 33, 34
};

static const Chess HandToChess[] = {
	EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,
	EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,
	EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,
	EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,
	EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,
	B_PAWN,B_SILVER,B_GOLD,B_BISHOP,B_ROOK,
	W_PAWN,W_SILVER,W_GOLD,W_BISHOP,W_ROOK
};

static const bool Promotable[] = {
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false, false, false,
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false
};

static const int CHESS_SCORE[] = {
	0,  107,  810,  907,  1291,  1670, 0, 0,
	0,  895,  933,    0,  1985,  2408, 0, 0,
	0, -107, -810, -907, -1291, -1670, 0, 0,
	0, -895, -933,    0, -1985, -2408
};

static const int HAND_SCORE[] = {
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	-152, -1110, -1260, -1464, -1998,
	 152,  1110,  1260,  1464,  1998
};

constexpr Chess promote(Chess c) {
	return Chess(c ^ PROMOTE);
}

constexpr Chess type_of(Chess c) {
	return Chess(c & 0xF);
}

constexpr Chess type_of(int c) {
    return Chess(c & 0xF);
}

constexpr bool is_sniper(Chess c) {
	return (c & 0x7) == BISHOP || (c & 0x7) == ROOK;
}
#endif