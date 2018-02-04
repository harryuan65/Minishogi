#ifndef _BOARD_
#define _BOARD_
#include "head.h"

#define BOARD_PATH   "board//"
#define LBOARD_PATH L"board//"
#define KIFU_PATH    "output//"
#define LKIFU_PATH  L"output//"

class Board {
private:
    static vector<U32> ZOBRIST_TABLE[TOTAL_BOARD_SIZE];
    static const int ZOBRIST_SEED = 10;

    bool m_turn;
	int m_step;
	int m_evaluate;
    U32 m_whiteHashcode;
    U32 m_blackHashcode;

	//第i個recordAction動完後的Zobrist儲存在第i+1個recordZobrist
	//recordZobrist第0個是初始盤面
	Action recordAction[120];
	U64 recordZobrist[121];
	//U32 recordZobristWhite[121];
	//U32 recordZobristBlack[121];

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
    bool LoadBoard(const string filename, int &offset);
	bool SaveKifu(const string filename, const string comment) const;

	bool IsGameOver();
	bool IsSennichite() const;
	bool IsCheckAfter(const int src, const int dst);
    inline bool GetTurn() const { return m_turn; }
	inline int GetStep() const { return m_step; }
	inline int Evaluate() const { return m_turn ? m_evaluate : -m_evaluate; };
	inline U64 GetZobristHash() const {
		return m_turn ? ((U64)m_blackHashcode << 32) | m_whiteHashcode : ((U64)m_whiteHashcode << 32) | m_blackHashcode;
	}
	// 從recordZobrist取得之前的zobrist 需要先改變m_turn和m_step
	inline void UndoZobristHash() {
		m_whiteHashcode = m_turn ? recordZobrist[m_step] >> 32 : recordZobrist[m_step];
		m_blackHashcode = m_turn ? recordZobrist[m_step] : recordZobrist[m_step] >> 32;
		//m_whiteHashcode = recordZobristWhite[m_step];
		//m_blackHashcode = recordZobristBlack[m_step];
	}
	unsigned int GetKifuHash() const ;
};

#endif