#ifndef _TYPES_H_
#define _TYPES_H_
#include <mutex>
#include <sstream>

#include "Observer.h"

enum SyncCout { IO_LOCK, IO_UNLOCK };
inline std::ostream& operator<<(std::ostream& os, SyncCout sc) {
	static std::mutex m;

	if (sc == IO_LOCK) m.lock();
	if (sc == IO_UNLOCK) m.unlock();

	return os;
}

#define sync_cout std::cout << IO_LOCK
#define sync_endl std::endl << IO_UNLOCK

typedef std::chrono::milliseconds::rep TimePoint;
inline TimePoint now() {
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::steady_clock::now().time_since_epoch()).count();
}

constexpr int SINGLE_GENE_MAX_MOVES = 112;
constexpr int TOTAL_GENE_MAX_MOVES = 162;  // AtkGene 21, MoveGene 29, HandGene 112
constexpr int MAX_SEARCH_DEPTH = 30;
constexpr int DEPTH_NONE = -256;
constexpr int MAX_PLY = 1024;
const std::string PIECE_2_CHAR = ".PSGBRK..PS.BR...psgbrk..ps.br";
const std::string HAND_2_CHAR = "PSGBRpsgbr";
const std::string PIECE_WORD = "  步銀金角飛王    ㄈ全  馬龍      步銀金角飛玉    ㄈ全  馬龍";
const std::string NONCOLOR_PIECE_WORD = " ． △步△銀△金△角△飛△王        △ㄈ△全    △馬△龍            ▼步▼銀▼金▼角▼飛▼玉        ▼ㄈ▼全    ▼馬▼龍";
const std::string COLOR_WORD[] = { "△", "▼" };

enum Turn : int {
	WHITE,
	BLACK,
	COLOR_NB = 2
};

enum Value : int {
	VALUE_ZERO			= 0,
	VALUE_PAWN			= 107,
	VALUE_SILVER		= 810,
	VALUE_GOLD			= 907,
	VALUE_BISHOP		= 1291,
	VALUE_ROOK			= 1670,
	VALUE_PRO_PAWN		= 895,
	VALUE_PRO_SILVER	= 933,
	VALUE_PRO_BISHOP	= 1985,
	VALUE_PRO_ROOK		= 2408,
	VALUE_HAND_PAWN		= 152,
	VALUE_HAND_SILVER	= 1110,
	VALUE_HAND_GOLD		= 1260,
	VALUE_HAND_BISHOP	= 1464,
	VALUE_HAND_ROOK		= 1998,

	PIN_PAWN			= 2,
	PIN_SILVER			= -204,
	PIN_GOLD			= -377,
	PIN_BISHOP			= -375,
	PIN_ROOK			= -500,
	PIN_PRO_PAWN		= PIN_PAWN,
	PIN_PRO_SILVER		= PIN_SILVER,
	PIN_PRO_BISHOP		= PIN_BISHOP,
	PIN_PRO_ROOK		= PIN_ROOK,

	VALUE_KNOWN_WIN		= 10000,
	VALUE_MATE			= 30000,
	VALUE_INFINITE		= 31001,
	VALUE_NONE			= 32002,
	VALUE_SENNICHITE	= 512,
	VALUE_SENNI_IN_MAX_COUNT = (int)VALUE_MATE - 2 * (int)VALUE_SENNICHITE - 1,
	VALUE_MATE_IN_MAX_PLY  = (int) VALUE_MATE - 2 * MAX_SEARCH_DEPTH,
	VALUE_MATED_IN_MAX_PLY = (int)-VALUE_MATE + 2 * MAX_SEARCH_DEPTH
};

enum Square : int {
	SQ_A5, SQ_A4, SQ_A3, SQ_A2, SQ_A1,
	SQ_B5, SQ_B4, SQ_B3, SQ_B2, SQ_B1,
	SQ_C5, SQ_C4, SQ_C3, SQ_C2, SQ_C1,
	SQ_D5, SQ_D4, SQ_D3, SQ_D2, SQ_D1,
	SQ_E5, SQ_E4, SQ_E3, SQ_E2, SQ_E1,
	SQ_F5, SQ_F4, SQ_F3, SQ_F2, SQ_F1,
	SQ_G5, SQ_G4, SQ_G3, SQ_G2, SQ_G1,
	SQ_NONE,
	SQ_W_HAND = 25,
	SQ_B_HAND = 30,
	SQUARE_ZERO = 0,
	BOARD_NB    = 25,
	SQUARE_NB   = 35
};

/// | isPro 1bit | srcSq 6bits | dstSq 6bits |
enum Move : int {
	MOVE_NULL = 0,
	MOVE_UNDO = 1 << 13,
	//MOVE_SAVEBOARD = 2 << 13,
	MOVE_NONE = 3 << 13
};

/// | color 1bit | isPro 1bit | baseType 3bits |
enum Piece : int {
	NO_PIECE     = 0,
	W_PAWN       = 1,  //步
	W_SILVER     = 2,  //銀
	W_GOLD       = 3,  //金
	W_BISHOP     = 4,  //角
	W_ROOK       = 5,  //飛
	W_KING       = 6,  //王
	W_PRO_PAWN   = 9,  //ㄈ
	W_PRO_SILVER = 10, //全
	W_PRO_BISHOP = 12, //馬
	W_PRO_ROOK   = 13, //龍
	B_PAWN       = 17, //步
	B_SILVER     = 18, //銀
	B_GOLD       = 19, //金
	B_BISHOP     = 20, //角
	B_ROOK       = 21, //飛
	B_KING       = 22, //王
	B_PRO_PAWN   = 25, //ㄈ
	B_PRO_SILVER = 26, //全
	B_PRO_BISHOP = 28, //馬
	B_PRO_ROOK   = 29, //龍
	PIECE_NB     = 30,

	PAWN         = 1,  //步
	SILVER       = 2,  //銀
	GOLD         = 3,  //金
	BISHOP       = 4,  //角
	ROOK         = 5,  //飛
	KING         = 6,  //王
	PRO_PAWN     = 9,  //ㄈ
	PRO_SILVER   = 10, //全
	PRO_BISHOP   = 12, //馬
	PRO_ROOK     = 13, //龍
	PIECE_TYPE_NB= 14,

	PROMOTE = 0x08,
	BLACKCHESS = 0x10
};

// 棋子種類 + 國籍 + 位置
enum BonaPiece : int {
	BONA_PIECE_ZERO = 0,
	F_PIECE = 0,
	F_HAND = F_PIECE + PIECE_NB * BOARD_NB,
	E_HAND = F_HAND + 10,
	BONA_PIECE_NB = E_HAND + 10
};

// BonaPieceList Index
enum BonaPieceIndex : int {
	BPI_PAWN   = 0,
	BPI_SILVER = 2,
	BPI_GOLD   = 4,
	BPI_BISHOP = 6,
	BPI_ROOK   = 8,
	BPI_KING   = 10,
	BPI_NONE   = 12,
	BONA_PIECE_INDEX_NB = 12
};

struct ExtMove {
	Move move;
	int score;

	operator Move() const { return move; }
	void operator=(const Move m) { move = m; }

	operator float() const = delete;
};

inline bool operator<(const ExtMove& f, const ExtMove& s) {
	return f.score < s.score;
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
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }    \
static std::istream& operator>>(std::istream &is, T &d) { is >> d; return is; }

ENABLE_FULL_OPERATORS_ON(Value)

ENABLE_INCR_OPERATORS_ON(Turn)
ENABLE_BASE_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(BonaPiece)
ENABLE_INCR_OPERATORS_ON(BonaPieceIndex)

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

constexpr Turn operator~(Turn c) {
	return Turn(c ^ BLACK);
}

constexpr bool is_drop(Square sq) {
	return sq >= BOARD_NB;
}

constexpr Square rotate_board_sq(Square sq) {
	return SQ_E1 - sq;
}

constexpr Square mirror_board_sq(Square sq) {
	return (Square)(sq + 4 - (sq % 5) * 2);
}

/// Return srcIndex of move
constexpr Square from_sq(Move m) {
	return Square((m >> 6) & 0x3F);
}

/// Return dstIndex of move
constexpr Square to_sq(Move m) {
	return Square(m & 0x3F);
}

/// Return isPro from move
constexpr bool is_promote(Move m) {
	return (m >> 12) & 0x1;
}

constexpr Move make_move(Square from, Square to, bool isPro) {
	return Move(((isPro << 12) | from << 6) | to);
}

/// Exclude MOVE_NULL, MOVE_UNDO, MOVE_SAVEBOARD, MOVE_NONE
constexpr bool IsDoMove(Move m) {
	return m && !(m >> 13);
}

/// uint32_t -> Move, for Communication
const static Move setU32(uint32_t u) {
	if (!u)
		return MOVE_NULL;
	return make_move(Square(u & 0x000003f), Square((u & 0x0000fc0) >> 6), (u & 0x1000000) >> 24);
}

/// Move -> uint_t, for Communication
const static int toU32(Move m) {
	if (!IsDoMove(m))
		return 0;
	return (is_promote(m) << 24) | (to_sq(m) << 6) | from_sq(m);
}

static Move usi2move(std::string str, Turn turn) {
	Move m;
	if (str == "resign")
		m = MOVE_NULL;
	else if (str == "undo")
		m = MOVE_UNDO;
	else if (str.length() != 4 && (str.length() == 5 && str[4] != '+'))
		m = MOVE_NONE;
	else if (str[1] == '*')
		if (HAND_2_CHAR.find(str[0]) == std::string::npos)
			m = MOVE_NONE;
		else
			m = make_move(Square(HAND_2_CHAR.find(str[0]) + (turn ? SQ_B_HAND : SQ_W_HAND)),
				Square((str[3] - 'a') * 5 + '5' - str[2]), false);
	else
		m = make_move(Square((str[1] - 'a') * 5 + '5' - str[0]),
			Square((str[3] - 'a') * 5 + '5' - str[2]), 
			str.length() == 5);
	return m;
}

static std::ostream& operator<<(std::ostream& os, Move m) {
	switch (m) {
	case MOVE_NULL:
		os << "resign"; //resign
		break;
	case MOVE_UNDO:
		os << "undo";
		break;
	case MOVE_NONE:
		os << "none";
		break;
	default:
		if (from_sq(m) >= BOARD_NB)
			os << HAND_2_CHAR[from_sq(m) % 5] << "*";
		else
			os << char('5' - from_sq(m) % 5) << char('a' + from_sq(m) / 5);
		os << char('5' - to_sq(m) % 5) << char('a' + to_sq(m) / 5) << (is_promote(m) ? "+" : "");
	}
	return os;
}

constexpr Piece promote(Piece p) {
	return Piece(p | PROMOTE);
}

constexpr Piece toggle_promote(Piece p) {
	return Piece(p ^ PROMOTE);
}

constexpr Piece type_of(Piece p) {
	return Piece(p & 0xF);
}

constexpr Piece type_of(int p) {
    return Piece(p & 0xF);
}

constexpr Turn color_of(Piece p) {
	return Turn(p >= BLACKCHESS);
}

constexpr bool is_promote(Piece c) {
	return c & PROMOTE;
}

constexpr BonaPiece to_bonapiece(Square sq, int c) {
	if (sq < BOARD_NB)
		return BonaPiece(c * BOARD_NB + sq);
	else
		return BonaPiece(F_HAND + (sq - SQ_W_HAND) * 2 + c - 1);
}

constexpr BonaPiece to_inv_bonapiece(Square sq, int c) {
	if (sq < BOARD_NB)
		return BonaPiece((c ^ BLACKCHESS)*BOARD_NB + rotate_board_sq(sq));
	else if (sq < SQ_B_HAND)
		return BonaPiece(E_HAND + (sq - SQ_W_HAND) * 2 + c - 1);
	else
		return BonaPiece(F_HAND + (sq - SQ_B_HAND) * 2 + c - 1);
}

constexpr BonaPiece mirror_bonapiece(BonaPiece bp) {
	Square sq = Square(bp % BOARD_NB);
	return bp < F_HAND ? BonaPiece(mirror_board_sq(sq) + bp - sq) : bp;
}

constexpr bool is_sniper(Piece c) {
	return (c & 0x7) == BISHOP || (c & 0x7) == ROOK;
}

static std::string fen2sfen(std::string fen) {
	std::string board, hand, turn;
	std::istringstream iss(fen);
	getline(iss, board, '[');
	getline(iss, hand, ']');
	iss >> turn;
	return board + " " + (turn == "w" ? "b" : "w") + " " + hand + " 1";
}

static inline std::string get_extension(std::string file) {
	return file.substr(file.find_last_of(".") + 1);
}

constexpr Square EatToHand[] = {
	SQ_NONE, SQ_G5, SQ_G4,   SQ_G3, SQ_G2, SQ_G1, SQ_NONE, SQ_NONE,
	SQ_NONE, SQ_G5, SQ_G4, SQ_NONE, SQ_G2, SQ_G1, SQ_NONE, SQ_NONE,
	SQ_NONE, SQ_F5, SQ_F4,   SQ_F3, SQ_F2, SQ_F1, SQ_NONE, SQ_NONE,
	SQ_NONE, SQ_F5, SQ_F4, SQ_NONE, SQ_F2, SQ_F1
};

constexpr Piece HandToPiece[] = {
	NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
	NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
	NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
	NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
	NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE,
	  W_PAWN, W_SILVER,   W_GOLD, W_BISHOP,   W_ROOK,
	  B_PAWN, B_SILVER,   B_GOLD, B_BISHOP,   B_ROOK
};

constexpr bool Promotable[] = {
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false, false, false,
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false
};

constexpr Value PIECE_SCORE[] = {
	VALUE_ZERO, VALUE_PAWN,  VALUE_SILVER, VALUE_GOLD, VALUE_BISHOP, VALUE_ROOK, VALUE_MATE, VALUE_ZERO,
	VALUE_ZERO, VALUE_PRO_PAWN, VALUE_PRO_SILVER, VALUE_ZERO, VALUE_PRO_BISHOP, VALUE_PRO_ROOK, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, -VALUE_PAWN, -VALUE_SILVER, -VALUE_GOLD, -VALUE_BISHOP, -VALUE_ROOK, -VALUE_MATE, VALUE_ZERO,
	VALUE_ZERO, -VALUE_PRO_PAWN, -VALUE_PRO_SILVER, VALUE_ZERO, -VALUE_PRO_BISHOP, -VALUE_PRO_ROOK
};

constexpr Value HAND_SCORE[] = {
	VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO, VALUE_ZERO,
	VALUE_HAND_PAWN, VALUE_HAND_SILVER, VALUE_HAND_GOLD, VALUE_HAND_BISHOP, VALUE_HAND_ROOK,
	-VALUE_HAND_PAWN, -VALUE_HAND_SILVER, -VALUE_HAND_GOLD, -VALUE_HAND_BISHOP, -VALUE_HAND_ROOK
};

constexpr Value PIN_SCORE[] = {
	VALUE_ZERO, PIN_PAWN, PIN_SILVER, PIN_GOLD, PIN_BISHOP, PIN_ROOK, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, PIN_PRO_PAWN, PIN_PRO_SILVER, VALUE_ZERO, PIN_PRO_BISHOP, PIN_PRO_ROOK, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, -PIN_PAWN, -PIN_SILVER, -PIN_GOLD, -PIN_BISHOP, -PIN_ROOK, VALUE_ZERO, VALUE_ZERO,
	VALUE_ZERO, -PIN_PRO_PAWN, -PIN_PRO_SILVER, VALUE_ZERO, -PIN_PRO_BISHOP, -PIN_PRO_ROOK
};

#endif