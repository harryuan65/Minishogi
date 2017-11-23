#include "board.h"
#include <bitset>
#include <Windows.h>

U32 Movement[20][BOARD_SIZE] = {
	/* 0 */{ 0 },
	/* 1 PAWN */
	{
		0x0000000, 	0x0000000, 	0x0000000, 	0x0000000, 	0x0000000,
		0x0000001, 	0x0000002, 	0x0000004, 	0x0000008, 	0x0000010,
		0x0000020, 	0x0000040, 	0x0000080, 	0x0000100,	0x0000200,
		0x0000400, 	0x0000800, 	0x0001000, 	0x0002000,	0x0004000,
		0x0008000, 	0x0010000, 	0x0020000, 	0x0040000,	0x0080000
	},
	/* 2 SILVER */
	{
		0x0000040, 	0x00000a0, 	0x0000140, 	0x0000280, 	0x0000100,
		0x0000803, 	0x0001407, 	0x000280e, 	0x000501c, 	0x0002018,
		0x0010060, 	0x00280e0, 	0x00501c0, 	0x00a0380,	0x0040300,
		0x0200c00, 	0x0501c00, 	0x0a03800, 	0x1407000,	0x0806000,
		0x0018000, 	0x0038000, 	0x0070000, 	0x00e0000,	0x00c0000
	}
	,
	/* 3 GOLD */
	{
		0x0000022, 	0x0000045, 	0x000008a, 	0x0000114, 	0x0000208,
		0x0000443, 	0x00008a7, 	0x000114e, 	0x000229c, 	0x0004118,
		0x0008860, 	0x00114e0, 	0x00229c0, 	0x0045380,	0x0082300,
		0x0110c00, 	0x0229c00, 	0x0453800, 	0x08a7000,	0x1046000,
		0x0218000, 	0x0538000, 	0x0a70000, 	0x14e0000,	0x08c0000
	},

	/*4*/{ 0 },
	/*5*/{ 0 },
	/* 6=22 KING (Both) */
	{
		0x0000062, 	0x00000e5, 	0x00001ca, 	0x0000394, 	0x0000308,
		0x0000c43, 	0x0001ca7, 	0x000394e, 	0x000729c, 	0x0006118,
		0x0018860, 	0x00394e0, 	0x00729c0, 	0x00e5380, 	0x00c2300,
		0x0310c00, 	0x0729c00, 	0x0e53800, 	0x1ca7000, 	0x1846000,
		0x0218000, 	0x0538000, 	0x0a70000, 	0x14e0000, 	0x08c0000
	},
	/*7*/{ 0 },
	/*8*/{ 0 },
	/*9*/{ 0 },
	/*10*/{ 0 },
	/*11*/{ 0 },
	/*12*/{ 0 },
	/*13*/{ 0 },
	/*14*/{ 0 },
	/*15*/{ 0 },
	/*16*/{ 0 },
	/* 17 黑PAWN */
	{
		0x0000020, 	0x0000040, 	0x0000080, 	0x0000100, 	0x0000200,
		0x0000400, 	0x0000800, 	0x0001000, 	0x0002000, 	0x0004000,
		0x0008000, 	0x0010000, 	0x0020000, 	0x0040000,	0x0080000,
		0x0100000, 	0x0200000, 	0x0400000, 	0x0800000,	0x1000000,
		0x0000000, 	0x0000000, 	0x0000000, 	0x0000000,	0x0000000
	},
	/* 18 黑SILVER */
	{
		0x0000060, 	0x00000e0, 	0x00001c0, 	0x0000380, 	0x0000300,
		0x0000c02, 	0x0001c05, 	0x000380a, 	0x0007014, 	0x0006008,
		0x0018040, 	0x00380a0, 	0x0070140, 	0x00e0280,	0x00c0100,
		0x0300800, 	0x0701400, 	0x0e02800, 	0x1c05000,	0x1802000,
		0x0010000, 	0x0028000, 	0x0050000, 	0x00a0000,	0x0040000
	},
	/* 19 黑GOLD */
	{
		0x0000062, 	0x00000e5, 	0x00001ca, 	0x0000394, 	0x0000308,
		0x0000c41, 	0x0001ca2, 	0x0003944, 	0x0007288, 	0x0006110,
		0x0018820, 	0x0039440, 	0x0072880, 	0x00e5100,	0x00c2200,
		0x0310400, 	0x0728800, 	0x0e51000, 	0x1ca2000,	0x1844000,
		0x0208000, 	0x0510000, 	0x0a20000, 	0x1440000,	0x0880000
	},
};
string showchess[] = {
	"X","步","銀","金","角","飛","玉","X",
	"X","ㄈ","全","X","馬","龍","X","X",
	"X","步","銀","金","角","飛","王","X",
	"X","ㄈ","全","X","馬","龍","X","X"
};
const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
inline void SetColor(int color = 8) {
	SetConsoleTextAttribute(hConsole, color);
}

//Rules
U32 RookMove(Board &board, int pos, int turn) {
	// upper (find LSB) ; lower (find MSB)
	U32 ret = 0;
	U32 occupied = board.occupied[WHITE] | board.occupied[BLACK];
	U32 rank, file;
	U32 upper, lower;

	// row
	upper = (occupied & row_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & row_lower[pos]) | LOWEST_BOARD_POS;

	upper = (upper & (~upper + 1)) << 1;
    lower = 1 << BitScanRev(lower);

	rank = (upper - lower) & row_mask(pos);

	// column
	upper = (occupied & column_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & column_lower[pos]) | LOWEST_BOARD_POS;

	upper = (upper & (~upper + 1)) << 1;
    lower = 1 << BitScanRev(lower);

	file = (upper - lower) & column_mask(pos);

	ret = rank | file;
	ret &= (turn == WHITE) ? ret ^ board.occupied[WHITE] \
		: ret ^ board.occupied[BLACK];
	return ret;
}

U32 BishopMove(Board &board, int pos, int turn) {
	// upper (find LSB) ; lower (find MSB)
	U32 ret = 0;
	U32 occupied = board.occupied[WHITE] | board.occupied[BLACK];
	U32 slope1, slope2;
	U32 upper, lower;

	// slope1 "/"
	upper = (occupied & slope1_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & slope1_lower[pos]) | LOWEST_BOARD_POS;

    upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope1 = (upper - lower) & slope1_mask(pos);

	// slope2 "\"
	upper = (occupied & slope2_upper[pos]) | HIGHTEST_BOARD_POS;
	lower = (occupied & slope2_lower[pos]) | LOWEST_BOARD_POS;

    upper = (upper & (~upper + 1)) << 1;
    lower = 1 << BitScanRev(lower);

	slope2 = (upper - lower) & slope2_mask(pos);

	ret = slope1 | slope2;
	ret = (turn == WHITE) ? ret & (ret ^ board.occupied[WHITE]) \
		: ret & (ret ^ board.occupied[BLACK]);
	return ret;
}

inline U32 DstBoard(Board board, int srcChess, int srcIndex, int dstIndex) {
	U32 TargetBoard = 1 << dstIndex;
	bool turn = srcChess > BLACKCHESS;

	switch (srcChess) {
	case BISHOP:
		return (BishopMove(board, srcIndex, turn) & TargetBoard);
	case BISHOP|BLACKCHESS:
		return (BishopMove(board, srcIndex, turn) & TargetBoard);
	case ROOK:
		return  (RookMove(board, srcIndex, turn) & TargetBoard);
	case ROOK|BLACKCHESS:
		return  (RookMove(board, srcIndex, turn) & TargetBoard);
	case PAWN | PROMOTE:
		return (Movement[GOLD][srcIndex] & TargetBoard);
	case SILVER | PROMOTE:
		return (Movement[GOLD][srcIndex] & TargetBoard);
	case BISHOP | PROMOTE:
		return ((BishopMove(board, srcIndex, turn) | Movement[KING][srcIndex]) & TargetBoard);
	case ROOK | PROMOTE:
		return ((RookMove(board, srcIndex, turn) | Movement[KING][srcIndex]) & TargetBoard);
	case PAWN | PROMOTE | BLACKCHESS:
		return (Movement[GOLD | BLACKCHESS][srcIndex] & TargetBoard);
	case SILVER | PROMOTE | BLACKCHESS:
		return (Movement[GOLD | BLACKCHESS][srcIndex] & TargetBoard);
	case BISHOP | PROMOTE | BLACKCHESS:
		return ((BishopMove(board, srcIndex, turn) | Movement[KING][srcIndex]) & TargetBoard);
	case ROOK | PROMOTE | BLACKCHESS:
		return ((RookMove(board, srcIndex, turn) | Movement[KING][srcIndex]) & TargetBoard);
	case KING | BLACKCHESS://因為這個傢伙的Movement跟KING一樣 就不特別創到22個
		return (Movement[KING][srcIndex] & TargetBoard);
	default:
		/*
		PAWN   bPAWN
		SILVER bSILVER
		GOLD   bGOLD
		*/
        if (board.bitboard[srcChess] == EMPTY) {
			cout << "Invalid Move:chess# " << srcChess << " not exist\n";
			return EMPTY;
		}
           return Movement[srcChess][srcIndex] & TargetBoard;
	}
}

Board::Board() { Initialize(); }
Board::~Board() {}

bool Board::Initialize() {
    memset(bitboard, BLANK, 32 * sizeof(int));
    memset(board, BLANK, TOTAL_BOARD_SIZE * sizeof(int));
    memset(hand, BLANK, 10 * sizeof(int));
	record.clear();

	occupied[WHITE] = WHITE_INIT;
	occupied[BLACK] = BLACK_INIT;

	bitboard[PAWN] = W_PAWN_INIT;
	bitboard[SILVER] = W_SILVER_INIT;
	bitboard[GOLD] = W_GOLD_INIT;
	bitboard[BISHOP] = W_BISHOP_INIT;
	bitboard[ROOK] = W_ROOK_INIT;
	bitboard[KING] = W_KING_INIT;

	bitboard[PAWN | BLACKCHESS] = B_PAWN_INIT;
	bitboard[SILVER | BLACKCHESS] = B_SILVER_INIT;
	bitboard[GOLD | BLACKCHESS] = B_GOLD_INIT;
	bitboard[BISHOP | BLACKCHESS] = B_BISHOP_INIT;
	bitboard[ROOK | BLACKCHESS] = B_ROOK_INIT;
	bitboard[KING | BLACKCHESS] = B_KING_INIT;

	board[A5] = ROOK | BLACKCHESS;
	board[A4] = BISHOP | BLACKCHESS;
	board[A3] = SILVER | BLACKCHESS;
	board[A2] = GOLD | BLACKCHESS;
	board[A1] = KING | BLACKCHESS;
	board[B1] = PAWN |BLACKCHESS ;

	board[E1] = ROOK;
	board[E2] = BISHOP;
	board[E3] = SILVER;
	board[E4] = GOLD;
	board[E5] = KING;
	board[D5] = PAWN;

	return true;
}

bool Board::Initialize(string &board_str) {
	stringstream ss;
	string token;
	ss << board_str;
	int i = 0, turn, chess;
	while(getline(ss, token, ' ') && i<25) {
		chess = atoi(token.c_str());
		if (showchess[chess] == "X")chess = BLANK;
		board[i] = chess;
		if (chess != 0) {
			turn = chess > BLACKCHESS;
			bitboard[chess] |= 1 << i;
			occupied[turn] |= 1 << i;
		}
		i++;
	}
	if (i != 25) {
		cout << "Invalid initialization:Not enough chess" << endl;
		return false;
	}

	//檢查初始化成不成功用的
	/*for (int i = 1; i < 32; i++)
	{
		if (showchess[i] != "X")
		{
			cout << "bitboard["<<showchess[i] << "] = " << bitboard[i] << endl;
		}
	}
	cout << "occupied[WHITE] = " << occupied[WHITE] << endl;
	cout << "occupied[BLACK] = " << occupied[BLACK] << endl;
	*/
	return true;
}

void Board::PrintChessBoard() {
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
		puts("—｜—｜—｜—｜—｜—｜—");
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
			printf("｜");/*
			if (board_count < 25) chess = board[board_count];
			else chess = hand[board_count - 25];*/
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
			case PAWN | PROMOTE: // 9 white e_pawn
				SetColor(207);
				printf("%2s", "ㄈ");
				break;
			case SILVER | PROMOTE: // 10 white e_silver
				SetColor(207);
				printf("%2s", "全");
				break;
			case BISHOP | PROMOTE: // 12 white e_bishop
				SetColor(207);
				printf("%2s", "馬");
				break;
			case ROOK | PROMOTE: // 13 white e_rook
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
			case PAWN | PROMOTE | BLACKCHESS: //16 black e_pawn
				SetColor(192);
				printf("%2s", "ㄈ");
				break;
			case SILVER | PROMOTE | BLACKCHESS: //17 black e_silver
				SetColor(192);
				printf("%2s", "全");
				break;
			case BISHOP | PROMOTE | BLACKCHESS: //18 black e_bishop
				SetColor(192);
				printf("%2s", "馬");
				SetColor();
				break;
			case ROOK | PROMOTE | BLACKCHESS: //19 black e_rook
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

		puts(" ");		//*
	}
	puts(" ");
	puts("—｜ 5｜ 4｜ 3｜ 2｜ 1｜—");
	cout << " F";
	for (int k = board_count; k < 35; k++)
	{
		cout << "｜";
		if (board[k] > 0)
		{
			SetColor(128+15*(k>29));
			cout << showchess[k % 5 + 1];
			SetColor();
		}
		else
		{
			cout << "  ";
		}
		if (k == 29) {
			cout <<"｜"<< endl;
			cout << " G";
		}
		
	}
	cout << "｜" << endl;

	puts("—｜—｜—｜—｜—｜—｜—");

}
bool Board::isGameOver() {
    if (bitboard[KING] == 0) {
        cout << "*****[黑方獲勝]*****" << endl;
        return true;
    }
	if (bitboard[KING | BLACKCHESS] == 0) {
		cout << "*****[白方獲勝]*****" << endl;
		return true;
	}
	return false;
}
void Board::DoMove(const Action action) {
    record.push_back(action);
    // 0 board[dst] board[src] dst src
    U32 srcIndex = (action & SRC_INDEX_MASK),
        dstIndex = (action & DST_INDEX_MASK) >> 6,
        srcChess = (action & SRC_CHESS_MASK) >> 12,
        dstChess = (action & DST_CHESS_MASK) >> 18,
        pro = action >> 24,
        turn = srcChess > BLACKCHESS,
        dstboard = 1 << dstIndex;

	if (srcChess == 0) {
		cout << "Invalid Move: No chess at " << srcIndex << endl;
		return;
	}
	if (srcIndex < BOARD_SIZE) { // 移動
		if (dstChess) { // 吃
			occupied[turn ^ 1] ^= dstboard; // 更新對方場上狀況
			bitboard[dstChess] ^= dstboard; // 更新對方手排
			board[EatToHand[dstChess ^ BLACKCHESS]]++; // 轉為該方手排
		}
        if (pro) { // 升變
            bitboard[srcChess] ^= 1 << srcIndex; // 移除準備升變的手排
            srcChess ^= PROMOTE;
        }
		board[srcIndex] = BLANK; // 原本清空
		occupied[turn] ^= (1 << srcIndex) | dstboard; // 更新該方場上狀況
		bitboard[srcChess] ^= dstboard; // 更新該方手排
	}
	else { // 打入
		occupied[turn] ^= dstboard; // 打入場上的位置
		bitboard[srcChess] ^= dstboard; // 打入該位置的手排
		board[EatToHand[srcIndex]]--; // 減少該手牌
	}
	board[dstIndex] = srcChess; // 放置到目的
}

void Board::UndoMove() {
	Action redo = record.back();
	record.pop_back();
	// 0 board[dst] board[src] dst src
    U32 srcIndex = (redo & SRC_INDEX_MASK),
        dstIndex = (redo & DST_INDEX_MASK) >> 6,
        srcChess = (redo & SRC_CHESS_MASK) >> 12,
        dstChess = (redo & DST_CHESS_MASK) >> 18,
        pro = redo >> 24,
        turn = srcChess > BLACKCHESS,
        dstboard = 1 << dstIndex;

	if (srcIndex < BOARD_SIZE) { // 之前是移動
        if (dstChess) { // 之前有吃子
            occupied[turn ^ 1] ^= dstboard; // 還原對方場上狀況
            bitboard[dstChess] ^= dstboard; // 還原對方手排
            board[EatToHand[dstChess ^ BLACKCHESS]]--; // 從該方手排移除
        }

        if (pro) // 之前有升變
            bitboard[srcChess ^ PROMOTE] ^= 1 << srcIndex; // 移除已升變的手排
        
        board[srcIndex] = srcChess; // 還原
        occupied[turn] ^= (1 << srcIndex) | dstboard; // 還原該方場上狀況
        bitboard[srcChess] ^= dstboard; // 還原該方手排
	}
	else { // 之前是打入
        occupied[turn] ^= dstboard; // 取消打入場上的位置
        bitboard[srcChess] ^= dstboard; // 取消打入該位置的手排
        board[EatToHand[srcChess]]++; // 收回該手牌
	}
    board[dstIndex] = dstChess; // 還原目的棋
}
int ConvertInput(std::string position) {
	/*cout << position << " = " <<
	(int)(5 * (position[0] - 'A'))<<"+"<< (int)('5' - position[1]) <<
	" = "<< (int)(5 * (position[0] - 'A') + ('5' - position[1] )) << endl;*/
	return (int)(5 * (position[0] - 'A') + ('5' - position[1]));
}

Action Human_DoMove(Board &board, int turn) {
	string from, to;
	int pro;
	cout << "Input X# X# 0/1:";
    cin.clear(); cin.ignore();
	cin >> from >> to >> pro;
	if ((pro < 0 || pro > 1) ||
		(from[0] < 'A'||from[0] > 'G') 
		||(from[1] < '0'||from[1] > '5') ||
		(to[0] < 'A'||to[0] > 'G') || (to[1] < '0'||to[1] > '5')) {
		cout << "Invalid Move:bad input\n";
		return 0;
	}

	int srcIndex = ConvertInput(from);
	int dstIndex = ConvertInput(to);
	int srcChess;
	bool handmove = false;

	//src位置的棋是什麼
	if (srcIndex < BOARD_SIZE) srcChess = board.board[srcIndex];
	else {
        if (!board.board[srcIndex]) {
            cout << "Invalid Move: No chess to handmove" << endl;
            return 0;
        }
		handmove = true;
		srcChess = (srcIndex < 40 ? BLACKCHESS : 0) | (srcIndex % 5 + 1);
	}

	if (srcChess >> 4 != turn) {
		cout << "Invalid Move: "<<showchess[srcChess]<<" is not your chess" << endl;
		return 0;
	}

	if (handmove) {
		if (pro) { 
			cout << "Invalid Move!: Promotion prohobited on handmove" << endl;
            return 0;
		}
			
		if (board.board[dstIndex]) {
			cout << "Invalid Move!: " << showchess[srcChess] << " 該位置有棋子" << endl;
			return 0;
		}
			
		if ((srcChess & 15) == PAWN) {
			if (!Movement[srcChess][dstIndex]) {
				cout << "Invalid Move!: " << showchess[srcChess] << " 打入該位置不能移動" << endl;
				return 0;
			}

			if (column_mask(dstIndex) & board.bitboard[srcChess]) {
				cout << "Invalid Move!: " << showchess[srcChess] << " 二步" << endl;
				return 0;
			}

            /* TODO: 步兵不可立即將死>打步詰(未做) */
		}
	}
	else {
		//處理升變 + 打入規則一：不能馬上升變
		if (pro) {
			if ((srcChess == PAWN || srcChess == SILVER || srcChess == BISHOP || srcChess == ROOK ||
				srcChess == (PAWN | BLACKCHESS) || srcChess == (SILVER | BLACKCHESS) || srcChess == (BISHOP | BLACKCHESS) || srcChess == (ROOK | BLACKCHESS))) {
				//是否在敵區內
				if (!((1 << dstIndex) &
                    (srcChess < BLACKCHESS ? BLACK_AREA : WHITE_AREA))) {
					cout << "你不在敵區，不能升變" << endl;
					return 0;
				}
			}
			else {
				cout << "你又不能升變" << endl;
				return 0;
			}
		}

        //將該位置可走的步法跟目的步法 & 比對，產生該棋的board結果，不合法就會是0
		if (!DstBoard(board, srcChess, srcIndex, dstIndex)) {
			cout << "Invalid Move!:invalid "<<showchess[srcChess]<<" move." << endl;
			return 0;
		}
	}

    return (pro << 24) | (board.board[dstIndex] << 18) | (srcChess << 12) | (dstIndex << 6) | srcIndex;
}

Action AI_DoMove(Board &board, int turn) {
	cout << "我是電腦，這步下完了" << endl;
	return 1;
}


//Generator ;Search
int Negascout() { return 0; };
bool MobeGenerator() { return true; };
int QuietscenceSearch() { return 0; };

//Rules
bool Uchifuzume() { return true; };
bool Sennichite() { return true; };
