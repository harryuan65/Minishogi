#include "board.h"
#ifdef WINDOWS_10
#define LINE_STRING "�X�U�X�U�X�U�X�U�X�U�X�U�X"
#else
#define LINE_STRING "�X�q�X�q�X�q�X�q�X�q�X�q�X"
#endif


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

    if ((srcChess & 7) == BISHOP) {
        if (srcChess & PROMOTE)
            return BishopMove(board, srcIndex) | Movement[KING][srcIndex];
        return BishopMove(board, srcIndex);
    }
    else if ((srcChess & 7) == ROOK) {
        if (srcChess & PROMOTE)
            return RookMove(board, srcIndex) | Movement[KING][srcIndex];
        return RookMove(board, srcIndex);
    }
    else if (srcChess & PROMOTE) {
        return Movement[GOLD | (srcChess & BLACKCHESS)][srcIndex];
    }
    return Movement[srcChess][srcIndex];
}

const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
inline void SetColor(int color = 8) {
    SetConsoleTextAttribute(hConsole, color);
}

vector<U32> Board::ZOBRIST_TABLE[35];

Board::Board() { 
	/* Make Zobrsit Table */
	if (ZOBRIST_TABLE[0].size() == 0) {
		srand(10);
		for (int i = 0; i < 25; i++) {
			ZOBRIST_TABLE[i] = vector<U32>(30);
		}
		for (int i = 25; i < 35; i++) {
			ZOBRIST_TABLE[i] = vector<U32>(3);
		}
		for (int i = 0; i < 35; i++)
			for (int j = 0; j < ZOBRIST_TABLE[i].size(); j++)
				ZOBRIST_TABLE[i][j] = rand();
	}
	Initialize(); 
}
Board::~Board() {}

void Board::CalZobristNumber() {
	m_whiteHashcode = 0;
	m_blackHashcode = 0;
	for (int i = 0; i < 25; i++) {
		if (board[i] != BLANK) {
			m_whiteHashcode ^= ZOBRIST_TABLE[i][board[i]];
			m_blackHashcode ^= ZOBRIST_TABLE[24 - i][board[i] ^ BLACKCHESS];
		}
	}
	for (int i = 25; i < 35; i++) {
		if (board[i]) {
			m_whiteHashcode ^= ZOBRIST_TABLE[i][1];
			m_blackHashcode ^= ZOBRIST_TABLE[i> 29 ? i - 5 : i + 5][1];
		}
		if (board[i] == 2) {
			m_whiteHashcode ^= ZOBRIST_TABLE[i][2];
			m_blackHashcode ^= ZOBRIST_TABLE[i> 29 ? i - 5 : i + 5][1];
		}
	}
}

void Board::CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess) {
	m_whiteHashcode ^= ZOBRIST_TABLE[srcIndex][srcChess];
	m_whiteHashcode ^= ZOBRIST_TABLE[dstIndex][dstChess];
	if (srcIndex > 24) {
		m_blackHashcode ^= ZOBRIST_TABLE[srcIndex > 29 ? srcIndex - 5 : srcIndex + 5][srcChess];
		m_blackHashcode ^= ZOBRIST_TABLE[24 - dstIndex][dstChess ^ BLACKCHESS];
	}
	else if (dstIndex > 24) {
		m_blackHashcode ^= ZOBRIST_TABLE[24 - srcIndex][srcChess ^ BLACKCHESS];
		m_blackHashcode ^= ZOBRIST_TABLE[dstIndex > 29 ? dstIndex - 5 : dstIndex + 5][dstChess];
	}
	else {
		m_blackHashcode ^= ZOBRIST_TABLE[24 - srcIndex][srcChess ^ BLACKCHESS];
		m_blackHashcode ^= ZOBRIST_TABLE[24 - dstIndex][srcChess ^ BLACKCHESS];
	}
}

bool Board::Initialize() {
    memset(bitboard, BLANK, 32 * sizeof(int));
    memset(board, BLANK, TOTAL_BOARD_SIZE * sizeof(int));
    //memset(hand, BLANK, 10 * sizeof(int));
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

	CalZobristNumber();
    return true;
}

bool Board::Initialize(string &board_str) {
    stringstream ss;
    string token;
    ss << board_str;
    int i = 0, turn, chess;
    while(getline(ss, token, ' ') && i<25) {
        chess = atoi(token.c_str());
        if (CHESS_WORD[chess] == "  ")chess = BLANK;
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
	CalZobristNumber();
    return true;
}

void Board::PrintChessBoard(bool turn) {
	cout << endl;
	cout << "---It's player " << turn << " turn!---" << endl;
	/* Print Other Hand */
	cout << "  �U";
	for (int i = 0; i < 5; i++) {
		if (!turn && board[30 + i])
			cout << CHESS_WORD[1 + i] << "�U";
		else if (turn && board[29 - i])
			cout << CHESS_WORD[5 - i] << "�U";
		else
			cout << "  �U";
	}
	cout << endl << "  �U";
	for (int i = 0; i < 5; i++) {
		if (!turn && board[30 + i] == 2)
			cout << CHESS_WORD[1 + i] << "�U";
		else if (turn && board[29 - i] == 2)
			cout << CHESS_WORD[5 - i] << "�U";
		else
			cout << "  �U";
	}
	/* Print Board */
	cout << endl << LINE_STRING << endl;
	cout << (turn ? "  �U 1�U 2�U 3�U 4�U 5�U" : "  �U 5�U 4�U 3�U 2�U 1�U") << endl;
	cout << LINE_STRING << endl;
	if (turn)
		for (int i = 4; i >= 0; i--) {
			cout << "  �U";
			for (int j = 4; j >= 0; j--) {
				if (board[i * 5 + j] & BLACKCHESS)
					cout << "�s�U";
				else
					cout << CHESS_WORD[board[i * 5 + j]] << "�U";
			}
			cout << endl << " " << (char)('A' + i) << "�U";
			for (int j = 4; j >= 0; j--) {
				if (board[i * 5 + j] != BLANK && !(board[i * 5 + j] & BLACKCHESS))
					cout << "�t�U";
				else
					cout << CHESS_WORD[board[i * 5 + j]] << "�U";
			}
			cout << endl << LINE_STRING << endl;
		}
	else
		for (int i = 0; i < 5; i++) {
			cout << "  �U";
			for (int j = 0; j < 5; j++) {
				if (board[i * 5 + j] != BLANK && !(board[i * 5 + j] & BLACKCHESS))
					cout << "�s�U";
				else
					cout << CHESS_WORD[board[i * 5 + j]] << "�U";
			}
			cout << endl << " " << (char)('A' + i) << "�U";
			for (int j = 0; j < 5; j++) {
				if (board[i * 5 + j] & BLACKCHESS)
					cout << "�t�U";
				else
					cout << CHESS_WORD[board[i * 5 + j]] << "�U";
			}
			cout << endl << LINE_STRING << endl;
		}
	cout << (turn ? "  �U 1�U 2�U 3�U 4�U 5�U" : "  �U 5�U 4�U 3�U 2�U 1�U") << endl;
	cout << LINE_STRING << endl;
	/* Print My Hand */
	cout << " F�U";
	for (int i = 0; i < 5; i++) {
		if (!turn && board[25 + i])
			cout << CHESS_WORD[1 + i] << "�U";
		else if (turn && board[34 - i])
			cout << CHESS_WORD[5 - i] << "�U";
		else
			cout << "  �U";
	}
	cout << endl << "  �U";
	for (int i = 0; i < 5; i++) {
		if (!turn && board[25 + i] == 2)
			cout << CHESS_WORD[1 + i] << "�U";
		else if (turn && board[34 - i] == 2)
			cout << CHESS_WORD[5 - i] << "�U";
		else
			cout << "  �U";
	}
	cout << endl;

	cout << "White Hashcode : " << GetHashcode(0) << endl;
	cout << "Black Hashcode : " << GetHashcode(1) << endl;
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
    // 0 board[dst] board[src] dst src
    U32 srcIndex = (action & SRC_INDEX_MASK),
        dstIndex = (action & DST_INDEX_MASK) >> 6,
        srcChess = BLANK,
        dstChess = BLANK,
        pro = action >> 24,
        dstboard = 1 << dstIndex;

    if (srcIndex < BOARD_SIZE) { // ����
        srcChess = board[srcIndex];
		dstChess = board[dstIndex];
        if (dstChess) { // �Y
            occupied[srcChess < BLACKCHESS] ^= dstboard; // ��s�����W���p
            bitboard[dstChess] ^= dstboard; // ��s�����
            board[EAT_CHESS2INDEX[dstChess]]++; // �ର�Ӥ���
			CalZobristNumber(dstIndex, EAT_CHESS2INDEX[dstChess], dstChess, board[EAT_CHESS2INDEX[dstChess]]);
        }

        occupied[srcChess > BLACKCHESS] ^= (1 << srcIndex) | dstboard; // ��s�Ӥ���W���p
        bitboard[srcChess] ^= 1 << srcIndex; // �����Ӥ��ƭ즳��m

        if (pro) 
			srcChess ^= PROMOTE; // ����
        bitboard[srcChess] ^= dstboard; // ��s�Ӥ��Ʀܥت���m
        board[srcIndex] = BLANK; // �쥻�M��
		CalZobristNumber(srcIndex, dstIndex, srcChess, srcChess);
    }
    else { // ���J
		srcChess = srcIndex > 29 ? srcIndex - 13 : srcIndex - 24;//HandToChess[srcIndex];
		CalZobristNumber(srcIndex, dstIndex, board[srcIndex], srcChess);
        occupied[(srcChess > BLACKCHESS)] ^= dstboard; // ���J���W����m
        bitboard[srcChess] ^= dstboard; // ���J�Ӥ�ƪ���m
        board[srcIndex]--; // ��ָӤ�P
    }
    board[dstIndex] = srcChess; // ��m��ت�

    record.push_back((dstChess << 18) | (srcChess << 12) | action);
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
			CalZobristNumber(EAT_CHESS2INDEX[dstChess], dstIndex, board[EAT_CHESS2INDEX[dstChess]], dstChess);
            occupied[turn ^ 1] ^= dstboard; // �٭�����W���p
            bitboard[dstChess] ^= dstboard; // �٭�����
            board[EAT_CHESS2INDEX[dstChess]]--; // �q�Ӥ��Ʋ���
        }

        occupied[turn] ^= (1 << srcIndex) | dstboard; // �٭�Ӥ���W���p
        bitboard[srcChess] ^= dstboard; // �����Ӥ��ƪ��ت���m
        
        if (pro) 
			srcChess ^= PROMOTE; // ���e������
        bitboard[srcChess] ^= 1 << srcIndex; // �٭�Ӥ��ƭ즳��m
        board[srcIndex] = srcChess; // �٭�
		CalZobristNumber(dstIndex, srcIndex, srcChess, srcChess);
    }
    else { // ���e�O���J
        occupied[turn] ^= dstboard; // �������J���W����m
        bitboard[srcChess] ^= dstboard; // �������J�Ӥ�ƪ���m
        board[srcIndex]++; // ���^�Ӥ�P
		CalZobristNumber(dstIndex, srcIndex, srcChess, board[srcIndex]);
    }
    board[dstIndex] = dstChess; // �٭�ت���
}

//TODO:
int Board::Evaluate(bool turn) {
	int score = 0;
	for (int i = 0; i < 25; i++) {
		//score += CHESS_SCORE
	}
	return score;
}

int ConvertInput(int row, int col, int turn) {
	if ((int)row >= (int)'A' && (int)row <= (int)'F' && col >= '1' && col <= '5') {
		if (row == 'F')
			return 25 + turn * 5 + '5' - col;
		else
			return (int)(row - 'A') * 5 + '5' - col;
	}
	return -1;
}

Action Human_DoMove(Board &board, int turn) {
	string cmd;
	cin.clear();
	cout << "�п�J���ʫ��O (�� E5D5+) : " << endl;
	cin >> cmd;
    cin.ignore();
	if (cmd.length() != 4 && cmd.length() != 5) {
		cout << "Invalid Move : Wrong input length" << endl;
		return 0;
	}

    int srcIndex = ConvertInput(toupper(cmd[0]), cmd[1], turn);
    int dstIndex = ConvertInput(toupper(cmd[2]), cmd[3], turn);
	if (srcIndex == -1 || dstIndex == -1 || (cmd.length() == 5 && cmd[4] != '+')) {
		cout << "Invalid Move : Bad Input" << endl;
		return 0;

	}
	bool isPro = cmd.length() == 5;
    int srcChess;

    //src��m���ѬO����
    if (srcIndex < BOARD_SIZE)
		srcChess = board.board[srcIndex];
    else {
        if (!board.board[srcIndex]) {
            cout << "Invalid Move: No chess to handmove" << endl;
            return 0;
        }
        srcChess = (srcIndex > 29 ? BLACKCHESS : 0) | (srcIndex % 5 + 1);
    }

    if (srcChess >> 4 != turn) {
        cout << "Invalid Move : "<< CHESS_WORD[srcChess]<<" is not your chess" << endl;
        return 0;
    }

    if (srcIndex >= BOARD_SIZE) { // ���J
        if (isPro) {
            cout << "Invalid Move : Promotion prohobited on handmove" << endl;
            return 0;
        }
            
        if (board.board[dstIndex]) {
            cout << "Invalid Move : " << CHESS_WORD[srcChess] << " �Ӧ�m���Ѥl" << endl;
            return 0;
        }
            
        if ((srcChess & 15) == PAWN) {
            if (!Movement[srcChess][dstIndex]) {
                cout << "Invalid Move : " << CHESS_WORD[srcChess] << " ���J�Ӧ�m���ಾ��" << endl;
                return 0;
            }

            if (column_mask(dstIndex) & board.bitboard[srcChess]) {
                cout << "Invalid Move : " << CHESS_WORD[srcChess] << " �G�B" << endl;
                return 0;
            }

            /* TODO: �B�L���i�ߧY�N��>���B��(����) */
        }
    }
    else {
        //�B�z���� + ���J�W�h�@�G���న�W����
        if (isPro) {
            if ((srcChess == PAWN || srcChess == SILVER || srcChess == BISHOP || srcChess == ROOK ||
                srcChess == (PAWN | BLACKCHESS) || srcChess == (SILVER | BLACKCHESS) || srcChess == (BISHOP | BLACKCHESS) || srcChess == (ROOK | BLACKCHESS))) {
                //�O�_�b�İϤ�
                if (!((1 << dstIndex) & (srcChess < BLACKCHESS ? BLACK_CAMP : WHITE_CAMP)) &&
					!((1 << srcIndex) & (srcChess < BLACKCHESS ? BLACK_CAMP : WHITE_CAMP))) {
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
            cout << "Invalid Move :invalid "<< CHESS_WORD[srcChess]<<" move." << endl;
            return 0;
        }
    }

    return (isPro << 24) | (dstIndex << 6) | srcIndex;
}

Action AI_DoMove(Board &board, int turn) {
	cout << "AI ��Ҥ�..." << endl;
	Action action = IDAS(board, turn);
	board.DoMove(action);
    return action;
}

//Rules
bool Uchifuzume() { return true; };
bool Sennichite() { return true; };

void MoveGenerator(const Board &board, const int turn, Action *movelist, int &start) {
    U32 srcboard, dstboard, src, dst, pro;
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

                pro = (Promotable[i] && ((1 << src | 1 << dst) & (turn ? WHITE_CAMP : BLACK_CAMP))) ? PRO_MASK : 0;
                movelist[(start)++] = pro | (dst << 6) | src;
            }
        }
    }
}

void HandGenerator(const Board &board, const int turn, Action *movelist, int &start) {
    U32 srcboard = blank_board, dstboard,
        src = (turn == BLACK ? 25 : 30), dst,
        srcChess = (src < 40 ? BLACKCHESS : 0), nifu = 0;

    if (board.board[src]) { // �B
        dstboard = board.bitboard[(turn << 4) | PAWN]; // �ڤ誺�B
        while (dstboard) {
            dst = BitScan(dstboard);
            dstboard ^= 1 << dst;
            nifu |= column_mask(dst); // �G�B
        }

        /* TODO: ���B��  (�ȮɳW�w�����e�褣�ॴ�J) */
        if (turn == BLACK) nifu |= board.bitboard[KING] >> 5;
        else nifu |= board.bitboard[KING | BLACKCHESS] << 5;

        dstboard = srcboard & ~((turn ? WHITE_CAMP : BLACK_CAMP) | nifu);
        while (dstboard) {
            dst = BitScan(dstboard);
            dstboard ^= 1 << dst;
            movelist[(start)++] = (dst << 6) | src;
        }
    }

    for (int i = 1; i < 5; i++) {
        if (board.board[++src]) {
            dstboard = srcboard;
            while (dstboard) {
                dst = BitScan(dstboard);
                dstboard ^= 1 << dst;
                movelist[(start)++] = (dst << 6) | src;
            }
        }
    }
}