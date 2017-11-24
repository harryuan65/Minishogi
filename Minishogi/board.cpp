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
static const char *showchess[] = {
    "  ", "�B", "��", "��", "��", "��", "��", "  ",
    "  ", "�w", "��", "  ", "��", "�s", "  ", "  ",
    "  ", "�B", "��", "��", "��", "��", "��", "  ",
    "  ", "�w", "��", "  ", "��", "�s",
};

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
        if (showchess[chess] == "  ")chess = BLANK;
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

static const char *rank_name[] = { "��", "��", "��", "��", "��", "��", "  ", "��", "  " };
void Board::PrintChessBoard() {
    int chess;
    int rank_count = 0;
    int board_count = 0;

    SetColor();
    puts("\n  �U���U���U���U���U���U");
    for (int i = 0; i < 9; i++) {
        puts("�X�U�X�U�X�U�X�U�X�U�X�U�X");
        if (i == 5) puts("  �U���U���U���U���U���U\n");

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

            printf("%s", showchess[chess]);
            SetColor();
        }
        printf("�U%s\n", rank_name[rank_count++]);
    }
    puts("");
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
        if (dstChess = board[dstIndex]) { // �Y
            occupied[srcChess < BLACKCHESS] ^= dstboard; // ��s�����W���p
            bitboard[dstChess] ^= dstboard; // ��s�����
            board[EatToHand[dstChess]]++; // �ର�Ӥ���
        }

        occupied[srcChess > BLACKCHESS] ^= (1 << srcIndex) | dstboard; // ��s�Ӥ���W���p
        bitboard[srcChess] ^= 1 << srcIndex; // �����Ӥ��ƭ즳��m

        if (pro) srcChess ^= PROMOTE; // ����
        bitboard[srcChess] ^= dstboard; // ��s�Ӥ��Ʀܥت���m
        board[srcIndex] = BLANK; // �쥻�M��
    }
    else { // ���J
        srcChess = HandToChess[srcIndex];
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

    //src��m���ѬO����
    if (srcIndex < BOARD_SIZE) srcChess = board.board[srcIndex];
    else {
        if (!board.board[srcIndex]) {
            cout << "Invalid Move: No chess to handmove" << endl;
            return 0;
        }
        srcChess = (srcIndex < 30 ? BLACKCHESS : 0) | (srcIndex % 5 + 1);
    }

    if (srcChess >> 4 != turn) {
        cout << "Invalid Move: "<<showchess[srcChess]<<" is not your chess" << endl;
        return 0;
    }

    if (srcIndex >= BOARD_SIZE) { // ���J
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
                    (srcChess < BLACKCHESS ? BLACK_CAMP : WHITE_CAMP))) {
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

    return (pro << 24) | (dstIndex << 6) | srcIndex;
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