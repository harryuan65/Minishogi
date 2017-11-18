#include "head.h"
#include<Windows.h>


#define hConsole GetStdHandle(STD_OUTPUT_HANDLE)
void SetColor(int color = 7)
{
	SetConsoleTextAttribute(hConsole, color);
}

Board::Board() {
	memset(bitboard, BLANK, 32 * sizeof(int));

}
Board::~Board() {

}
bool Board::Initialize()
{
	occupied[WHITE] = WHITE_INIT;
	occupied[BLACK] = BLACK_INIT;

	bitboard[PAWN] = W_PAWN_INIT;//0
	bitboard[SILVER] = W_SILVER_INIT;//1
	bitboard[GOLD] = W_GOLD_INIT;//4
	bitboard[BISHOP] = W_BISHOP_INIT;//6
	bitboard[ROOK] = W_ROOK_INIT;//8
	bitboard[KING] = W_KING_INIT;//10
	bitboard[PAWN | UPGRADED] = 0;//12
	bitboard[SILVER | UPGRADED] = 0;//14
	bitboard[BISHOP | UPGRADED] = 0;//16
	bitboard[ROOK | UPGRADED] = 0;//18

	bitboard[PAWN | BLACKCHESS] = B_PAWN_INIT;//
	bitboard[SILVER | BLACKCHESS] = B_SILVER_INIT;//
	bitboard[GOLD | BLACKCHESS] = B_GOLD_INIT;//
	bitboard[BISHOP | BLACKCHESS] = B_BISHOP_INIT;//
	bitboard[ROOK | BLACKCHESS] = B_ROOK_INIT;//
	bitboard[KING | BLACKCHESS] = B_KING_INIT;//
	bitboard[PAWN | UPGRADED | BLACKCHESS] = 0;//
	bitboard[SILVER | UPGRADED | BLACKCHESS] = 0;//
	bitboard[BISHOP | UPGRADED | BLACKCHESS] = 0;//
	bitboard[ROOK | UPGRADED | BLACKCHESS] = 0;//
	bitboard[0] = EMPTY;
	bitboard[7] = EMPTY;
	bitboard[8] = EMPTY;
	bitboard[11] = EMPTY;
	bitboard[14] = EMPTY;
	bitboard[15]= EMPTY;
	bitboard[16] = EMPTY;
	bitboard[23] = EMPTY;
	bitboard[24] = EMPTY;
	bitboard[27] = EMPTY;
	bitboard[30] = EMPTY;
	bitboard[31] = EMPTY;

	memset(board, BLANK, BOARD_SIZE * sizeof(int));

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
			printf("%2s", "步");
			break;
		case SILVER: // white silver2
			SetColor(143);
			printf("%2s", "銀");
			break;
		case GOLD: // white gold3
			SetColor(143);
			printf("%2s", "金");
			break;
		case BISHOP: // white bishop4
			SetColor(143);
			printf("%2s", "角");
			break;
		case ROOK: // white rook5
			SetColor(143);
			printf("%2s", "飛");
			break;
		case KING: // white king6
			SetColor(143);//白色字
			printf("%2s", "玉");
			break;
		case PAWN | UPGRADED: // 9 white e_pawn
			SetColor(207);
			printf("%2s", "ㄈ");
			break;
		case SILVER | UPGRADED: // 10 white e_silver
			SetColor(207);
			printf("%2s", "全");
			break;
		case BISHOP | UPGRADED: // 12 white e_bishop
			SetColor(207);
			printf("%2s", "馬");
			break;
		case ROOK | UPGRADED: // 13 white e_rook
			SetColor(207);
			printf("%2s", "龍");
			break;
		case PAWN | BLACKCHESS: // 10 black pawn
			SetColor(128);
			printf("%2s", "步");
			break;
		case SILVER | BLACKCHESS: //11black silver
			SetColor(128);
			printf("%2s", "銀");
			break;
		case GOLD | BLACKCHESS: //12 black gold
			SetColor(128);
			printf("%2s", "金");
			break;
		case BISHOP | BLACKCHESS: //13 black bishop
			SetColor(128); //黑色
			printf("%2s", "角");
			break;
		case ROOK | BLACKCHESS: //14 black rook
			SetColor(128);
			printf("%2s", "飛");
			break;
		case KING | BLACKCHESS: //15 black king
			SetColor(128);
			printf("%2s", "王");
			break;
		case PAWN | UPGRADED | BLACKCHESS: //16 black e_pawn
			SetColor(192);
			printf("%2s", "ㄈ");
			break;
		case SILVER | UPGRADED | BLACKCHESS: //17 black e_silver
			SetColor(192);
			printf("%2s", "全");
			break;
		case BISHOP | UPGRADED | BLACKCHESS: //18 black e_bishop
			SetColor(192);
			printf("%2s", "馬");
			SetColor();
			break;
		case ROOK | UPGRADED | BLACKCHESS: //19 black e_rook
			SetColor(192);
			printf("%2s", "龍");
			break;
		default:
			SetColor();
			cout << "空";
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
	printf("%2s｜", " ");//*
	printf("%2d", 5);
	printf("｜");
	printf("%2d", 4);
	printf("｜");
	printf("%2d", 3);
	printf("｜");
	printf("%2d", 2);
	printf("｜");
	printf("%2d", 1);
	puts("｜");
	for (int i = 0; i < 5; i++)
	{
		puts("—｜—｜—｜—｜—｜—｜—\t ｜—｜—｜—｜—｜—｜");
		if (i == 5)
		{
			printf("%2s｜", " ");  //*
			printf("%2d", 5);//*
			printf("｜");
			printf("%2d", 4);
			printf("｜");
			printf("%2d", 3);
			printf("｜");
			printf("%2d", 2);
			printf("｜");
			printf("%2d", 1);//*
			SetColor();
			puts("｜");
			puts(" \n");
		}
		printf("%2s", rank_name[rank_count]);
		SetColor();
		for (int j = 0; j < 5; j++) //128 = 黑 ; 143 = 白  ;207 = 紅底白字  ; 192 = 紅底黑字
		{
			printf("｜");
			switch (board[board_count])
			{
			case BLANK:
				printf("%2s", " ");
				break;
			case PAWN: // white pawn1
				SetColor(143);
				printf("%2s", "步");
				break;
			case SILVER: // white silver2
				SetColor(143);
				printf("%2s", "銀");
				break;
			case GOLD: // white gold3
				SetColor(143);
				printf("%2s", "金");
				break;
			case BISHOP: // white bishop4
				SetColor(143);
				printf("%2s", "角");
				break;
			case ROOK: // white rook5
				SetColor(143);
				printf("%2s", "飛");
				break;
			case KING: // white king6
				SetColor(143);//白色字
				printf("%2s", "玉");
				break;
			case PAWN | UPGRADED: // 9 white e_pawn
				SetColor(207);
				printf("%2s", "ㄈ");
				break;
			case SILVER | UPGRADED: // 10 white e_silver
				SetColor(207);
				printf("%2s", "全");
				break;
			case BISHOP | UPGRADED: // 12 white e_bishop
				SetColor(207);
				printf("%2s", "馬");
				break;
			case ROOK | UPGRADED: // 13 white e_rook
				SetColor(207);
				printf("%2s", "龍");
				break;
				//黑色棋
			case PAWN | BLACKCHESS: // 10 black pawn
				SetColor(128);
				printf("%2s", "步");
				break;
			case SILVER | BLACKCHESS: //11black silver
				SetColor(128);
				printf("%2s", "銀");
				break;
			case GOLD | BLACKCHESS: //12 black gold
				SetColor(128);
				printf("%2s", "金");
				break;
			case BISHOP | BLACKCHESS: //13 black bishop
				SetColor(128); //黑色
				printf("%2s", "角");
				break;
			case ROOK | BLACKCHESS: //14 black rook
				SetColor(128);
				printf("%2s", "飛");
				break;
			case KING | BLACKCHESS: //15 black king
				SetColor(128);
				printf("%2s", "王");
				break;
			case PAWN | UPGRADED | BLACKCHESS: //16 black e_pawn
				SetColor(192);
				printf("%2s", "ㄈ");
				break;
			case SILVER | UPGRADED | BLACKCHESS: //17 black e_silver
				SetColor(192);
				printf("%2s", "全");
				break;
			case BISHOP | UPGRADED | BLACKCHESS: //18 black e_bishop
				SetColor(192);
				printf("%2s", "馬");
				SetColor();
				break;
			case ROOK | UPGRADED | BLACKCHESS: //19 black e_rook
				SetColor(192);
				printf("%2s", "龍");
				break;
			default:
				break;
			}
			SetColor();
			board_count++;
		}
		printf("｜");
		SetColor(15);
		printf("%2s", rank_name[rank_count++]);//*
		SetColor();
		printf("\t ");
		int temp = board_count - 1;
		for (int j = temp - 4; j <= temp; j++)
		{
			printf("｜");
			SetColor(95);
			printf("%2d", j);
			SetColor();
		}
		printf("｜");
		puts(" ");		//*
	}
	puts(" ");			//*

}
bool Board::isGameOver() {
	return false;
}


//外部函式
int ConvertInput(std::string position) {
	/*cout << position << " = " <<
	(int)(5 * (position[0] - 'A'))<<"+"<< (int)('5' - position[1]) <<
	" = "<< (int)(5 * (position[0] - 'A') + ('5' - position[1] )) << endl;*/
	return (int)(5 * (position[0] - 'A') + ('5' - position[1]));
}
bool Human_DoMove(Board &currentboard, std::string from, std::string to, int pro, int isWhiteturn)
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
		U32 dstboard;
		bool dstHasChess = currentboard.board[to_pos] != 0;
		if (isWhiteturn)//若是輪到白就只找白色的
		{
			switch (currentboard.board[from_pos])//src位置的棋是什麼
			{
			case PAWN: 
				cout << "現在步兵要動了" << endl;
				dstboard = DstBoard(currentboard, PAWN,from_pos , dstHasChess);
				cout << "dstboard = " << dstboard << endl;
				break;
			case SILVER: // white silver
				cout << "銀" << endl;
				break;
			case GOLD: // white gold
				cout << "金" << endl;
				break;
			case BISHOP: // white bishop
				cout << "角" << endl;
				break;
			case ROOK: // white rook
				cout << "飛" << endl;
				break;
			case KING: // white king
					   //白色字 現在改黑了
				cout << "玉" << endl;
				break;
			case PAWN | UPGRADED: // 9 white e_pawn
				cout << "ㄈ" << endl;
				break;
			case SILVER | UPGRADED: // 10 white e_silver
				cout << "全" << endl;
				break;
			case BISHOP | UPGRADED: // 12 white e_bishop
				cout << "馬" << endl;
				break;
			case ROOK | UPGRADED: // 13 white e_rook
				cout << "龍" << endl;
				break;
			default:
				cout << "Invalid Move:Not Your Turn" << endl;
				break;
			}
		}
		else
		{
			switch (currentboard.board[from_pos])
			{
			case PAWN | BLACKCHESS: // 17 black pawn
				printf("%2s", "步");
				break;
			case SILVER | BLACKCHESS: //18 black silver
				printf("%2s", "銀");
				break;
			case GOLD | BLACKCHESS: //19 black gold
				printf("%2s", "金");
				break;
			case BISHOP | BLACKCHESS: //20 black bishop
				printf("%2s", "角");
				break;
			case ROOK | BLACKCHESS: //21 black rook
				printf("%2s", "飛");
				break;
			case KING | BLACKCHESS: //22 black king
				printf("%2s", "王");
				break;
			case PAWN | UPGRADED | BLACKCHESS: //25 black e_pawn
				printf("%2s", "ㄈ");
				break;
			case SILVER | UPGRADED | BLACKCHESS: //26 black e_silver
				printf("%2s", "全");
				break;
			case BISHOP | UPGRADED | BLACKCHESS: //28 black e_bishop
				printf("%2s", "馬");
				break;
			case ROOK | UPGRADED | BLACKCHESS: //29 black e_rook
				printf("%2s", "龍");
				break;
			default:
				cout << "Invalid Move:Not Your Turn" << endl;
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


//Rules
inline U32 DstBoard(Board board,int chessnumber, int position,bool isAttack) {
	U32 TargetBoard;
	bool turn = chessnumber - 16 > 0 ? BLACK : WHITE;
	if (isAttack)
		if (turn == BLACK)//黑棋要攻擊的話
			TargetBoard = board.occupied[WHITE];
		else
			TargetBoard = board.occupied[BLACK];
	else
		TargetBoard = blank_board;

	switch (chessnumber)
	{
	case BISHOP:
		return BishopMove(board, position, turn)|TargetBoard;
		break;
	case ROOK:
		return RookMove(board, position, turn) | TargetBoard;
		break;
	case PAWN | UPGRADED:
		return Movement[GOLD][position] | TargetBoard;
		break;
	case SILVER | UPGRADED:
		return Movement[GOLD][position] | TargetBoard;
		break;
	case BISHOP | UPGRADED:
		return BishopMove(board ,position,turn) | Movement[KING][position] |TargetBoard;
		break;
	case ROOK | UPGRADED:
		return RookMove(board, position, turn) | Movement[KING][position] | TargetBoard;
		break;
	case PAWN | UPGRADED | BLACKCHESS:
		return Movement[GOLD | BLACKCHESS][position] | TargetBoard;
		break;
	case SILVER | UPGRADED | BLACKCHESS:
		return Movement[GOLD | BLACKCHESS][position] | TargetBoard;
		break;
	case BISHOP | UPGRADED | BLACKCHESS:
		return BishopMove(board, position, turn) | Movement[KING][position] | TargetBoard;
		break;
	case ROOK | UPGRADED | BLACKCHESS:
		return RookMove(board, position, turn) | Movement[KING][position] | TargetBoard;
		break;
	default:
		/*
		PAWN   bPAWN
		SILVER bSILVER
		GOLD   bGOLD
		*/
		if (board.bitboard[chessnumber] == EMPTY)
		{
			std::cout << "Invalid Move:chess# "<<chessnumber<<"not exist\n";
			return EMPTY;
		}
		return Movement[chessnumber][position] | TargetBoard;
		break;
	}
}
U32 RookMove(Board board, int pos, int turn)
{
	// upper (find LSB) ; lower (find MSB)
	U32 ret = 0;
	U32 occupied = board.occupied[WHITE] | board.occupied[BLACK];
	U32 rank, file;
	U32 upper, lower;
	int bitpos;

	// rank
	upper = (occupied & row_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & row_lower[pos]) | LOWEST_BOARD_POS;

	bitpos = BitScan(&upper);
	upper = 1 << (bitpos + 1);

	bitpos = BitScanRev(lower);
	lower = 1 << bitpos;

	rank = (upper - lower) & rank_mask(pos);

	// file
	upper = (occupied & column_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & column_lower[pos]) | LOWEST_BOARD_POS;

	bitpos = BitScan(&upper);
	upper = 1 << (bitpos + 1);

	bitpos = BitScanRev(lower);
	lower = 1 << bitpos;

	file = (upper - lower) & file_mask(pos);

	ret = rank | file;
	ret &= (turn == WHITE) ? ret ^ board.occupied[WHITE] \
		: ret ^ board.occupied[BLACK];

	return ret;
}
U32 BishopMove(Board board, int pos, int turn)
{
	// upper (find LSB) ; lower (find MSB)
	U32 ret = 0;
	U32 occupied = board.occupied[WHITE] | board.occupied[BLACK];
	U32 slope1, slope2;
	U32 upper, lower;
	int bitpos;

	// slope1 "/"
	upper = (occupied & slope1_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & slope1_lower[pos]) | LOWEST_BOARD_POS;

	bitpos = BitScan(&upper);
	upper = 1 << (bitpos + 1);

	bitpos = BitScanRev(lower);
	lower = 1 << bitpos;

	slope1 = (upper - lower) & slope1_mask(pos);

	// slope2 "\"
	upper = (occupied & slope2_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & slope2_lower[pos]) | LOWEST_BOARD_POS;

	bitpos = BitScan(&upper);
	upper = 1 << (bitpos + 1);

	bitpos = BitScanRev(lower);
	lower = 1 << bitpos;

	slope2 = (upper - lower) & slope2_mask(pos);

	ret = slope1 | slope2;
	ret = (turn == WHITE) ? ret & (ret ^ board.occupied[WHITE]) \
		: ret & (ret ^ board.occupied[BLACK]);
	return ret;
}


//Generator ;Search
int Negascout() {};
bool MobeGenerator() {};
int QuietscenceSearch() {};

//Rules
bool Uchifuzume() {};
bool Sennichite() {};
