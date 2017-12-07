#pragma once
#ifndef _BOARD_
#define _BOARD_
#include"head.h"

class Board {
private:
    U32 turn;
    U32 checking;
    U32 nonBlockable;

    static vector<U32> ZOBRIST_TABLE[TOTAL_BOARD_SIZE];
    static const int ZOBRIST_SEED = 10;
    U32 m_whiteHashcode;
    U32 m_blackHashcode;

    void CalZobristNumber();
    void CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess);

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
    int Evaluate();
    void PrintChessBoard();
    bool SavePlaybook();//棋譜，回傳成功
    bool SaveBoard(const char* filename);
    bool LoadBoard(const char* filename);

	bool IsSennichite(Action action);
	bool IsStillChecking(const int src, const int dst);
    inline bool IsChecking() { return checking; }
    inline bool IsChecking() const { return checking; }
    inline bool IsnonBlockable() { return nonBlockable; }
    inline bool IsnonBlockable() const { return nonBlockable; }
    inline U32 GetTurn() { return turn; }
    inline U32 GetTurn() const { return turn; }
    inline U32 GetHashcode(bool turn) { return turn ? m_blackHashcode : m_whiteHashcode; }
};

Action Human_DoMove(Board &board);
Action AI_DoMove(Board &board);

//Rules
inline U32 Movable(const Board &board, const int srcIndex);
U32 RookMove(const Board &board, const int pos);
U32 BishopMove(const Board &board, const int pos);

void AttackGenerator(Board &board, Action *movelist, U32 &start);
void MoveGenerator(Board &board, Action *movelist, U32 &start);
void HandGenerator(Board &board, Action *movelist, U32 &start);

/*
hand[] ->move gene, move, Hash 
*/

#endif 