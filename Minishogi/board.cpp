#include "board.h"
#ifdef WINDOWS_10
#define LINE_STRING "—｜—｜—｜—｜—｜—｜—"
#else
#define LINE_STRING "—┼—┼—┼—┼—┼—┼—"
#endif
//cout << srcIndex << " " << dstIndex << " " << srcChess << " " << dstChess << endl; //For Debug

vector<U32> Board::ZOBRIST_TABLE[37];

Board::Board() { 
	/* Make Zobrsit Table */
	if (ZOBRIST_TABLE[0].size() == 0) {
		srand(14);
		for (int i = 0; i < 25; i++) {
			ZOBRIST_TABLE[i] = vector<U32>(30);
		}
		for (int i = 25; i < 37; i++) {
			ZOBRIST_TABLE[i] = vector<U32>(3);
		}
		for (int i = 0; i < 37; i++)
			for (int j = 0; j < ZOBRIST_TABLE[i].size(); j++)
				ZOBRIST_TABLE[i][j] = rand() | rand() << 16;
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
	for (int i = 25; i < 37; i++) {
		if (board[i]) {
			m_whiteHashcode ^= ZOBRIST_TABLE[i][1];
			m_blackHashcode ^= ZOBRIST_TABLE[i> 30 ? i - 6 : i + 6][1];
		}
		if (board[i] == 2) {
			m_whiteHashcode ^= ZOBRIST_TABLE[i][2];
			m_blackHashcode ^= ZOBRIST_TABLE[i> 30 ? i - 6 : i + 6][1];
		}
	}
}

void Board::CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess) {
	m_whiteHashcode ^= ZOBRIST_TABLE[srcIndex][srcChess];
	m_whiteHashcode ^= ZOBRIST_TABLE[dstIndex][dstChess];
	if (srcIndex > 24) {
		m_blackHashcode ^= ZOBRIST_TABLE[srcIndex > 30 ? srcIndex - 6 : srcIndex + 6][srcChess];
		m_blackHashcode ^= ZOBRIST_TABLE[24 - dstIndex][dstChess ^ BLACKCHESS];
	}
	else if (dstIndex > 24) {
		m_blackHashcode ^= ZOBRIST_TABLE[24 - srcIndex][srcChess ^ BLACKCHESS];
		m_blackHashcode ^= ZOBRIST_TABLE[dstIndex > 30 ? dstIndex - 6 : dstIndex + 6][dstChess];
	}
	else {
		m_blackHashcode ^= ZOBRIST_TABLE[24 - srcIndex][srcChess ^ BLACKCHESS];
		m_blackHashcode ^= ZOBRIST_TABLE[24 - dstIndex][dstChess ^ BLACKCHESS];
	}
}

bool Board::Initialize() {
    memset(bitboard, BLANK, 32 * sizeof(int));
    memset(board, BLANK, TOTAL_BOARD_SIZE * sizeof(int));
    record.clear();

    occupied[WHITE_TURN] = WHITE_INIT;
    occupied[BLACK_TURN] = BLACK_INIT;

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
	record.clear();
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
	CalZobristNumber();
    return true;
}

void Board::PrintChessBoard(bool turn) const {
	cout << "-----It's player " << turn << " turn!-----" << endl;
	/* Print Other Hand */
	cout << "  ｜";
	for (int i = 0; i < 5; i++)
		cout << (board[31 + i] == 2 ? CHESS_WORD[1 + i] : "  ") << "｜";
	cout << endl << (turn ? " F｜" : "  ｜");
	for (int i = 0; i < 5; i++)
		cout << (board[31 + i] ? CHESS_WORD[1 + i] : "  ") << "｜";
	cout << endl;
	/* Print Board */
	cout << LINE_STRING << endl;
	cout << "  ｜ 5｜ 4｜ 3｜ 2｜ 1｜" << endl;
	cout << LINE_STRING << endl;
	for (int i = 0; i < 5; i++) {
		cout << "  ｜";
		for (int j = 0; j < 5; j++)
			if (board[i * 5 + j] != BLANK && !(board[i * 5 + j] & BLACKCHESS))
				cout << "︿｜";
			else
				cout << CHESS_WORD[board[i * 5 + j]] << "｜";
		cout << endl << " " << (char)('A' + i) << "｜";
		for (int j = 0; j < 5; j++)
			if (board[i * 5 + j] & BLACKCHESS)
				cout << "﹀｜";
			else
				cout << CHESS_WORD[board[i * 5 + j]] << "｜";
		cout << (char)('A' + i) << endl << LINE_STRING << endl;
	}
	cout << "  ｜ 5｜ 4｜ 3｜ 2｜ 1｜" << endl;
	cout << LINE_STRING << endl;
	/* Print My Hand */
	cout << (turn ? "  ｜" : " F｜");
	for (int i = 0; i < 5; i++)
		cout << (board[25 + i] ? CHESS_WORD[1 + i] : "  ") << "｜";
	cout << endl << "  ｜";
	for (int i = 0; i < 5; i++)
		cout << (board[25 + i] == 2 ? CHESS_WORD[1 + i] : "  ") << "｜";
	cout << endl;

	/* Zobrist Hashcode */
	cout << "White Hashcode : " << GetHashcode(WHITE_TURN) << endl;
	cout << "Black Hashcode : " << GetHashcode(BLACK_TURN) << endl;

	/* Evaluate */
	cout << "Evaluate : " << GetEvaluate(turn) << endl;
}

//TODO : 改成IsCheckMate 很難做
bool Board::IsGameOver() {
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
    // 0 board[dst] board[src] dst src
	int srcIndex = ACTION_TO_SRCINDEX(action),
		dstIndex = ACTION_TO_DSTINDEX(action),
		srcChess = BLANK,
		dstChess = BLANK;
    U32 dstboard = 1 << dstIndex;
	bool isPro = ACTION_TO_ISPRO(action);

    if (srcIndex < BOARD_SIZE) { // 移動
        srcChess = board[srcIndex];
		dstChess = board[dstIndex];
        if (dstChess) { // 吃
            occupied[srcChess < BLACKCHESS] ^= dstboard; // 更新對方場上狀況
            bitboard[dstChess] ^= dstboard; // 更新對方手排
            board[EATCHESS_TO_INDEX[dstChess]]++; // 轉為該方手排
			CalZobristNumber(dstIndex, EATCHESS_TO_INDEX[dstChess], dstChess, board[EATCHESS_TO_INDEX[dstChess]]);
        }

        occupied[srcChess > BLACKCHESS] ^= (1 << srcIndex) | dstboard; // 更新該方場上狀況
        bitboard[srcChess] ^= 1 << srcIndex; // 移除該方手排原有位置

        if (isPro) 
			srcChess ^= PROMOTE; // 升變
        bitboard[srcChess] ^= dstboard; // 更新該方手排至目的位置
        board[srcIndex] = BLANK; // 原本清空
		CalZobristNumber(srcIndex, dstIndex, isPro ? srcChess^PROMOTE : srcChess, srcChess);
    }
    else { // 打入
		srcChess = srcIndex > 30 ? srcIndex - 14 : srcIndex - 24;
		CalZobristNumber(srcIndex, dstIndex, board[srcIndex], srcChess);
        occupied[(srcChess > BLACKCHESS)] ^= dstboard; // 打入場上的位置
        bitboard[srcChess] ^= dstboard; // 打入該手排的位置
        board[srcIndex]--; // 減少該手牌
    }
    board[dstIndex] = srcChess; // 放置到目的

	//cout << srcIndex << " " << dstIndex << " " << srcChess << " " << dstChess << endl;
    record.push_back((dstChess << 18) | (srcChess << 12) | action);
}

void Board::UndoMove() {
	Action redo = record.back();
    record.pop_back();
    // 0 board[dst] board[src] dst src
	int srcIndex = ACTION_TO_SRCINDEX(redo),
		dstIndex = ACTION_TO_DSTINDEX(redo),
		srcChess = ACTION_TO_SRCCHESS(redo),
		dstChess = ACTION_TO_DSTCHESS(redo);
	U32 dstboard = 1 << dstIndex;
	bool isPro = redo >> 24, turn = srcChess & BLACKCHESS;

    if (srcIndex < BOARD_SIZE) { // 之前是移動
        if (dstChess) { // 之前有吃子
			CalZobristNumber(EATCHESS_TO_INDEX[dstChess], dstIndex, board[EATCHESS_TO_INDEX[dstChess]], dstChess);
            occupied[turn ^ 1] ^= dstboard; // 還原對方場上狀況
            bitboard[dstChess] ^= dstboard; // 還原對方手排
            board[EATCHESS_TO_INDEX[dstChess]]--; // 從該方手排移除
        }

        occupied[turn] ^= (1 << srcIndex) | dstboard; // 還原該方場上狀況
        bitboard[srcChess] ^= dstboard; // 移除該方手排的目的位置
        
		if (isPro)
			srcChess ^= PROMOTE; // 之前有升變
        bitboard[srcChess] ^= 1 << srcIndex; // 還原該方手排原有位置
        board[srcIndex] = srcChess; // 還原
		CalZobristNumber(dstIndex, srcIndex, isPro ? srcChess^PROMOTE : srcChess, srcChess);
    }
    else { // 之前是打入
        occupied[turn] ^= dstboard; // 取消打入場上的位置
        bitboard[srcChess] ^= dstboard; // 取消打入該手排的位置
        board[srcIndex]++; // 收回該手牌
		CalZobristNumber(dstIndex, srcIndex, srcChess, board[srcIndex]);
    }
    board[dstIndex] = dstChess; // 還原目的棋
}

int Board::GetEvaluate(bool turn) const {
	int score = 0;
	for (int i = 0; i < 25; i++) {
		score += CHESS_SCORE[board[i]];
	}
	for (int i = 25; i < 37; i++) {
		score += HAND_SCORE[i - 25] * board[i];
	}
	return turn ? -score : score;
}

//TODO : 仍然無法避免全部的循環
bool Board::IsSennichite(Action action) {
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

int ConvertInput(int row, int col, int turn) {
	if ((int)row >= (int)'A' && (int)row <= (int)'F' && col >= '1' && col <= '5') {
		if (row == 'F')
			return 25 + turn * 6 + '5' - col;
		else
			return (int)(row - 'A') * 5 + '5' - col;
	}
	return -1;
}

Action Human_DoMove(Board &board, int turn) {
	string cmd;
	cin.clear();
	cout << "請輸入移動指令 (例 E5D5+) : " << endl;
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

    //src位置的棋是什麼
    if (srcIndex < BOARD_SIZE)
		srcChess = board.board[srcIndex];
    else {
        if (!board.board[srcIndex]) {
            cout << "Invalid Move: No chess to handmove" << endl;
            return 0;
        }
        srcChess = (srcIndex > 30 ? BLACKCHESS : 0) | (srcIndex % 5 + 1);
    }

    if (srcChess >> 4 != turn) {
        cout << "Invalid Move : "<< CHESS_WORD[srcChess]<<" is not your chess" << endl;
        return 0;
    }

    if (srcIndex >= BOARD_SIZE) { // 打入
        if (isPro) {
            cout << "Invalid Move : Promotion prohobited on handmove" << endl;
            return 0;
        }
            
        if (board.board[dstIndex]) {
            cout << "Invalid Move : " << CHESS_WORD[srcChess] << " 該位置有棋子" << endl;
            return 0;
        }
            
        if ((srcChess & 15) == PAWN) {
            if (!Movement[srcChess][dstIndex]) {
                cout << "Invalid Move : " << CHESS_WORD[srcChess] << " 打入該位置不能移動" << endl;
                return 0;
            }

            if (column_mask(dstIndex) & board.bitboard[srcChess]) {
                cout << "Invalid Move : " << CHESS_WORD[srcChess] << " 二步" << endl;
                return 0;
            }

            /* TODO: 步兵不可立即將死>打步詰(未做) */
        }
    }
    else {
        //處理升變 + 打入規則一：不能馬上升變
        if (isPro) {
            if ((srcChess == PAWN || srcChess == SILVER || srcChess == BISHOP || srcChess == ROOK ||
                srcChess == (PAWN | BLACKCHESS) || srcChess == (SILVER | BLACKCHESS) || srcChess == (BISHOP | BLACKCHESS) || srcChess == (ROOK | BLACKCHESS))) {
                //是否在敵區內
                if (!((1 << dstIndex) & (srcChess < BLACKCHESS ? BLACK_CAMP : WHITE_CAMP)) &&
					!((1 << srcIndex) & (srcChess < BLACKCHESS ? BLACK_CAMP : WHITE_CAMP))) {
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
        if (!(Movable(board, srcIndex) & (1 << dstIndex))) {
            cout << "Invalid Move :invalid "<< CHESS_WORD[srcChess]<<" move." << endl;
            return 0;
        }
    }

    return (isPro << 24) | (dstIndex << 6) | srcIndex;
}

Action AI_DoMove(Board &board, int turn) {
	cout << "AI 思考中..." << endl;
	Action action = IDAS(board, turn);
	return action;
}


/* Move Rules */
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

bool Uchifuzume() { return true; };

/* Move Gene */
void MoveGenerator(const Board &board, const int turn, Action *movelist, int &start) {
    U32 srcboard, dstboard, src, dst, pro;
    for (int i = 0; i < 10; i++) {
        srcboard = board.bitboard[MoveOrdering[i] | (turn << 4)];
        while (srcboard) {
            src = BitScan(srcboard);
            srcboard ^= 1 << src;
            dstboard = Movable(board, src); // 一顆棋所有的走步範圍
            dstboard &= (dstboard ^ board.occupied[turn]); // 把我方排除掉
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
        src = (turn == BLACK_TURN ? 31 : 25), dst,
        srcChess = (src < 40 ? BLACKCHESS : 0), nifu = 0;

    if (board.board[src]) { // 步
        dstboard = board.bitboard[(turn << 4) | PAWN]; // 我方的步
        while (dstboard) {
            dst = BitScan(dstboard);
            dstboard ^= 1 << dst;
            nifu |= column_mask(dst); // 二步
        }

        /* TODO: 打步詰  (暫時規定王的前方不能打入) */
        if (turn == BLACK_TURN) nifu |= board.bitboard[KING] >> 5;
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