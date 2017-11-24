#include "board.h"
#include <bitset>
#include <Windows.h>

U32 RookMove(const Board &board, const int pos) {
    // upper (find LSB) ; lower (find MSB)
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

    return rank | file;
}

U32 BishopMove(const Board &board, const int pos) {
    // upper (find LSB) ; lower (find MSB)
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

    return slope1 | slope2;
}

inline U32 Movable(const Board &board, const int srcIndex) {
    int srcChess = board.board[srcIndex];

    if ((srcChess & 15) == BISHOP) {
        if (srcChess & PROMOTE)
            return BishopMove(board, srcIndex) | Movement[KING][srcIndex];
        return BishopMove(board, srcIndex);
    }
    else if ((srcChess & 15) == ROOK) {
        if (srcChess & PROMOTE)
            return RookMove(board, srcIndex) | Movement[KING][srcIndex];
        return RookMove(board, srcIndex);
    }
    else if (srcChess & PROMOTE) {
        return Movement[GOLD | (srcChess & BLACKCHESS)][srcIndex];
    }
    return Movement[srcChess][srcIndex];
}

string showchess[] = {
	"X","�B","��","��","��","��","��","X",
	"X","�w","��","X","��","�s","X","X",
	"X","�B","��","��","��","��","��","X",
	"X","�w","��","X","��","�s","X","X"
};
const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
inline void SetColor(int color = 8) {
	SetConsoleTextAttribute(hConsole, color);
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

	//�ˬd��l�Ʀ������\�Ϊ�
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
	for (int i = 0; i < 5; i++)
	{
		puts("�X�U�X�U�X�U�X�U�X�U�X�U�X");
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
		int chess = -1;
		for (int j = 0; j < 5; j++) //128 = �� ; 143 = ��  ;207 = �����զr  ; 192 = �����¦r
		{
			printf("�U");/*
			if (board_count < 25) chess = board[board_count];
			else chess = hand[board_count - 25];*/
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
			case PAWN | PROMOTE: // 9 white e_pawn
				SetColor(207);
				printf("%2s", "�w");
				break;
			case SILVER | PROMOTE: // 10 white e_silver
				SetColor(207);
				printf("%2s", "��");
				break;
			case BISHOP | PROMOTE: // 12 white e_bishop
				SetColor(207);
				printf("%2s", "��");
				break;
			case ROOK | PROMOTE: // 13 white e_rook
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
			case PAWN | PROMOTE | BLACKCHESS: //16 black e_pawn
				SetColor(192);
				printf("%2s", "�w");
				break;
			case SILVER | PROMOTE | BLACKCHESS: //17 black e_silver
				SetColor(192);
				printf("%2s", "��");
				break;
			case BISHOP | PROMOTE | BLACKCHESS: //18 black e_bishop
				SetColor(192);
				printf("%2s", "��");
				SetColor();
				break;
			case ROOK | PROMOTE | BLACKCHESS: //19 black e_rook
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
		printf("%2s", rank_name[rank_count++]);//*
		printf("\t ");

		puts(" ");		//*
	}
	puts(" ");
	puts("�X�U 5�U 4�U 3�U 2�U 1�U�X");
	cout << " F";
	for (int k = board_count; k < 35; k++)
	{
		cout << "�U";
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
			cout <<"�U"<< endl;
			cout << " G";
		}
		
	}
	cout << "�U" << endl;

	puts("�X�U�X�U�X�U�X�U�X�U�X�U�X");
}
bool Board::isGameOver() {
    if (bitboard[KING] == 0) {
        cout << "*****[�¤����]*****" << endl;
        return true;
    }
	if (bitboard[KING | BLACKCHESS] == 0) {
		cout << "*****[�դ����]*****" << endl;
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

	if (!srcChess) {
		cerr << "Invalid Move: No chess at " << srcIndex << endl;
		return;
	}
	if (srcIndex < BOARD_SIZE) { // ����
		if (dstChess) { // �Y
			occupied[turn ^ 1] ^= dstboard; // ��s�����W���p
			bitboard[dstChess] ^= dstboard; // ��s�����
			board[EatToHand[dstChess ^ BLACKCHESS]]++; // �ର�Ӥ���
		}
        if (pro) { // ����
            bitboard[srcChess] ^= 1 << srcIndex; // �����ǳƤ��ܪ����
            srcChess ^= PROMOTE;
        }
		board[srcIndex] = BLANK; // �쥻�M��
		occupied[turn] ^= (1 << srcIndex) | dstboard; // ��s�Ӥ���W���p
		bitboard[srcChess] ^= dstboard; // ��s�Ӥ���
	}
	else { // ���J
		occupied[turn] ^= dstboard; // ���J���W����m
		bitboard[srcChess] ^= dstboard; // ���J�Ӥ�ƪ���m
		board[EatToHand[srcIndex]]--; // ��ָӤ�P
	}
	board[dstIndex] = srcChess; // ��m��ت�
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

	if (srcIndex < BOARD_SIZE) { // ���e�O����
        if (dstChess) { // ���e���Y�l
            occupied[turn ^ 1] ^= dstboard; // �٭�����W���p
            bitboard[dstChess] ^= dstboard; // �٭�����
            board[EatToHand[dstChess ^ BLACKCHESS]]--; // �q�Ӥ��Ʋ���
        }

        if (pro) // ���e������
            bitboard[srcChess ^ PROMOTE] ^= 1 << srcIndex; // �����w���ܪ����
        
        board[srcIndex] = srcChess; // �٭�
        occupied[turn] ^= (1 << srcIndex) | dstboard; // �٭�Ӥ���W���p
        bitboard[srcChess] ^= dstboard; // �٭�Ӥ���
	}
	else { // ���e�O���J
        occupied[turn] ^= dstboard; // �������J���W����m
        bitboard[srcChess] ^= dstboard; // �������J�Ӥ�ƪ���m
        board[EatToHand[srcChess]]++; // ���^�Ӥ�P
	}
    board[dstIndex] = dstChess; // �٭�ت���
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

	//src��m���ѬO����
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
			cout << "Invalid Move!: " << showchess[srcChess] << " �Ӧ�m���Ѥl" << endl;
			return 0;
		}
			
		if ((srcChess & 15) == PAWN) {
			if (!Movement[srcChess][dstIndex]) {
				cout << "Invalid Move!: " << showchess[srcChess] << " ���J�Ӧ�m���ಾ��" << endl;
				return 0;
			}

			if (column_mask(dstIndex) & board.bitboard[srcChess]) {
				cout << "Invalid Move!: " << showchess[srcChess] << " �G�B" << endl;
				return 0;
			}

            /* TODO: �B�L���i�ߧY�N��>���B��(����) */
		}
	}
	else {
		//�B�z���� + ���J�W�h�@�G���న�W����
		if (pro) {
			if ((srcChess == PAWN || srcChess == SILVER || srcChess == BISHOP || srcChess == ROOK ||
				srcChess == (PAWN | BLACKCHESS) || srcChess == (SILVER | BLACKCHESS) || srcChess == (BISHOP | BLACKCHESS) || srcChess == (ROOK | BLACKCHESS))) {
				//�O�_�b�İϤ�
				if (!((1 << dstIndex) &
                    (srcChess < BLACKCHESS ? BLACK_AREA : WHITE_AREA))) {
					cout << "�A���b�İϡA�������" << endl;
					return 0;
				}
			}
			else {
				cout << "�A�S�������" << endl;
				return 0;
			}
		}

        //�N�Ӧ�m�i�����B�k��ت��B�k & ���A���͸ӴѪ�board���G�A���X�k�N�|�O0
		if (!(Movable(board, srcIndex) & (1 << dstIndex))) {
			cout << "Invalid Move!:invalid "<<showchess[srcChess]<<" move." << endl;
			return 0;
		}
	}

    return (pro << 24) | (board.board[dstIndex] << 18) | (srcChess << 12) | (dstIndex << 6) | srcIndex;
}

Action AI_DoMove(Board &board, int turn) {
	cout << "�ڬO�q���A�o�B�U���F" << endl;
	return 1;
}


//Generator ;Search
int Negascout() { return 0; };
int QuietscenceSearch() { return 0; };

//Rules
bool Uchifuzume() { return true; };
bool Sennichite() { return true; };


void MoveGenerator(const Board &board, Action *movelist, int &start, const int turn) {
    U32 srcboard, dstboard, tmpboard, src, dst;
    for (int i = 0; i < 10; i++) {
        srcboard = board.bitboard[MoveOrdering[i] | (turn << 4)];
        while (srcboard) {
            src = BitScan(srcboard);
            srcboard ^= 1 << src;
            dstboard = Movable(board, src); // �@���ѩҦ������B�d��
            dstboard &= (dstboard ^ board.occupied[turn]); // ��ڤ�ư���
            while (dstboard) {
                dst = BitScan(dstboard);
                dstboard ^= 1 << dst;
                /* TODO: promote */
                movelist[(start)++] = (board.board[dst] << 18) | (board.board[src] << 12) | (dst << 6) | src;
            }
        }
    }
}

/* TODO: ������ */
U32 Promotable(int src, int dst, int turn) {
    if (turn == WHITE ?
        // �@�벾�� || �Y�l����
        (src < BOARD_SIZE && dst < 5) || (src < 5 && dst < BOARD_SIZE) :
        (src < BOARD_SIZE && dst > 19) || (src > 19 && src < BOARD_SIZE && dst < BOARD_SIZE))
        return PRO_MASK;
    return 0;
}