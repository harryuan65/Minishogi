#include"head.h"
#include<Windows.h>
using namespace std;

#define hConsole GetStdHandle(STD_OUTPUT_HANDLE)
void SetColor(int color = 7)
{
	SetConsoleTextAttribute(hConsole, color);
}
bool Board::Initialize()
{
	occupied[WHITE] = WHITE_INIT;
	occupied[BLACK] = BLACK_INIT;

	chesspiece[PAWN] = W_PAWN_INIT;//0
	chesspiece[SILVER] = W_SILVER_INIT;//1
	chesspiece[GOLD] = W_GOLD_INIT;//4
	chesspiece[BISHOP] = W_BISHOP_INIT;//6
	chesspiece[ROOK] = W_ROOK_INIT;//8
	chesspiece[KING] = W_KING_INIT;//10
	chesspiece[E_PAWN] = 0;//12
	chesspiece[E_SILVER] = 0;//14
	chesspiece[E_BISHOP] = 0;//16
	chesspiece[E_ROOK] = 0;//18

	chesspiece[PAWN | BLACKCHESS] = B_PAWN_INIT;//1
	chesspiece[SILVER | BLACKCHESS] = B_SILVER_INIT;//3
	chesspiece[GOLD | BLACKCHESS] = B_GOLD_INIT;//5
	chesspiece[BISHOP | BLACKCHESS] = B_BISHOP_INIT;//7
	chesspiece[ROOK | BLACKCHESS] = B_ROOK_INIT;//9
	chesspiece[KING | BLACKCHESS] = B_KING_INIT;//11
	chesspiece[E_PAWN | BLACKCHESS] = 0;//13
	chesspiece[E_SILVER | BLACKCHESS] = 0;//15
	chesspiece[E_BISHOP | BLACKCHESS] = 0;//17
	chesspiece[E_ROOK | BLACKCHESS] = 0;//19

							 //memset(board, BLANK, CHESS_BOARD_SIZE * sizeof(int));

	board[A5] = ROOK | BLACKCHESS;//5+16 ->4+10
	board[A4] = BISHOP | BLACKCHESS; //4+16 ->3+10
	board[A3] = SILVER | BLACKCHESS;//2+16 ->1+10
	board[A2] = GOLD | BLACKCHESS;//3+16 ->2+10
	board[A1] = KING | BLACKCHESS;//6+16 ->5+10
	board[B1] = PAWN |BLACKCHESS ;//0+16->0+10

	board[E1] = ROOK;//5
	board[E2] = BISHOP;//4
	board[E3] = SILVER;//2
	board[E4] = GOLD;//3
	board[E5] = KING;//6
	board[D5] = PAWN;//0

	return true;
}
bool Board::Initialize(string board_str){
	stringstream ss;
	string token;
	ss << board_str;
	int i = 0;
	
	while(getline(ss, token, ' '))
	{
		int chesstype = atoi(token.c_str());
		board[i] = chesstype;
		switch (board[i])
		{
		case BLANK:
			printf("%2s", " ");
			break;
		case PAWN: // white pawn1
			SetColor(143);
			printf("%2s", "�B");
			break;
		case SILVER: // white silver2
			SetColor(143);
			printf("%2s", "��");
			break;
		case GOLD: // white gold3
			SetColor(143);
			printf("%2s", "��");
			break;
		case BISHOP: // white bishop4
			SetColor(143);
			printf("%2s", "��");
			break;
		case ROOK: // white rook5
			SetColor(143);
			printf("%2s", "��");
			break;
		case KING: // white king6
			SetColor(143);//�զ�r
			printf("%2s", "��");
			break;
		case E_PAWN: // 9 white e_pawn
			SetColor(207);
			printf("%2s", "�w");
			break;
		case E_SILVER: // 10 white e_silver
			SetColor(207);
			printf("%2s", "��");
			break;
		case E_BISHOP: // 12 white e_bishop
			SetColor(207);
			printf("%2s", "��");
			break;
		case E_ROOK: // 13 white e_rook
			SetColor(207);
			printf("%2s", "�s");
			break;
		case PAWN | BLACKCHESS: // 17 black pawn
			SetColor(128);
			printf("%2s", "�B");
			break;
		case SILVER | BLACKCHESS: //18 black silver
			SetColor(128);
			printf("%2s", "��");
			break;
		case GOLD | BLACKCHESS: //19 black gold
			SetColor(128);
			printf("%2s", "��");
			break;
		case BISHOP | BLACKCHESS: //20 black bishop
			SetColor(128); //�¦�
			printf("%2s", "��");
			break;
		case ROOK | BLACKCHESS: //21 black rook
			SetColor(128);
			printf("%2s", "��");
			break;
		case KING | BLACKCHESS: //22 black king
			SetColor(128);
			printf("%2s", "��");
			break;
		case E_PAWN | BLACKCHESS: //25 black e_pawn
			SetColor(192);
			printf("%2s", "�w");
			break;
		case E_SILVER | BLACKCHESS: //26 black e_silver
			SetColor(192);
			printf("%2s", "��");
			break;
		case E_BISHOP | BLACKCHESS: //28 black e_bishop
			SetColor(192);
			printf("%2s", "��");
			SetColor();
			break;
		case E_ROOK | BLACKCHESS: //29 black e_rook
			SetColor(192);
			printf("%2s", "�s");
			break;
		default:
			break;
		}
		if ((i+1) %5==0)cout << endl;
		SetColor();
		//cout<<"atoi("<<token<<") = "<<atoi(token.c_str())<<" ";
		i++;
	}
	cout << endl;
	return true;
}
void Board::PrintChessBoard()
{
	char *rank_name[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I" };
	int rank_count = 0;
	int board_count = 0;
	SetColor();
	puts(" ");//*
	printf("%2s�U", " ");//*
	SetColor(15);
	printf("%2d", 5);
	SetColor();
	printf("�U");
	SetColor(15);
	printf("%2d", 4);
	SetColor();
	printf("�U");
	SetColor(15);
	printf("%2d", 3);
	SetColor();
	printf("�U");
	SetColor(15);
	printf("%2d", 2);
	SetColor();
	printf("�U");
	SetColor(15);
	printf("%2d", 1);
	SetColor();
	puts("�U");
	for (int i = 0; i < 9; i++)
	{
		puts("�X�U�X�U�X�U�X�U�X�U�X�U�X\t �U�X�U�X�U�X�U�X�U�X�U");
		if (i == 5)
		{
			printf("%2s�U", " ");  //*
			SetColor(15);
			printf("%2d", 5);//*
			SetColor();
			printf("�U");
			SetColor(15);
			printf("%2d", 4);
			SetColor();
			printf("�U");
			SetColor(15);
			printf("%2d", 3);
			SetColor();
			printf("�U");
			SetColor(15);
			printf("%2d", 2);
			SetColor();
			printf("�U");
			SetColor(15);
			printf("%2d", 1);//*
			SetColor();
			puts("�U");
			puts(" \n");
		}
		//SetColor(15);
		printf("%2s", rank_name[rank_count]);
		SetColor();
		for (int j = 0; j < 5; j++) //128 = �� ; 143 = ��  ;207 = �����զr  ; 192 = �����¦r
		{
			printf("�U");
			switch (board[board_count])
			{
			case BLANK:
				printf("%2s", " ");
				break;
			case PAWN: // white pawn1
				SetColor(143);
				printf("%2s", "�B");
				break;
			case SILVER: // white silver2
				SetColor(143);
				printf("%2s", "��");
				break;
			case GOLD: // white gold3
				SetColor(143);
				printf("%2s", "��");
				break;
			case BISHOP: // white bishop4
				SetColor(143);
				printf("%2s", "��");
				break;
			case ROOK: // white rook5
				SetColor(143);
				printf("%2s", "��");
				break;
			case KING: // white king6
				SetColor(143);//�զ�r
				printf("%2s", "��");
				break;
			case E_PAWN: // 9 white e_pawn
				SetColor(207);
				printf("%2s", "�w");
				break;
			case E_SILVER: // 10 white e_silver
				SetColor(207);
				printf("%2s", "��");
				break;
			case E_BISHOP: // 12 white e_bishop
				SetColor(207);
				printf("%2s", "��");
				break;
			case E_ROOK: // 13 white e_rook
				SetColor(207);
				printf("%2s", "�s");
				break;
			case bPAWN: // 17 black pawn
				SetColor(128);
				printf("%2s", "�B");
				break;
			case bSILVER: //18 black silver
				SetColor(128);
				printf("%2s", "��");
				break;
			case bGOLD: //19 black gold
				SetColor(128);
				printf("%2s", "��");
				break;
			case bBISHOP: //20 black bishop
				SetColor(128); //�¦�
				printf("%2s", "��");
				break;
			case bROOK: //21 black rook
				SetColor(128);
				printf("%2s", "��");
				break;
			case bKING: //22 black king
				SetColor(128);
				printf("%2s", "��");
				break;
			case b_E_PAWN: //25 black e_pawn
				SetColor(192);
				printf("%2s", "�w");
				break;
			case b_E_SILVER: //26 black e_silver
				SetColor(192);
				printf("%2s", "��");
				break;
			case b_E_BISHOP: //28 black e_bishop
				SetColor(192);
				printf("%2s", "��");
				SetColor();
				break;
			case b_E_ROOK: //29 black e_rook
				SetColor(192);
				printf("%2s", "�s");
				break;
			}
			SetColor();
			board_count++;
		}
		printf("�U");
		SetColor(15);
		printf("%2s", rank_name[rank_count++]);//*
		SetColor();
		printf("\t ");
		int temp = board_count - 1;
		for (int j = temp - 4; j <= temp; j++)
		{
			printf("�U");
			SetColor(95);
			printf("%2d", j);
			SetColor();
		}
		printf("�U");
		puts(" ");		//*
	}
	puts(" ");			//*

	return;
}
bool Board::isGameOver() {
	return false;
}



//�~���禡
int ConvertInput(std::string position) {
	/*cout << position << " = " <<
	(int)(5 * (position[0] - 'A'))<<"+"<< (int)('5' - position[1]) <<
	" = "<< (int)(5 * (position[0] - 'A') + ('5' - position[1] )) << endl;*/
	return (int)(5 * (position[0] - 'A') + ('5' - position[1]));
}
bool Human_DoMove(int *chessboard, Board &board, std::string from, std::string to, int pro, int isWhiteturn)
{
	if ((pro < 0 || pro>1) ||
		(from[0] < 'A'&&from[1] > 'F') && (from[1] >= '0'&&from[1] > '9') &&
		(to[0] < 'A'&&to[1] > 'F') && (to[1] < '0'&&to[1] > '9'))
	{
		std::cout << "Invalid Move:bad input\n";
		return false;
	}
	else
	{
		int from_pos = ConvertInput(from);
		int to_pos = ConvertInput(to);
		if (isWhiteturn)
		{
			switch (chessboard[from_pos])
			{
			case BLANK:
				cout << " " << endl;
				break;
			case PAWN: // white pawn
				cout << "�B" << endl;

				//cout<<"BitScan(wPawn) = "<<BitScan(&(board.chesspiece[w_Pawn]));
				//���OBitScan�|�^��15
				break;
			case SILVER: // white silver
				cout << "��" << endl;
				break;
			case GOLD: // white gold
				cout << "��" << endl;
				break;
			case BISHOP: // white bishop
				cout << "��" << endl;
				break;
			case ROOK: // white rook
				cout << "��" << endl;
				break;
			case KING: // white king
					   //�զ�r �{�b��¤F
				cout << "��" << endl;
				break;
			case E_PAWN: // 9 white e_pawn
				cout << "�w" << endl;
				break;
			case E_SILVER: // 10 white e_silver
				cout << "��" << endl;
				break;
			case E_BISHOP: // 12 white e_bishop
				cout << "��" << endl;
				break;
			case E_ROOK: // 13 white e_rook
				cout << "�s" << endl;
				break;
			}
		}
		else
		{
			switch (chessboard[from_pos])
			{
			case bPAWN: // 17 black pawn
				cout << "�B" << endl;
				break;
			case bSILVER: //18 black silver
				cout << "��" << endl;
				break;
			case bGOLD: //19 black gold
				cout << "��" << endl;
				break;
			case bBISHOP: //20 black bishop
				cout << "��" << endl;
				break;
			case bROOK: //21 black rook
				cout << "��" << endl;
				break;
			case bKING: //22 black king
				cout << "��" << endl;
				break;
			case b_E_PAWN: //25 black e_pawn
				cout << "�w" << endl;
				break;
			case b_E_SILVER: //26 black e_silver
				cout << "��" << endl;
				break;
			case b_E_BISHOP: //28 black e_bishop
				cout << "��" << endl;
				break;
			case b_E_ROOK: //29 black e_rook
				cout << "�s" << endl;
				break;
			default:
				break;
			}
			return false;
		}
	}
}
bool AI_DoMove(Board &board, int isWhiteturn)
{
	return true;
}


//Generator ;Search
int Negascout();
bool MobeGenerator();
int QuietscenceSearch();

//Rules
bool Uchifuzume();
bool Sennichite();

