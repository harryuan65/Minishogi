#ifndef _BOARD_
#define _BOARD_
#include "head.h"

//不要問L是做什麼 你會怕
#define BOARD_PATH   "board//"
#define LBOARD_PATH L"board//"
#define KIFU_PATH    "output//"
#define LKIFU_PATH  L"output//"

class Board {
private:
    static vector<U32> ZOBRIST_TABLE[TOTAL_BOARD_SIZE];
    static const int ZOBRIST_SEED = 10;

    bool m_turn;
	int m_evaluate;
    U32 m_whiteHashcode;
    U32 m_blackHashcode;

public:
    U32 occupied[2];
    U32 bitboard[32];
    int board[TOTAL_BOARD_SIZE];
    vector<Action> record;

    Board();
    ~Board();
    void Initialize();
    void Initialize(const char *str);
    void PrintChessBoard() const;
	void PrintNoncolorBoard(ostream &os) const;
	void CalZobristNumber();
	//void CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess);

	void DoMove(Action m_Action);
	void UndoMove();

    bool SaveBoard(const string filename, const string comment) const;
    bool LoadBoard(const string filename, int &offset);
	bool SaveKifu(const string filename, const string comment) const;

	bool IsGameOver();
	bool IsSennichite(Action action) const;
	bool IsCheckAfter(const int src, const int dst);
    inline bool GetTurn() const { return m_turn; }
	inline int Evaluate() const { return m_turn ? m_evaluate : -m_evaluate; };
	inline U64 GetZobristHash() const {
		return m_turn ? (m_blackHashcode << 32) | m_whiteHashcode : (m_whiteHashcode << 32) | m_blackHashcode;
	}
	unsigned int GetKifuHash();
};

#endif 