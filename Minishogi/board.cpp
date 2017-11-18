#include "head.h"
#include "board.h"
#include<Windows.h>
U32 Movement[20][BOARD_SIZE] = {
	/* 0 */{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/* 1 PAWN */
	{
		0x0000000, 	0x0000000, 	0x0000000, 	0x0000000, 	0x0000000,
		0x0000001, 	0x0000002, 	0x0000004, 	0x0000008, 	0x0000010,
		0x0000020, 	0x0000040, 	0x0000080, 	0x0000100,	0x0000200,
		0x0000400, 	0x0000800, 	0x0001000, 	0x0002000,	0x0004000,
		0x0008000, 	0x0010000, 	0x0020000, 	0x0040000,	0x0080000
	},
	/* 2 SILVER*/
	{
		0x0000040, 	0x00000a0, 	0x0000140, 	0x0000280, 	0x0000100,
		0x0000803, 	0x0001407, 	0x000280e, 	0x000501c, 	0x0002018,
		0x0010060, 	0x00280e0, 	0x00501c0, 	0x00a0380,	0x0040300,
		0x0200c00, 	0x0501c00, 	0x0a03800, 	0x1407000,	0x0806000,
		0x0018000, 	0x0038000, 	0x0070000, 	0x00e0000,	0x00c0000
	}
	,
	/* 3 GOLD*/
	{
		0x0000022, 	0x0000045, 	0x000008a, 	0x0000114, 	0x0000208,
		0x0000443, 	0x00008a7, 	0x000114e, 	0x000229c, 	0x0004118,
		0x0008860, 	0x00114e0, 	0x00229c0, 	0x0045380,	0x0082300,
		0x0110c00, 	0x0229c00, 	0x0453800, 	0x08a7000,	0x1046000,
		0x0218000, 	0x0538000, 	0x0a70000, 	0x14e0000,	0x08c0000
	},

	/*4*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*5*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/* 6=22 KING (Both)*/
	{
		0x0000062, 	0x00000e5, 	0x00001ca, 	0x0000394, 	0x0000308,
		0x0000c43, 	0x0001ca7, 	0x000394e, 	0x000729c, 	0x0006118,
		0x0018860, 	0x00394e0, 	0x00729c0, 	0x00e5380, 	0x00c2300,
		0x0310c00, 	0x0729c00, 	0x0e53800, 	0x1ca7000, 	0x1846000,
		0x0218000, 	0x0538000, 	0x0a70000, 	0x14e0000, 	0x08c0000
	},
	/*7*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*8*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*9*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*10*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*11*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*12*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*13*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*14*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*15*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*16*/{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	/*17 黑PAWN*/
	{
		0x0000020, 	0x0000040, 	0x0000080, 	0x0000100, 	0x0000200,
		0x0000400, 	0x0000800, 	0x0001000, 	0x0002000, 	0x0004000,
		0x0008000, 	0x0010000, 	0x0020000, 	0x0040000,	0x0080000,
		0x0100000, 	0x0200000, 	0x0400000, 	0x0800000,	0x1000000,
		0x0000000, 	0x0000000, 	0x0000000, 	0x0000000,	0x0000000
	},
	/*18 黑SILVER*/
	{
		0x0000060, 	0x00000e0, 	0x00001c0, 	0x0000380, 	0x0000300,
		0x0000c02, 	0x0001c05, 	0x000380a, 	0x0007014, 	0x0006008,
		0x0018040, 	0x00380a0, 	0x0070140, 	0x00e0280,	0x00c0100,
		0x0300800, 	0x0701400, 	0x0e02800, 	0x1c05000,	0x1802000,
		0x0010000, 	0x0028000, 	0x0050000, 	0x00a0000,	0x0040000
	},
	/*19 黑GOLD*/
	{
		0x0000062, 	0x00000e5, 	0x00001ca, 	0x0000394, 	0x0000308,
		0x0000c41, 	0x0001ca2, 	0x0003944, 	0x0007288, 	0x0006110,
		0x0018820, 	0x0039440, 	0x0072880, 	0x00e5100,	0x00c2200,
		0x0310400, 	0x0728800, 	0x0e51000, 	0x1ca2000,	0x1844000,
		0x0208000, 	0x0510000, 	0x0a20000, 	0x1440000,	0x0880000
	},
};
string showchess[] = {
	"X","w步","w銀","w金","w角","w飛","王","X",
	"X","wㄈ","w全","X","w馬","w龍","X","X","X"
	"X","b步","b銀","b金","b角","b飛","玉","X",
	"X","wㄈ","b全","X","b馬","b龍","X","X"
};
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
	handcount = 0;
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

	hand[0] = BLANK;
	hand[1] = BLANK;
	hand[2] = BLANK;
	hand[3] = BLANK;
	hand[4] = BLANK;
	hand[5] = BLANK;
	hand[6] = BLANK;
	hand[7] = BLANK;
	hand[8] = BLANK;
	hand[9] = BLANK; 
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
	for (int i = 0; i < 7; i++)
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
		int chess = -1;
		for (int j = 0; j < 5; j++) //128 = 黑 ; 143 = 白  ;207 = 紅底白字  ; 192 = 紅底黑字
		{
			printf("｜");
			if (board_count < 25) chess = board[board_count];
			else chess = hand[board_count - 25];
			switch (chess)
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
		printf("%2s", rank_name[rank_count++]);//*
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
	puts(" ");


}
bool Board::isGameOver() {
	return false;
}

//Rules
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

U32 DstBoard(Board board, int chessnumber, int from_pos,int to_pos) {
	U32 TargetBoard = 1 << to_pos ;
	bool turn = chessnumber - 16 > 0 ? BLACK : WHITE;
	U32 bs;

	switch (chessnumber)
	{
	case BISHOP:
		return BishopMove(board, from_pos, turn) & TargetBoard;
	case ROOK:
		return  RookMove(board, from_pos, turn) & TargetBoard;
	case PAWN | UPGRADED:
		return Movement[GOLD][from_pos] & TargetBoard;
	case SILVER | UPGRADED:
		return Movement[GOLD][from_pos] & TargetBoard;
	case BISHOP | UPGRADED:
		return (BishopMove(board, from_pos, turn) | Movement[KING][from_pos]) & TargetBoard;
	case ROOK | UPGRADED:
		return (RookMove(board, from_pos, turn) | Movement[KING][from_pos]) & TargetBoard;
	case PAWN | UPGRADED | BLACKCHESS:
		return Movement[GOLD | BLACKCHESS][from_pos] & TargetBoard;
	case SILVER | UPGRADED | BLACKCHESS:
		return Movement[GOLD | BLACKCHESS][from_pos] & TargetBoard;
	case BISHOP | UPGRADED | BLACKCHESS:
		return (BishopMove(board, from_pos, turn) | Movement[KING][from_pos]) & TargetBoard;
	case ROOK | UPGRADED | BLACKCHESS:
		return (RookMove(board, from_pos, turn) | Movement[KING][from_pos]) & TargetBoard;
	default:
		/*
		PAWN   bPAWN
		SILVER bSILVER
		GOLD   bGOLD
		*/
		if (board.bitboard[chessnumber] == EMPTY)
		{
			std::cout << "Invalid Move:chess# " << chessnumber << "not exist\n";
			return EMPTY;
		}
		return Movement[chessnumber][from_pos] & TargetBoard;
	}
}

//外部函式
int ConvertInput(std::string position) {
	/*cout << position << " = " <<
	(int)(5 * (position[0] - 'A'))<<"+"<< (int)('5' - position[1]) <<
	" = "<< (int)(5 * (position[0] - 'A') + ('5' - position[1] )) << endl;*/
	return (int)(5 * (position[0] - 'A') + ('5' - position[1]));
}
bool Board::DoMove(Action m_Action)
{
	//00 000001 000000 000001 001010 001111
	//000001 
	//Action input;
	//cin >> input;
	//m_Action = input;
	int from_pos = (m_Action << 26)>>26,
		to_pos=(m_Action<<20)>>26,
		pro=m_Action>>24,
		chessnumber=(m_Action<<14)>>26,
		dstchess = (m_Action<<8)>>25,
		turn = (chessnumber - BLACKCHESS < 0) ? WHITE : BLACK;//board[src]

	//cout << pro <<  " " <<dstchess<< " " << chessnumber<< " "<< to_pos<< " " <<from_pos<< " "  << endl;
	U32 dstboard = 1 << to_pos;
	if (dstchess != BLANK)//要去的地方有敵隊棋子=>吃掉
	{
		occupied[(dstchess - BLACKCHESS < 0) ? WHITE : BLACK] ^= dstboard; //清空被吃掉那個位置 哪種顏色的棋子被吃 就更新那顏色的occupied
		int eaten = board[to_pos];
		bitboard[eaten] = 0;//那顆棋子不見了
		cout << "[" << showchess[chessnumber] << "] 吃掉 " << "[" << showchess[eaten] << "]" << endl;
		eaten ^= BLACKCHESS;
		hand[handcount] = eaten;
		handcount++;
	}

	bitboard[chessnumber] = dstboard;//更新該棋子自己的位置
	board[to_pos] = chessnumber;//將board上面目的位置更新為該棋子(編號)
	board[from_pos] = BLANK;//原本清空
	occupied[turn] = ((occupied[turn] ^ (1 << from_pos) | dstboard)<<7)>>7;//更新該方的occupied
	cout << occupied[turn] << endl;;
	return true;
}
bool Human_DoMove(Board &currentboard, int turn)
{
	string from, to;
	int pro;
	cout << "Input X# X# 0/1:";
	cin >> from >> to >> pro;
	if ((pro < 0 || pro>1) ||
		(from[0] < 'A'&&from[1] > 'G') && (from[1] >= '0'&&from[1] > '5') &&
		(to[0] < 'A'&&to[1] > 'G') && (to[1] < '0'&&to[1] > '5'))
	{
		cout << "Invalid Move:bad input\n";
		return false;
	}
	else
	{
		int from_pos = ConvertInput(from);
		int to_pos = ConvertInput(to);
		U32 dstboard = EMPTY;
		int chessnumber = EMPTY;
		bool handmove = false;
		//turn = 0白色，看是src哪種旗子

		//src位置的棋是什麼
		
		if (from_pos>24)
		{
			if (currentboard.hand[from_pos - 25] != 0)
			{
				// 打入
				chessnumber = currentboard.hand[from_pos - 25];
				handmove = true;
			}
			else
			{
				cout << "Invalid Move:Chess not exist" << endl;
				return false;
			}
				
		}
		else
		{
			chessnumber = currentboard.board[from_pos];
		}

		//處理升變 + 打入規則一：不能馬上升變
		if (pro == 1)
		{
			if (!handmove && chessnumber == PAWN || chessnumber == SILVER || chessnumber == BISHOP || chessnumber == ROOK
				|| chessnumber == (PAWN | BLACKCHESS) || chessnumber == (SILVER | BLACKCHESS) || chessnumber == (BISHOP | BLACKCHESS) || chessnumber == (ROOK | BLACKCHESS))
			{
				//是否在敵區內
				if (chessnumber-16<0&&((1 << to_pos) & 0x0003ff) > 0 )
				{
					currentboard.bitboard[chessnumber | UPGRADED] = currentboard.bitboard[chessnumber];
					currentboard.bitboard[chessnumber] = 0;
					currentboard.board[from_pos] = chessnumber | UPGRADED;
					chessnumber |= UPGRADED;
				}
				else if (chessnumber - 16>0&& ((1 << to_pos) & 0x1ff800) > 0)
				{
					currentboard.bitboard[chessnumber | UPGRADED] = currentboard.bitboard[chessnumber];
					currentboard.bitboard[chessnumber] = 0;
					currentboard.board[from_pos] = chessnumber | UPGRADED;
					chessnumber |= UPGRADED;
				}
				else
				{
					cout << "你不在敵區，是在升變沙小" << endl;
					return false;
				}
				/*
				11111					00000
				11111					00000
				00000					00000
				00000					11111
				00000 = 0x0003ff		11111 = 0x1ff800
				*/
			}
			else
			{
				cout << "你又不能升，是在升變沙小" << endl;
				return false;
			}
		}
		
		if (handmove)//若是打入
		{
			/*
			規則二：不能打在不能走的位置
			規則三：步兵不能打在自己步兵同行=>二步
			規則四：步兵不可立即將死>打步詰
			*/
			if (chessnumber == PAWN || chessnumber == (PAWN | BLACKCHESS))
			{
				if (Movement[PAWN][to_pos] == 0)
				{
					cout << "Invalid Move!: " << showchess[chessnumber] << " 打入該位置不能移動" << endl;
					return false;
				}
				else if (currentboard.board[to_pos] == KING || currentboard.board[to_pos] == (KING | BLACKCHESS))
				{
					cout << "Invalid Move!: " << showchess[chessnumber] << " 打步詰" << endl;
					return false;
				}
				else
				{
					U32 samecolumn = 1 << to_pos;
					U32 temp = 1 << to_pos;
					while (temp)
					{
						temp <<= 5;
						samecolumn |= temp;
					}
					temp = 1 << to_pos;
					while (temp)
					{
						temp >>= 5;
						samecolumn |= temp;
					}
					samecolumn <<= 7;
					samecolumn >>= 7;

					if ((samecolumn & currentboard.bitboard[chessnumber]) > 0)
					{
						cout << "Invalid Move!: " << showchess[chessnumber] << " 二步" << endl;
						return false;
					}

					Action action = 0;
					action = (pro << 25) | (currentboard.board[to_pos] << 18) | (currentboard.board[from_pos] << 12) | to_pos << 6 | from_pos;
					//cout << action << endl;
					currentboard.DoMove(action);
					return true;
				}
			

			}
		}
		else
		{
			dstboard = DstBoard(currentboard, chessnumber, from_pos, to_pos);//將該位置可走的步法跟目的步法&比對，產生該棋的board結果，不合法就會是0
			if (dstboard == 0)
			{
				cout << "Invalid Move!:invalid "<<showchess[chessnumber]<<" move." << endl;
				return false;
			}
			else
			{
				//00 000000 000000 000000 000000 000000
				//00 000001 ch_dst ch_src dst    src
				Action action = 0;
				action = (pro << 25) | (currentboard.board[to_pos]<<18) | (currentboard.board[from_pos] << 12) | to_pos<<6 | from_pos;
				//cout << action << endl;
				currentboard.DoMove(action);
				return true;
			}

		}
		


	}
}
bool AI_DoMove(Board &board, int isWhiteturn)
{
	cout << "呵呵我是電腦，我這步下完了" << endl;
	return true;
}




//Generator ;Search
int Negascout() { return 0; };
bool MobeGenerator() { return true; };
int QuietscenceSearch() { return 0; };

//Rules
bool Uchifuzume() { return true; };
bool Sennichite() { return true; };
