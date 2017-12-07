#include "board.h"
#include <Windows.h>
#ifdef WINDOWS_10
#define LINE_STRING "�X�U�X�U�X�U�X�U�X�U�X�U�X"
#else
#define LINE_STRING "�X�q�X�q�X�q�X�q�X�q�X�q�X"
#endif

Board::Board() {}

Board::~Board() {}

void Board::Initialize() {
    Initialize(
        "21 20 18 19 22"
        " 0  0  0  0 17"
        " 0  0  0  0  0"
        " 1  0  0  0  0"
        " 6  3  2  4  5");
}

/*
��J�榡: �e 25 �ӿ�J�ѤlID
[�� 10 �ӿ�J��ƭӼ� 0~2 (�i��)]
*/
void Board::Initialize(const char *s) {
    memset(occupied, BLANK, 2 * sizeof(int));
    memset(bitboard, BLANK, 32 * sizeof(int));
    memset(board, BLANK, TOTAL_BOARD_SIZE * sizeof(int));
    record.clear();
    turn = WHITE_TURN; // ����O��
    checkstate.clear();
    checking = 0;
    nonBlockable = 0;

    stringstream input(s);
    int i = 0, chess;
    for (; i < BOARD_SIZE && input >> chess; ++i) {
        if (0 < chess && chess < 32 && CHESS_WORD[chess] != "  ") {
            board[i] = chess;
            bitboard[chess] |= 1 << i;
            occupied[chess > BLACKCHESS] |= 1 << i;
        }
    }

    if (input.eof()) return;
    for (; i < TOTAL_BOARD_SIZE && input >> chess; ++i)
        if (chess == 1 || chess == 2)
            board[i] = chess;
}

bool Board::SaveBoard(const char* filename) {
    fstream file(filename, ios::out | ios::app);

    if (file) {
        for (int i = 0; i < 35; ++i)
            file << board[i] << " ";
        
        file << ".\n";
        file.close();
        return true;
    }
    return false;
}

bool Board::LoadBoard(const char* filename) {
    fstream file(filename, ios::in);

    if (file) {
        char state[128];
        file.getline(state, 128, '.');
        file.close();
        Initialize(state);
        return true;
    }
    return false;
}

static const char *rank_name[] = { " A", " B", " C", " D", " E", " F", "  ", " G", "  " };
void Board::PrintChessBoard() {
    int chess;
    int rank_count = 0;
    int board_count = 0;

    SetColor();
    puts("\n  �U 5�U 4�U 3�U 2�U 1�U");
    for (int i = 0; i < 9; i++) {
        puts(LINE_STRING);
        if (i == 5) puts("  �U 5�U 4�U 3�U 2�U 1�U\n");

        printf("%s", rank_name[rank_count]);
        for (int j = 0; j < 5; j++, board_count++) {
            printf("�U");
            if (board_count < BOARD_SIZE) chess = board[board_count];
            else if ((i == 5 && board[board_count % 5 + 25]) ||
                (i == 6 && board[board_count % 5 + 25] == 2))
                chess = BLACKCHESS | board_count % 5 + 1;
            else if (i == 7 && board[board_count % 5 + 30] ||
                (i == 8 && board[board_count % 5 + 30] == 2))
                chess = (board_count % 5 + 1);
            else chess = 0;
            // 10 = �� ; 11 = �դ��� ; 12 = �� ; 13 = �¤���
            if (chess & BLACKCHESS) {
                if (chess & PROMOTE) SetColor(13);
                else SetColor(12);
            }
            else {
                if (chess & PROMOTE) SetColor(11);
                else SetColor(10);
            }

            printf("%s", CHESS_WORD[chess]);
            SetColor();
        }
        printf("�U%s\n", rank_name[rank_count++]);
    }
    puts("");
}

bool Board::IsGameOver() {
    Action moveList[MAX_MOVE_NUM] = { 0 };
    U32 cnt = 0;
    AttackGenerator(*this, moveList, cnt);
    if (!cnt)
        MoveGenerator(*this, moveList, cnt);
    if (!cnt)
        HandGenerator(*this, moveList, cnt);

    if (!cnt) {
        PrintChessBoard();
        cout << (turn ? "White" : "Black") << " Win\n";
        return true;
    }

    return false;
}

void Board::DoMove(const Action action) {
    // 0 board[dst] board[src] dst src
    U32 srcIndex = ACTION_TO_SRCINDEX(action),
        dstIndex = ACTION_TO_DSTINDEX(action),
        srcChess = BLANK,
        dstChess = BLANK,
        pro = ACTION_TO_ISPRO(action),
        dstboard = 1 << dstIndex;

    if (srcIndex < BOARD_SIZE) { // ����
        srcChess = board[srcIndex];
        if (dstChess = board[dstIndex]) { // �Y
            occupied[turn ^ 1] ^= dstboard; // ��s�����W���p
            bitboard[dstChess] ^= dstboard; // ��s�����
            board[EatToHand[dstChess]]++; // �ର�Ӥ���
        }

        occupied[turn] ^= (1 << srcIndex) | dstboard; // ��s�Ӥ���W���p
        bitboard[srcChess] ^= 1 << srcIndex; // �����Ӥ��ƭ즳��m

        if (pro) srcChess ^= PROMOTE; // ����
        bitboard[srcChess] ^= dstboard; // ��s�Ӥ��Ʀܥت���m
        board[srcIndex] = BLANK; // �쥻�M��
    }
    else { // ���J
        srcChess = HandToChess[srcIndex];
        occupied[turn] ^= dstboard; // ���J���W����m
        bitboard[srcChess] ^= dstboard; // ���J�Ӥ�ƪ���m
        board[srcIndex]--; // ��ָӤ�P
    }
    board[dstIndex] = srcChess; // ��m��ت�
    record.push_back((dstChess << 18) | (srcChess << 12) | action);

    turn ^= 1;

    /*checkstate.push_back(nonBlockable | checking);
    checking = 0;
    nonBlockable = 0;
    U32 attackboard;
    U32 kingboard = bitboard[KING | turn << 4],
        kingpos = BitScan(kingboard);
    if (srcIndex < BOARD_SIZE) {
        attackboard = (RookMove(*this, srcIndex) | BishopMove(*this, srcIndex)) & occupied[turn ^ 1];
        while (attackboard) {
            srcIndex = BitScan(attackboard);
            if (Movable(*this, srcIndex) & kingboard) {
                checking = 1;
                if (Movement[KING][srcIndex] & kingboard) {
                    nonBlockable = 2;
                    break;
                }
            }
            attackboard ^= 1 << srcIndex;
        }
    }
    else if (Movable(*this, dstIndex) & kingboard) {
        checking = 1;
        if (Movement[KING][dstIndex] & kingboard)
            nonBlockable = 2;
    }*/
}

void Board::UndoMove() {
    if (record.size() == 0) { // �Ĺs�Ӥ����O�d�ΨӧP�_�����
        cout << "Can not undo any more!" << endl;
        return;
    }
    Action redo = record.back();
    record.pop_back();
    turn ^= 1;

    // 0 board[dst] board[src] dst src
    U32 srcIndex = ACTION_TO_SRCINDEX(redo),
        dstIndex = ACTION_TO_DSTINDEX(redo),
        srcChess = ACTION_TO_SRCCHESS(redo),
        dstChess = ACTION_TO_DSTCHESS(redo),
        pro = ACTION_TO_ISPRO(redo),
        dstboard = 1 << dstIndex;

    if (srcIndex < BOARD_SIZE) { // ���e�O����
        if (dstChess) { // ���e���Y�l
            occupied[turn ^ 1] ^= dstboard; // �٭�����W���p
            bitboard[dstChess] ^= dstboard; // �٭�����
            board[EatToHand[dstChess]]--; // �q�Ӥ��Ʋ���
        }

        occupied[turn] ^= (1 << srcIndex) | dstboard; // �٭�Ӥ���W���p
        bitboard[srcChess] ^= dstboard; // �����Ӥ��ƪ��ت���m

        if (pro) srcChess ^= PROMOTE; // ���e������
        bitboard[srcChess] ^= 1 << srcIndex; // �٭�Ӥ��ƭ즳��m
        board[srcIndex] = srcChess; // �٭�
    }
    else { // ���e�O���J
        occupied[turn] ^= dstboard; // �������J���W����m
        bitboard[srcChess] ^= dstboard; // �������J�Ӥ�ƪ���m
        board[srcIndex]++; // ���^�Ӥ�P
    }
    board[dstIndex] = dstChess; // �٭�ت���
    //checking = checkstate.back() & 1;
    //nonBlockable = checkstate.back() & 2;
    //checkstate.pop_back();
}

int ConvertInput(int row, int col, int turn) {
    if ('A' <= row && row <= 'G' && '1' <= col && col <= '5') {
        if (row == 'F')
            return 25 + '5' - col;
        else if (row == 'G')
            return 30 + '5' - col;
        else
            return (row - 'A') * 5 + '5' - col;
    }
    return -1;
}

Action Human_DoMove(Board &board) {
    string cmd;
    cin.clear();
    cout << "�п�J���ʫ��O (�� E5D5+) : " << endl;
    cin >> cmd;
    cin.ignore();
    if (cmd.length() != 4 && cmd.length() != 5) {
        cout << "Invalid Move : Wrong input length" << endl;
        return 0;
    }

    int srcIndex = ConvertInput(toupper(cmd[0]), cmd[1], board.GetTurn());
    int dstIndex = ConvertInput(toupper(cmd[2]), cmd[3], board.GetTurn());
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
        srcChess = (srcIndex < 30 ? BLACKCHESS : 0) | (srcIndex % 5 + 1);
    }

    if (srcChess >> 4 != board.GetTurn()) {
        cout << "Invalid Move : " << CHESS_WORD[srcChess] << " is not your chess" << endl;
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
            cout << "Invalid Move :invalid " << CHESS_WORD[srcChess] << " move." << endl;
            return 0;
        }
    }

    return (isPro << 24) | (dstIndex << 6) | srcIndex;
}

Action AI_DoMove(Board &board) {
    cout << "AI ��Ҥ�..." << endl;
    return IDAS(board, board.GetTurn());
}

int Board::Evaluate() {
    int i = 0, score = 0;
    for (; i < 25; i++) {
		if (board[i])
			score += CHESS_SCORE[board[i]];
    }
    for (; i < 35; i++) {
		if (board[i])
			score += HAND_SCORE[i - 25] * board[i];
    }
    return turn ? score : -score;
}

U32 RookMove(const Board &board, const int pos) {
    // upper (find LSB) ; lower (find MSB)
    U32 occupied = board.occupied[WHITE_TURN] | board.occupied[BLACK_TURN];
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
    U32 occupied = board.occupied[WHITE_TURN] | board.occupied[BLACK_TURN];
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
    const int srcChess = board.board[srcIndex];

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

bool Board::IsStillChecking(const int src, const int dst) {
    bool isStillChecking = false;
    U32 dstboard = 1 << dst, moveboard = (1 << src) | dstboard;

    /************ DoMove ************/
    if (src < BOARD_SIZE) {
        if (board[dst]) // �Y
            occupied[turn ^ 1] ^= dstboard;

        occupied[turn] ^= moveboard;
        bitboard[board[src]] ^= moveboard;
    }
    else {
        occupied[turn] ^= dstboard;
    }
    /************ DoMove ************/

    /* get the position of the checked king */
    U32 kingboard = bitboard[KING | (turn ? BLACKCHESS : 0)],
        kingpos = BitScan(kingboard);

    /* get the possible position which might attack king */
    U32 attackboard = (RookMove(*this, kingpos) | BishopMove(*this, kingpos)) & occupied[turn ^ 1];

    /* search the possible position */
    while (attackboard) {
        U32 attsrc = BitScan(attackboard);
        isStillChecking = Movable(*this, attsrc) & kingboard;
        if (isStillChecking) break;
        attackboard ^= 1 << attsrc;
    }

    /************ UnDoMove ************/
    if (src < BOARD_SIZE) {
        if (board[dst]) // �Y
            occupied[turn ^ 1] ^= dstboard;

        occupied[turn] ^= moveboard;
        bitboard[board[src]] ^= moveboard;
    }
    else {
        occupied[turn] ^= dstboard;
    }
    /************ UnDoMove ************/
    if (isStillChecking) ++nnnn;
    return isStillChecking;
}

void AttackGenerator(Board &board, Action *movelist, U32 &start) {
    U32 turn = board.GetTurn();
	U32 srcboard, dstboard, opboard = board.occupied[turn ^ 1], src, dst, pro;
    U32 kingpos = 0, WB = turn << 4;
    //if (board.IsnonBlockable())
    //    kingpos = board.bitboard[KING | WB];

	for (int i = 0; i < 10; i++) {
		srcboard = board.bitboard[AttackOrdering[i] | WB];
		while (srcboard) {
			src = BitScan(srcboard);
            srcboard ^= 1 << src;
			dstboard = Movable(board, src) & opboard;
			while (dstboard) {
                dst = turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;

                //if (kingpos && !(Movement[KING][dst] & kingpos)) continue;
                //if (board.IsChecking() && 
                if (board.IsStillChecking(src, dst)) continue;
                pro = (A_Promotable[i] && ((1 << src | 1 << dst) & ENEMYCAMP(turn))) ? PRO_MASK : 0;
                movelist[start++] = pro | (dst << 6) | src;
			}
		}
	}
}

void MoveGenerator(Board &board, Action *movelist, U32 &start) {
    U32 turn = board.GetTurn();
	U32 srcboard, dstboard, blankboard = blank_board, src, dst, pro;
    U32 kingpos = 0, WB = turn << 4;
    //if (board.IsnonBlockable())
    //    kingpos = board.bitboard[KING | WB];

	for (int i = 0; i < 10; i++) {
		srcboard = board.bitboard[MoveOrdering[i] | WB];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(board, src) & blankboard;
			while (dstboard) {
                dst = turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;

                //if (kingpos && !(Movement[KING][dst] & kingpos)) continue;
                //if (board.IsChecking() && 
                if (board.IsStillChecking(src, dst)) continue;
				pro = (M_Promotable[i] && ((1 << src | 1 << dst) & ENEMYCAMP(turn))) ? PRO_MASK : 0;
				movelist[start++] = pro | (dst << 6) | src;
			}
		}
	}
}
bool Board::IsSennichite(Action action) {
	if (record.size() < 5) return false;
	if (ACTION_TO_SRCINDEX(record[record.size() - 1]) ==
		ACTION_TO_DSTINDEX(record[record.size() - 3]) &&
		ACTION_TO_DSTINDEX(record[record.size() - 1]) ==
		ACTION_TO_SRCINDEX(record[record.size() - 3]) &&
		ACTION_TO_SRCINDEX(record[record.size() - 2]) == ACTION_TO_DSTINDEX(action) &&
		ACTION_TO_DSTINDEX(record[record.size() - 2]) == ACTION_TO_SRCINDEX(action)) {
		return true;
	}
	return false;
}
void HandGenerator(Board &board, Action *movelist, U32 &start) {
    /* TODO: �Q�άd��W�@�B�ֳt�P�_���S������A������ */
    //if (board.IsnonBlockable()) return;

    U32 turn = board.GetTurn();
    U32 srcboard = 0, dstboard = blank_board, src = (turn ? 30 : 35), dst, nifu = 0;
    U32 handcnt=0;
    for (int i = 1; i <= 5; ++i)
        if (!board.board[src - i])
            ++handcnt;
    if (handcnt == 5) return;

    while (dstboard) {
        dst = BitScan(dstboard);
        dstboard ^= 1 << dst;
        if (board.IsStillChecking(src, dst)) continue;
        srcboard |= 1 << dst;
    }
    /*if (board.IsChecking()) {
        U32 kingpos = BitScan(board.bitboard[KING | (turn << 4)]);
        U32 attackboard = BishopMove(board, kingpos),
            opboard = (board.bitboard[BISHOP | (turn ? 0 : BLACKCHESS)] |
            board.bitboard[BISHOP | PROMOTE | (turn ? 0 : BLACKCHESS)]) & attackboard;
        if (opboard) {
            srcboard = attackboard ^ opboard;
            if (slope1_lower[kingpos] & opboard)
                srcboard &= slope1_lower[kingpos];
            else if (slope1_upper[kingpos] & opboard)
                srcboard &= slope1_upper[kingpos];
            else if (slope2_lower[kingpos] & opboard)
                srcboard &= slope2_lower[kingpos];
            else
                srcboard &= slope2_upper[kingpos];
        }
        else {
            attackboard = RookMove(board, kingpos);
            opboard = (board.bitboard[ROOK | (turn ? 0 : BLACKCHESS)] |
            board.bitboard[ROOK | PROMOTE | (turn ? 0 : BLACKCHESS)]) & attackboard;
            srcboard = attackboard ^ opboard;
            if (column_lower[kingpos] & opboard)
                srcboard &= column_lower[kingpos];
            else if (column_upper[kingpos] & opboard)
                srcboard &= column_upper[kingpos];
            else if (row_lower[kingpos] & opboard)
                srcboard &= row_lower[kingpos];
            else
                srcboard &= row_upper[kingpos];
        }
    }
    else srcboard = blank_board;*/

    for (int i = 1; i < 5; i++) { // �B�H�~�����
        if (board.board[--src]) {
            dstboard = srcboard;
            while (dstboard) {
                dst = BitScan(dstboard);
                dstboard ^= 1 << dst;
                movelist[start++] = (dst << 6) | src;
            }
        }
    }

    if (board.board[--src]) { // �B
        dstboard = board.bitboard[(turn << 4) | PAWN]; // �ڤ誺�B
        while (dstboard) {
            dst = BitScan(dstboard);
            dstboard ^= 1 << dst;
            nifu |= column_mask(dst); // �G�B
        }

        /* TODO: ���B��  (�ȮɳW�w�����e�褣�ॴ�J) */
        if (turn == BLACK_TURN) nifu |= board.bitboard[KING] >> 5;
        else nifu |= board.bitboard[KING | BLACKCHESS] << 5;

        dstboard = srcboard & ~(ENEMYCAMP(turn) | nifu);
        while (dstboard) {
            dst = BitScan(dstboard);
            dstboard ^= 1 << dst;
            movelist[start++] = (dst << 6) | src;
        }
    }
}