#ifndef _CHESS_
#define _CHESS_

enum {
    BLANK  = 0,
    PAWN   = 1, //步
	SILVER = 2, //銀
	GOLD   = 3, //金
	BISHOP = 4, //角
	ROOK   = 5, //飛
	KING   = 6, //王
	PROMOTE    = 0x08,
	BLACKCHESS = 0x10,
	CHESSCOUNT = 30
};

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

static const int HandToChess[] = {
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	17, 18, 19, 20, 21,
	1,  2,  3,  4,  5
};

static const bool Promotable[] = {
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false, false, false,
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false
};

static const int CHECKMATE = 30000; // 避免PERFECT_ENDGAME_PV時超過SHRT_MAX

static const int CHESS_SCORE[] = {
	0, -107, -810, -907, -1291, -1670, 0, 0,
	0, -895, -933,    0, -1985, -2408, 0, 0,
	0,  107,  810,  907,  1291,  1670, 0, 0,
	0,  895,  933,    0,  1985,  2408
};

static const int HAND_SCORE[] = {
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	152,  1110,  1260,  1464,  1998,
	-152, -1110, -1260, -1464, -1998
};

#endif