#include"head.h"
#ifndef _BOARD_
#define _BOARD_

class  Board {
public:
	U32 occupied[2];
	U32 bitboard[32];
	int board[35];
	int hand[10];//手牌數量
	vector<Action> record;

	Board();
	~Board();
	bool Initialize();
	bool Initialize(std::string &board_str/*Board &board, int *chessboard*/);
	void DoMove(Action m_Action);
	void UndoMove();
	void PrintChessBoard(/*裡面用board來印*/);
	bool isGameOver();
	unsigned int Hashing();//同型表的東西
	int Evaluate();//評價函數
	bool SavePlaybook();//棋譜，回傳成功
	bool SaveBoard(std::string filename);
	bool LoadBoard(std::string filename);
};


Action Human_DoMove(Board &board, int turn);
Action AI_DoMove(Board &board, int turn);

//Rules
inline U32 Movable(const Board &board, const int srcIndex);
U32 RookMove(const Board &board, const int pos);
U32 BishopMove(const Board &board, const int pos);

//Generator
int Negascout();
int QuietscenceSearch();

bool Uchifuzume();
bool Sennichite();

void MoveGenerator(const Board &board, Action *movelist, int &start, const int turn);

/*
hand[] ->move gene, move, Hash 
*/

#endif 