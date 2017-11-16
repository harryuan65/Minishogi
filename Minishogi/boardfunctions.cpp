#include"head.h"
#include<Windows.h>

//���U�Ϊ�
#define hConsole GetStdHandle(STD_OUTPUT_HANDLE)
void SetColor(int color = 7)
{
	SetConsoleTextAttribute(hConsole,color);
}


void Initalize(playerboard &board, int *chessboard)
{
	board.w_occupied = WHITE_INIT;
	board.b_occupied = BLACK_INIT;

	board.chesspiece[w_Pawn] = W_PAWN_INIT;
	board.chesspiece[w_Silver] = W_SILVER_INIT;
	board.chesspiece[w_Gold] = W_GOLD_INIT;
	board.chesspiece[w_Bishop] = W_BISHOP_INIT;
	board.chesspiece[w_Rook] = W_ROOK_INIT;
	board.chesspiece[w_King] = W_KING_INIT;
	board.chesspiece[w_e_Pawn] = 0;
	board.chesspiece[w_e_Silver] = 0;
	board.chesspiece[w_e_Bishop] = 0;
	board.chesspiece[w_e_Rook] = 0;

	board.chesspiece[b_Pawn] = B_PAWN_INIT;
	board.chesspiece[b_Silver] = B_SILVER_INIT;
	board.chesspiece[b_Gold] = B_GOLD_INIT;
	board.chesspiece[b_Bishop] = B_BISHOP_INIT;
	board.chesspiece[b_Rook] = B_ROOK_INIT;
	board.chesspiece[b_King] = B_KING_INIT;
	board.chesspiece[b_e_Pawn] = 0;
	board.chesspiece[b_e_Silver] = 0;
	board.chesspiece[b_e_Bishop] = 0;
	board.chesspiece[b_e_Rook] = 0;

	memset(chessboard, BLANK, CHESS_BOARD_SIZE * sizeof(int));

	chessboard[A5] = ROOK | COLOR_BOUND;
	chessboard[A4] = BISHOP | COLOR_BOUND;
	chessboard[A3] = SILVER | COLOR_BOUND;
	chessboard[A2] = GOLD | COLOR_BOUND;
	chessboard[A1] = KING | COLOR_BOUND;
	chessboard[B1] = PAWN | COLOR_BOUND;

	chessboard[E1] = ROOK;
	chessboard[E2] = BISHOP;
	chessboard[E3] = SILVER;
	chessboard[E4] = GOLD;
	chessboard[E5] = KING;
	chessboard[D5] = PAWN;

	return;
}
void PrintChessBoard(int *chessboard)
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
			switch (chessboard[board_count])
			{
			case BLANK:
				printf("%2s", " ");
				break;
			case PAWN: // white pawn
					SetColor(143);
				printf("%2s", "�B");
				break;
			case SILVER: // white silver
					SetColor(143);
				printf("%2s", "��");
				break;
			case GOLD: // white gold
					SetColor(143);
				printf("%2s", "��");
				break;
			case BISHOP: // white bishop
					SetColor(143);
				printf("%2s", "��");
				break;
			case ROOK: // white rook
					SetColor(143);
				printf("%2s", "��");
				break;
			case KING: // white king
					SetColor(143);//�զ�r �{�b��¤F
				printf("%2s", "��");
				break;
			case ePAWN: // 9 white e_pawn
					SetColor(207);
				printf("%2s", "�w");
				break;
			case eSILVER: // 10 white e_silver
					 SetColor(207);
				printf("%2s", "��");
				break;
			case eBISHOP: // 12 white e_bishop
					 SetColor(207);
				printf("%2s", "��");
				break;
			case eROOK: // 13 white e_rook
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
			case bePAWN: //25 black e_pawn
					 SetColor(192);
				printf("%2s", "�w");
				break;
			case beSILVER: //26 black e_silver
					 SetColor(192);
				printf("%2s", "��");
				break;
			case beBISHOP: //28 black e_bishop
					 SetColor(192);
				printf("%2s", "��");
				SetColor();
				break;
			case beROOK: //29 black e_rook
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


