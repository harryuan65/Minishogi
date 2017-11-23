#include"head.h"
#ifndef _BOARD_
#define _BOARD_

class  Board {
public:
	U32 occupied[2];
	U32 bitboard[32];
	int board[35];
	int hand[10];//��P�ƶq
	vector<Action> record;

	Board();
	~Board();
	bool Initialize();
	bool Initialize(std::string &board_str/*Board &board, int *chessboard*/);
	void DoMove(Action m_Action);
	void UndoMove();
	void PrintChessBoard(/*�̭���board�ӦL*/);
	bool isGameOver();
	unsigned int Hashing();//�P�����F��
	int Evaluate();//�������
	bool SavePlaybook();//���СA�^�Ǧ��\
	bool SaveBoard(std::string filename);
	bool LoadBoard(std::string filename);
};


Action Human_DoMove(Board &board, int turn);
Action AI_DoMove(Board &board, int turn);

//Rules
inline U32 DstBoard(Board board,int chessnumber,int from_pos,int to_pos);
U32 BishopMove(Board &board, int pos, int turn);
U32 RookMove(Board &board, int pos, int turn);

//Generator
int Negascout();
bool MobeGenerator();
int QuietscenceSearch();

bool Uchifuzume();
bool Sennichite();

/*
hand[] ->move gene, move, Hash 
*/

#endif 