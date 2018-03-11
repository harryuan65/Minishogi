#ifndef _CHESS_
#define _CHESS_

enum {
    BLANK  = 0,
    PAWN   = 1, //�B
	SILVER = 2, //��
	GOLD   = 3, //��
	BISHOP = 4, //��
	ROOK   = 5, //��
	KING   = 6, //��
	PROMOTE    = 0x08,
	BLACKCHESS = 0x10,
	CHESSCOUNT = 30
};

static const char CHESS_WORD[][3] = {
	"  ","�B","��","��","��","��","��","  ",
	"  ","�w","��","  ","��","�s","  ","  ",
	"  ","�B","��","��","��","��","��","  ",
	"  ","�w","��","  ","��","�s"
};

static const char SAVE_CHESS_WORD[][5] = {
	" �D ","���B","����","����","����","����","����","    ",
	"    ","���w","����","    ","����","���s","    ","    ",
	"    ","���B","����","����","����","����","����","    ",
	"    ","���w","����","    ","����","���s"
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

static const int CHECKMATE = 30000; // �קKPERFECT_ENDGAME_PV�ɶW�LSHRT_MAX

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