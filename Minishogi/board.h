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
    U32 checking;
    U32 nonBlockable;

public:
    U32 occupied[2];
    U32 bitboard[32];
    int board[TOTAL_BOARD_SIZE];
    vector<Action> record;
    vector<U32> checkstate;

    Board();
    ~Board();
    void Initialize();
    void Initialize(const char *str);
    void DoMove(Action m_Action);
    void UndoMove();
    bool IsGameOver();
    int Evaluate(); //之後改成類似zobrist的方式
    void PrintChessBoard() const;
	void PrintNoncolorBoard(ostream &os) const;
	//void CalEvaluate();
	//void CalZobristNumber();
	//void CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess);
    bool SaveBoard(const string filename, const string comment) const;
    bool LoadBoard(const string filename, int &offset);
	bool SaveKifu(const string filename, const string comment) const;

	bool IsSennichite(Action action) const;
	bool IsStillChecking(const int src, const int dst);
    inline bool IsChecking() const { return checking; }
    inline bool IsnonBlockable() const { return nonBlockable; }
    inline bool GetTurn() const { return m_turn; }
	inline int GetEvaluate(bool turn) const {return turn ? -m_evaluate : m_evaluate; }
    inline U32 GetHashcode(bool turn) const { return turn ? m_blackHashcode : m_whiteHashcode; }
};

ostream & operator<<(ostream &os, const Board &board);

#endif 