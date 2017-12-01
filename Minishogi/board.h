#ifndef _BOARD_
#define _BOARD_
#include "head.h"

class Board {
private:
	static vector<U32> ZOBRIST_TABLE[37];
	U64 m_whiteHashcode;
	U64 m_blackHashcode;

	void CalZobristNumber();
	void CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess);

public:
	static const int ZOBRIST_SEED = 15;
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
	void PrintChessBoard(bool turn) const;
    bool IsGameOver();
	int GetEvaluate(bool turn) const;
	bool SaveResult();
    bool SavePlaybook(); //棋譜，回傳成功
    bool SaveBoard(string filename);
    bool LoadBoard(string filename);
	bool IsSennichite(Action action);
	inline U64 GetHashcode(bool turn) const { 
		return turn ? (m_blackHashcode << 32) | m_whiteHashcode : (m_whiteHashcode << 32) | m_blackHashcode;
			//m_blackHashcode : m_whiteHashcode;
	}
	inline bool CheckChessCount() {
		int count = 0;
		for (int i = 0; i < 25; i++) {
			count += (bool)board[i];
		}
		for (int i = 25; i < 37; i++) {
			count += board[i];
		}
		if (count != 12) {
			return false;
		}
		return true;
	}
};

Action Human_DoMove(Board &board, int turn);
Action AI_DoMove(Board &board, int turn);

/* Move Rules */
inline U32 Movable(const Board &board, const int srcIndex);
U32 RookMove(const Board &board, const int pos);
U32 BishopMove(const Board &board, const int pos);

bool Uchifuzume();

/* Move Gene */
void MoveGenerator(const Board &board, const int turn, Action *movelist, int &start);
void HandGenerator(const Board &board, const int turn, Action *movelist, int &start);

/*
hand[] ->move gene, move, Hash 
*/

#endif 