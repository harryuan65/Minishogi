#ifndef _LIBRARY_
#define _LIBRARY_
#include <string>
using namespace std;

#define WINDOWS_10
#define PV_DISABLE
//#define BEST_ENDGAME_SEARCH
//#define TRANSPOSITION_DISABLE

typedef unsigned __int64 U64;
typedef unsigned __int32 U32;

/*    Minishogi    */
const int BOARD_SIZE = 25;
const int TOTAL_BOARD_SIZE = 37;

/*    Chess    */
enum Chess {
    BLANK  = 0,
    PAWN   = 1, //步
	SILVER = 2, //銀
	GOLD   = 3, //金
	BISHOP = 4, //角
	ROOK   = 5, //飛
	KING   = 6, //王
	PROMOTE    = 0x08,
	BLACKCHESS = 0x10,
	COUNT  = 30
};


const char CHESS_WORD[][3] = {
	"  ","步","銀","金","角","飛","王","  ",
	"  ","ㄈ","全","  ","馬","龍","  ","  ",
	"  ","步","銀","金","角","飛","玉","  ",
	"  ","ㄈ","全","  ","馬","龍"
};

const char SAVE_CHESS_WORD[][5] = {
	" ． ","△步","△銀","△金","△角","△飛","△王","    ",
	"    ","△ㄈ","△全","    ","△馬","△龍","    ","    ",
	"    ","▼步","▼銀","▼金","▼角","▼飛","▼玉","    ",
	"    ","▼ㄈ","▼全","    ","▼馬","▼龍"
};

const int EatToHand[] = {
	0, 25, 26, 27, 28, 29, 35, 0,
	0, 25, 26,  0, 28, 29,  0, 0,
	0, 30, 31, 32, 33, 34, 36, 0,
	0, 30, 31,  0, 33, 34
};

const int HandToChess[] = {
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	17, 18, 19, 20, 21,
	1,  2,  3,  4,  5,
	22,  6,
};

const int CHECKMATE = 30000; // 避免PERFECT_ENDGAME_PV時超過SHRT_MAX

const bool Promotable[] = {
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false, false, false,
	false,  true,  true, false,  true,  true, false, false,
	false, false, false, false, false, false
};

const int CHESS_SCORE[] = {
	0, -107, -810, -907, -1291, -1670, 0, 0,
	0, -895, -933,    0, -1985, -2408, 0, 0,
	0,  107,  810,  907,  1291,  1670, 0, 0,
	0,  895,  933,    0,  1985,  2408
};

const int HAND_SCORE[] = {
	152,  1110,  1260,  1464,  1998,
	-152, -1110, -1260, -1464, -1998
};

/*    Action    */
typedef unsigned __int32 Action;

const U32 PRO_MASK = 1 << 24;
const U32 ACTION_SURRENDER = 1 << 25;
const U32 ACTION_UNDO = 1 << 26;
const U32 ACTION_SAVEBOARD = 1 << 27;

inline int ACTION_TO_SRCINDEX(Action action) { return  action & 0x0000003f; }
inline int ACTION_TO_DSTINDEX(Action action) { return (action & 0x00000fc0) >> 6; }
inline int ACTION_TO_SRCCHESS(Action action) { return (action & 0x0003f000) >> 12; }
inline int ACTION_TO_DSTCHESS(Action action) { return (action & 0x00fc0000) >> 18; }
inline bool ACTION_TO_ISPRO(Action action) { return  action & PRO_MASK; }


/*struct Action{
	unsigned __int32 action;

	inline int srcIndex() { return  action & 0x0000003f; }
	inline int dstIndex() { return (action & 0x00000fc0) >> 6; }
	inline int srcChess() { return (action & 0x0003f000) >> 12; }
	inline int dstChess() { return (action & 0x00fc0000) >> 18; }
	inline bool isPro()   { return action & PRO_MASK; }
	inline bool isSurrender() { return action & (1 << 25); }
	inline bool isUndo()      { return action & (1 << 26); }
	inline bool isSaveBoard() { return action & (1 << 27); }
};*/

/*    Action Array    */
// max move number including attack (21) and move (29)
// and the hand chess (112) (23 * 4 + 20)
const int MAX_MOVE_NUM = 112; // max 162 ; testing max

inline int Input2Index(char row, char col) {
	row = toupper(row);
	if ('A' <= row && row <= 'G' && '1' <= col && col <= '5') {
		return (row - 'A') * 5 + '5' - col;
	}
	return -1;
}

inline string Index2Input(int index) {
	if (0 <= index && index < 35) {
		string str;
		str.push_back('A' + index / 5);
		str.push_back('5' - index % 5);
		return str;
	}
	return "";
}

inline string Action2String(Action action) {
	if (action == ACTION_SURRENDER) {
		return "SURRENDER";
	}
	else if (action == ACTION_UNDO) {
		return "UNDO";
	}
	else if (action == ACTION_SAVEBOARD) {
		return "SAVEBOARD";
	}
	else {
		return Index2Input(ACTION_TO_SRCINDEX(action)) +
			Index2Input(ACTION_TO_DSTINDEX(action)) +
			(ACTION_TO_ISPRO(action) ? "+" : " ");
	}
}
#endif