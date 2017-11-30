#ifndef _BOARD_
#define _BOARD_
#include "head.h"

class Board {
private:
	static vector<U32> ZOBRIST_TABLE[37];
	static const int ZOBRIST_SEED = 10;
	U32 m_whiteHashcode;
	U32 m_blackHashcode;

	void CalZobristNumber();
	void CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess);

public:
    U32 occupied[2];
    U32 bitboard[32];
    int board[37];
    vector<Action> record;

    Board();
    ~Board();
    bool Initialize();
    bool Initialize(string &board_str);
    void DoMove(Action m_Action);
    void UndoMove();
	void PrintChessBoard(bool turn);
    bool IsGameOver();
	int GetEvaluate(bool turn);
	bool SaveResult();
    bool SavePlaybook(); //���СA�^�Ǧ��\
    bool SaveBoard(string filename);
    bool LoadBoard(string filename);
	inline U32 GetHashcode(bool turn) { return turn ? m_blackHashcode : m_whiteHashcode; }
};

Action Human_DoMove(Board &board, int turn);
Action AI_DoMove(Board &board, int turn);

/* Move Rules */
inline U32 Movable(const Board &board, const int srcIndex);
U32 RookMove(const Board &board, const int pos);
U32 BishopMove(const Board &board, const int pos);

bool Uchifuzume();
bool Sennichite();

/* Move Gene */
void MoveGenerator(const Board &board, const int turn, Action *movelist, int &start);
void HandGenerator(const Board &board, const int turn, Action *movelist, int &start);

/*
hand[] ->move gene, move, Hash 
*/

#endif 