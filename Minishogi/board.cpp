#include"head.h"
#include<Windows.h>


#define hConsole GetStdHandle(STD_OUTPUT_HANDLE)
void SetColor(int color = 7)
{
	SetConsoleTextAttribute(hConsole, color);
}

Board::Board() {
	chesspiece[32] = {BLANK};
}
Board::~Board() {

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
	chesspiece[PAWN | UPGRADED] = 0;//12
	chesspiece[SILVER | UPGRADED] = 0;//14
	chesspiece[BISHOP | UPGRADED] = 0;//16
	chesspiece[ROOK | UPGRADED] = 0;//18

	chesspiece[PAWN | BLACKCHESS] = B_PAWN_INIT;//
	chesspiece[SILVER | BLACKCHESS] = B_SILVER_INIT;//
	chesspiece[GOLD | BLACKCHESS] = B_GOLD_INIT;//
	chesspiece[BISHOP | BLACKCHESS] = B_BISHOP_INIT;//
	chesspiece[ROOK | BLACKCHESS] = B_ROOK_INIT;//
	chesspiece[KING | BLACKCHESS] = B_KING_INIT;//
	chesspiece[PAWN | UPGRADED | BLACKCHESS] = 0;//
	chesspiece[SILVER | UPGRADED | BLACKCHESS] = 0;//
	chesspiece[BISHOP | UPGRADED | BLACKCHESS] = 0;//
	chesspiece[ROOK | UPGRADED | BLACKCHESS] = 0;//

	memset(board, BLANK, CHESS_BOARD_SIZE * sizeof(int));

	board[A5] = ROOK | BLACKCHESS;//20
	board[A4] = BISHOP | BLACKCHESS; //19
	board[A3] = SILVER | BLACKCHESS;//17
	board[A2] = GOLD | BLACKCHESS;//18
	board[A1] = KING | BLACKCHESS;//21
	board[B1] = PAWN |BLACKCHESS ;//16

	board[E1] = ROOK;//4
	board[E2] = BISHOP;//3
	board[E3] = SILVER;//1
	board[E4] = GOLD;//2
	board[E5] = KING;//5
	board[D5] = PAWN;//0

	return true;
}
bool Board::Initialize(string board_str){
	stringstream ss;
	string token;
	ss << board_str;
	int i = 0;
	
	while(getline(ss, token, ' ')&&i<25)
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
		case PAWN | UPGRADED: // 9 white e_pawn
			SetColor(207);
			printf("%2s", "�w");
			break;
		case SILVER | UPGRADED: // 10 white e_silver
			SetColor(207);
			printf("%2s", "��");
			break;
		case BISHOP | UPGRADED: // 12 white e_bishop
			SetColor(207);
			printf("%2s", "��");
			break;
		case ROOK | UPGRADED: // 13 white e_rook
			SetColor(207);
			printf("%2s", "�s");
			break;
		case PAWN | BLACKCHESS: // 10 black pawn
			SetColor(128);
			printf("%2s", "�B");
			break;
		case SILVER | BLACKCHESS: //11black silver
			SetColor(128);
			printf("%2s", "��");
			break;
		case GOLD | BLACKCHESS: //12 black gold
			SetColor(128);
			printf("%2s", "��");
			break;
		case BISHOP | BLACKCHESS: //13 black bishop
			SetColor(128); //�¦�
			printf("%2s", "��");
			break;
		case ROOK | BLACKCHESS: //14 black rook
			SetColor(128);
			printf("%2s", "��");
			break;
		case KING | BLACKCHESS: //15 black king
			SetColor(128);
			printf("%2s", "��");
			break;
		case PAWN | UPGRADED | BLACKCHESS: //16 black e_pawn
			SetColor(192);
			printf("%2s", "�w");
			break;
		case SILVER | UPGRADED | BLACKCHESS: //17 black e_silver
			SetColor(192);
			printf("%2s", "��");
			break;
		case BISHOP | UPGRADED | BLACKCHESS: //18 black e_bishop
			SetColor(192);
			printf("%2s", "��");
			SetColor();
			break;
		case ROOK | UPGRADED | BLACKCHESS: //19 black e_rook
			SetColor(192);
			printf("%2s", "�s");
			break;
		default:
			SetColor();
			cout << "��";
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
	puts(" ");//*
	printf("%2s�U", " ");//*
	printf("%2d", 5);
	printf("�U");
	printf("%2d", 4);
	printf("�U");
	printf("%2d", 3);
	printf("�U");
	printf("%2d", 2);
	printf("�U");
	printf("%2d", 1);
	puts("�U");
	for (int i = 0; i < 9; i++)
	{
		puts("�X�U�X�U�X�U�X�U�X�U�X�U�X\t �U�X�U�X�U�X�U�X�U�X�U");
		if (i == 5)
		{
			printf("%2s�U", " ");  //*
			printf("%2d", 5);//*
			printf("�U");
			printf("%2d", 4);
			printf("�U");
			printf("%2d", 3);
			printf("�U");
			printf("%2d", 2);
			printf("�U");
			printf("%2d", 1);//*
			SetColor();
			puts("�U");
			puts(" \n");
		}
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
			case PAWN | UPGRADED: // 9 white e_pawn
				SetColor(207);
				printf("%2s", "�w");
				break;
			case SILVER | UPGRADED: // 10 white e_silver
				SetColor(207);
				printf("%2s", "��");
				break;
			case BISHOP | UPGRADED: // 12 white e_bishop
				SetColor(207);
				printf("%2s", "��");
				break;
			case ROOK | UPGRADED: // 13 white e_rook
				SetColor(207);
				printf("%2s", "�s");
				break;
				//�¦��
			case PAWN | BLACKCHESS: // 10 black pawn
				SetColor(128);
				printf("%2s", "�B");
				break;
			case SILVER | BLACKCHESS: //11black silver
				SetColor(128);
				printf("%2s", "��");
				break;
			case GOLD | BLACKCHESS: //12 black gold
				SetColor(128);
				printf("%2s", "��");
				break;
			case BISHOP | BLACKCHESS: //13 black bishop
				SetColor(128); //�¦�
				printf("%2s", "��");
				break;
			case ROOK | BLACKCHESS: //14 black rook
				SetColor(128);
				printf("%2s", "��");
				break;
			case KING | BLACKCHESS: //15 black king
				SetColor(128);
				printf("%2s", "��");
				break;
			case PAWN | UPGRADED | BLACKCHESS: //16 black e_pawn
				SetColor(192);
				printf("%2s", "�w");
				break;
			case SILVER | UPGRADED | BLACKCHESS: //17 black e_silver
				SetColor(192);
				printf("%2s", "��");
				break;
			case BISHOP | UPGRADED | BLACKCHESS: //18 black e_bishop
				SetColor(192);
				printf("%2s", "��");
				SetColor();
				break;
			case ROOK | UPGRADED | BLACKCHESS: //19 black e_rook
				SetColor(192);
				printf("%2s", "�s");
				break;
			default:
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
			case PAWN | UPGRADED: // 9 white e_pawn
				cout << "�w" << endl;
				break;
			case SILVER | UPGRADED: // 10 white e_silver
				cout << "��" << endl;
				break;
			case BISHOP | UPGRADED: // 12 white e_bishop
				cout << "��" << endl;
				break;
			case ROOK | UPGRADED: // 13 white e_rook
				cout << "�s" << endl;
				break;
			}
		}
		else
		{
			switch (chessboard[from_pos])
			{
			case PAWN | BLACKCHESS: // 10 black pawn
				printf("%2s", "�B");
				break;
			case SILVER | BLACKCHESS: //11black silver
				printf("%2s", "��");
				break;
			case GOLD | BLACKCHESS: //12 black gold
				printf("%2s", "��");
				break;
			case BISHOP | BLACKCHESS: //13 black bishop
				printf("%2s", "��");
				break;
			case ROOK | BLACKCHESS: //14 black rook
				printf("%2s", "��");
				break;
			case KING | BLACKCHESS: //15 black king
				printf("%2s", "��");
				break;
			case PAWN | UPGRADED | BLACKCHESS: //16 black e_pawn
				printf("%2s", "�w");
				break;
			case SILVER | UPGRADED | BLACKCHESS: //17 black e_silver
				printf("%2s", "��");
				break;
			case BISHOP | UPGRADED | BLACKCHESS: //18 black e_bishop
				printf("%2s", "��");
				break;
			case ROOK | UPGRADED | BLACKCHESS: //19 black e_rook
				printf("%2s", "�s");
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
int Negascout() {};
bool MobeGenerator() {};
int QuietscenceSearch() {};

//Rules
bool Uchifuzume() {};
bool Sennichite() {};
