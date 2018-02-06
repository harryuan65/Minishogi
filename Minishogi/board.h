#ifndef _BOARD_
#define _BOARD_
#include "head.h"

#define BOARD_PATH   "board//"
#define LBOARD_PATH L"board//"
#define KIFU_PATH    "output//"
#define LKIFU_PATH  L"output//"

class Board {
private:
    bool m_turn;
	int m_step;
	int m_evaluate;
	Zobrist::HalfZobrist m_whiteHashcode;
	Zobrist::HalfZobrist m_blackHashcode;

	//第i個recordAction動完後的Zobrist儲存在第i+1個recordZobrist
	//recordZobrist第0個是初始盤面
	Action recordAction[120];
	Zobrist::Zobrist recordZobrist[121];
	//Zobrist::HalfZobrist recordZobristWhite[121];
	//Zobrist::HalfZobrist recordZobristBlack[121];

public:
    U32 occupied[2];
    U32 bitboard[32];
    int board[TOTAL_BOARD_SIZE];

    Board();
    ~Board();
    void Initialize();
    void Initialize(const char *str);
    void PrintChessBoard() const;
	void PrintNoncolorBoard(ostream &os) const;
	void CalZobristNumber();
	void CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess);

	void DoMove(Action m_Action);
	void UndoMove();

    bool SaveBoard(const string filename, const string comment) const;
    bool LoadBoard(const string filename, streamoff &offset);
	bool SaveKifu(const string filename, const string comment) const;

	bool IsGameOver();
	bool IsSennichite() const;
	bool IsCheckAfter(const int src, const int dst);
    inline int GetTurn() const { return m_turn; }
	inline int GetStep() const { return m_step; }
	inline int Evaluate() const { return m_turn ? m_evaluate : -m_evaluate; };
	inline Zobrist::Zobrist GetZobristHash() const { 
		return Zobrist::GetZobristHash(m_whiteHashcode, m_blackHashcode, m_turn); 
	}
	unsigned int GetKifuHash() const ;
};

#endif