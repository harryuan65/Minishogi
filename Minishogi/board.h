#include"head.h"
#ifndef _BOARD_
#define _BOARD_

class  Board {
public:
	U32 occupied[2];
	U32 bitboard[32];
	int board[25];
	int hand[10];//��P�ƶq
	int handcount;
//	vector<Action> record;
	Board();
	~Board();
	bool Initialize();
	bool Initialize(std::string board_str/*Board &board, int *chessboard*/);
	bool DoMove(Action m_Action);
	//bool UndoMove(Action m_Action);
	void PrintChessBoard(/*�̭���board�ӦL*/);
	bool isGameOver();
	//unsigned int Hashing();//�P�����F��
	//int Evaluate();//�������
	//bool SavePlaybook();//���СA�^�Ǧ��\
	//bool SaveBoard(std::string filename);
	//bool LoadBoard(std::string filename);
};


bool Human_DoMove(Board &currentboard, int isWhiteturn);
bool AI_DoMove(Board &board, int isWhiteturn);

//Rules
U32 DstBoard(Board board,int chessnumber,int position,bool isAttack);
U32 BishopMove(Board board, int pos, int turn);
U32 RookMove(Board board, int pos, int turn);

//Generator
int Negascout();
bool MobeGenerator();
int QuietscenceSearch();

bool Uchifuzume();
bool Sennichite();





#endif 