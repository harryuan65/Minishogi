/*
+Example Board
▼飛▼角▼銀▼金▼玉
．  ．  ．  ． ▼步
．  ．  ．  ．  ．
△步  ．  ．  ．  ．
△王△金△銀△角△飛
▼0步0銀0金0角0飛
△0步0銀0金0角0飛
*/
#ifndef _BOARD_
#define _BOARD_
#include "head.h"

#define BOARD_PATH   "board//"
#define KIFU_PATH    "output//"

class Board {
private:
    bool m_turn;
	int m_step;
	int m_evaluate;
	Zobrist::Zobrist m_hashcode;
	Zobrist::Zobrist m_hashcode2;

	//第i個recordAction動完後的Zobrist儲存在第i+1個recordZobrist
	//recordZobrist第0個是初始盤面
	Action recordAction[120];
	Zobrist::Zobrist recordZobrist[121];
	Zobrist::Zobrist recordZobrist2[121];

public:
    U32 occupied[2];
    U32 bitboard[32];
    int board[TOTAL_BOARD_SIZE];

    void Initialize();
    void Initialize(const char *str);
    void PrintChessBoard() const;
	void PrintNoncolorBoard(ostream &os) const;

	void DoMove(Action m_Action);
	void UndoMove();

    bool SaveBoard(string filename, string comment) const;
    bool LoadBoard(string filename, streamoff &offset);
	bool SaveKifu(string filename) const;

	bool IsGameOver();
	bool IsSennichite() const;
	bool IsCheckAfter(const int src, const int dst);
	inline void NextTurn() { m_turn ^= 1; }
    inline int GetTurn() const { return m_turn; }
	inline int GetStep() const { return m_step; }
	inline int GetEvaluate() const { return m_turn ? m_evaluate : -m_evaluate; };
	inline Zobrist::Zobrist GetZobristHash() const { return m_turn ? m_hashcode2 : m_hashcode; }
	unsigned int GetKifuHash() const ;
};

#endif