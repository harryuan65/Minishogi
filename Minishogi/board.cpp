﻿#include "board.h"
#ifdef WINDOWS_10
#define LINE_STRING "—┼ —┼ —┼ —┼ —┼ —┼ —"
#else
#define LINE_STRING "—┼—┼—┼—┼—┼—┼—"
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
輸入格式: 前 25 個輸入棋子ID
[後 10 個輸入手排個數 0~2 (可選)]
*/
void Board::Initialize(const char *s) {
    memset(occupied, BLANK, 2 * sizeof(int));
    memset(bitboard, BLANK, 32 * sizeof(int));
    memset(board, BLANK, TOTAL_BOARD_SIZE * sizeof(int));
    m_turn = WHITE_TURN; // 先手是白
	m_step = 0;
    m_evaluate = 0;

    stringstream input(s);
    int i = 0, chess;
    for (; i < BOARD_SIZE && input >> chess; ++i) {
        if (0 < chess && chess < 32 && CHESS_WORD[chess] != "  ") {
            board[i] = chess;
            bitboard[chess] |= 1 << i;
            occupied[chess > BLACKCHESS] |= 1 << i;
        }
    }

	if (!input.eof()) {
		for (; i < TOTAL_BOARD_SIZE && input >> chess; ++i)
			if (chess == 1 || chess == 2)
				board[i] = chess;
	}

	CalZobristNumber();
	recordZobrist[0] = GetZobristHash();
	//recordZobristWhite[m_step + 1] = m_whiteHashcode;
	//recordZobristBlack[m_step + 1] = m_blackHashcode;
}

void Board::PrintChessBoard() const{
	static const char *RANK_NAME[] = { " A", " B", " C", " D", " E", " F", "  ", " G", "  " };
    int chess;
    int rank_count = 0;
    int board_count = 0;

    SetColor();
    puts("  ｜ 5｜ 4｜ 3｜ 2｜ 1｜");
    for (int i = 0; i < 9; i++) {
        puts(LINE_STRING);
        if (i == 5) puts("  ｜ 5｜ 4｜ 3｜ 2｜ 1｜\n");

        printf("%s", RANK_NAME[rank_count]);
        for (int j = 0; j < 5; j++, board_count++) {
            printf("｜");
            if (board_count < BOARD_SIZE) 
				chess = board[board_count];
            else if ((i == 5 && board[board_count % 5 + 25]) ||
                (i == 6 && board[board_count % 5 + 25] == 2))
                chess = BLACKCHESS | board_count % 5 + 1;
            else if (i == 7 && board[board_count % 5 + 30] ||
                (i == 8 && board[board_count % 5 + 30] == 2))
                chess = (board_count % 5 + 1);
            else chess = 0;
            // 10 = 白 ; 11 = 白升變 ; 12 = 黑 ; 13 = 黑升變
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
        printf("｜%s\n", RANK_NAME[rank_count++]);
    }
	cout << "Zobrist : " << setw(16) << hex << GetZobristHash() << dec << "\n";
}

void Board::PrintNoncolorBoard(ostream &os) const {
	for (int i = 0; i < BOARD_SIZE; i++) {
		if (board[i] == BLANK)
			os << " ． ";
		else {
			os << SAVE_CHESS_WORD[board[i]];
		}
		if (i % 5 == 4)
			os << "\n";
	}
	os << "▼";
	for (int i = 0; i < 5; i++)
		os << board[i + 25] << CHESS_WORD[i + 1];
	os << "\n△";
	for (int i = 0; i < 5; i++) {
		os << board[i + 30] << CHESS_WORD[i + 1];
	}
	os << "\n";
	os << "Zobrist : " << setw(16) << hex << GetZobristHash() << dec << "\n";
}

/* 完全重新計算Zobrist */
void Board::CalZobristNumber() {
	m_whiteHashcode = 0;
	m_blackHashcode = 0;
	for (int i = 0; i < 25; i++) {
		if (board[i] != BLANK) {
			m_whiteHashcode ^= Zobrist::key[i][board[i]];
			m_blackHashcode ^= Zobrist::key[24 - i][board[i] ^ BLACKCHESS];
		}
	}
	for (int i = 25; i < 37; i++) {
		if (board[i]) {
			m_whiteHashcode ^= Zobrist::key[i][1];
			m_blackHashcode ^= Zobrist::key[i> 30 ? i - 6 : i + 6][1];
		}
		if (board[i] == 2) {
			m_whiteHashcode ^= Zobrist::key[i][2];
			m_blackHashcode ^= Zobrist::key[i> 30 ? i - 6 : i + 6][1];
		}
	}
}

/* 移動完計算Zobrist */
void Board::CalZobristNumber(int srcIndex, int dstIndex, int srcChess, int dstChess) {
	m_whiteHashcode ^= Zobrist::key[srcIndex][srcChess];
	m_whiteHashcode ^= Zobrist::key[dstIndex][dstChess];
	if (srcIndex > 24) {
		/* 打入 */
		m_blackHashcode ^= Zobrist::key[srcIndex > 30 ? srcIndex - 6 : srcIndex + 6][srcChess];
		m_blackHashcode ^= Zobrist::key[24 - dstIndex][dstChess ^ BLACKCHESS];
	}
	else if (dstIndex > 24) {
		/* 吃掉 */
		m_blackHashcode ^= Zobrist::key[24 - srcIndex][srcChess ^ BLACKCHESS];
		m_blackHashcode ^= Zobrist::key[dstIndex > 30 ? dstIndex - 6 : dstIndex + 6][dstChess];
	}
	else {
		/* 移動 */
		m_blackHashcode ^= Zobrist::key[24 - srcIndex][srcChess ^ BLACKCHESS];
		m_blackHashcode ^= Zobrist::key[24 - dstIndex][dstChess ^ BLACKCHESS];
	}
}

void Board::DoMove(const Action action) {
    // 0 board[dst] board[src] dst src
    U32 srcIndex = ACTION_TO_SRCINDEX(action),
        dstIndex = ACTION_TO_DSTINDEX(action),
        srcChess = BLANK,
        dstChess = BLANK,
        pro = ACTION_TO_ISPRO(action),
        dstboard = 1 << dstIndex;

    if (srcIndex < BOARD_SIZE) { // 移動
        srcChess = board[srcIndex];
        if (dstChess = board[dstIndex]) { // 吃
            occupied[m_turn ^ 1] ^= dstboard; // 更新對方場上狀況
            bitboard[dstChess] ^= dstboard; // 更新對方手牌
            board[EatToHand[dstChess]]++; // 轉為該方手牌
            m_evaluate += HAND_SCORE[EatToHand[dstChess] - 25] - CHESS_SCORE[dstChess];
			CalZobristNumber(dstIndex, EatToHand[dstChess], dstChess, board[EatToHand[dstChess]]);
        }

        occupied[m_turn] ^= (1 << srcIndex) | dstboard; // 更新該方場上狀況
        bitboard[srcChess] ^= 1 << srcIndex; // 移除該方手牌原有位置

        if (pro) {
            srcChess ^= PROMOTE; // 升變
            m_evaluate += CHESS_SCORE[srcChess] - CHESS_SCORE[srcChess ^ PROMOTE];
        }
        bitboard[srcChess] ^= dstboard; // 更新該方手牌至目的位置
        board[srcIndex] = BLANK; // 原本清空
		CalZobristNumber(srcIndex, dstIndex, pro ? srcChess ^ PROMOTE : srcChess, srcChess);
    }
    else { // 打入
        srcChess = HandToChess[srcIndex];
		CalZobristNumber(srcIndex, dstIndex, board[srcIndex], srcChess);
        occupied[m_turn] ^= dstboard; // 打入場上的位置
        bitboard[srcChess] ^= dstboard; // 打入該手牌的位置
        board[srcIndex]--; // 減少該手牌
        m_evaluate += CHESS_SCORE[srcChess] - HAND_SCORE[srcIndex - 25];
    }
    board[dstIndex] = srcChess; // 放置到目的
	recordAction[m_step] = (dstChess << 18) | (srcChess << 12) | action;
	recordZobrist[m_step + 1] = GetZobristHash();
	//recordZobristWhite[m_step + 1] = m_whiteHashcode;
	//recordZobristBlack[m_step + 1] = m_blackHashcode;

    m_turn ^= 1;
	m_step++;
}

void Board::UndoMove() {
    if (m_step == 0) { // 第零個元素保留用來判斷先後手
        cout << "Can not undo any more!" << endl;
        return;
    }
    Action redo = recordAction[m_step - 1];

	m_turn ^= 1;
	m_step--;
	m_whiteHashcode = m_turn ? recordZobrist[m_step] >> 32 : recordZobrist[m_step];
	m_blackHashcode = m_turn ? recordZobrist[m_step] : recordZobrist[m_step] >> 32;
	//m_whiteHashcode = recordZobristWhite[m_step];
	//m_blackHashcode = recordZobristBlack[m_step];

    // 0 board[dst] board[src] dst src
    U32 srcIndex = ACTION_TO_SRCINDEX(redo),
        dstIndex = ACTION_TO_DSTINDEX(redo),
        srcChess = ACTION_TO_SRCCHESS(redo),
        dstChess = ACTION_TO_DSTCHESS(redo),
        pro = ACTION_TO_ISPRO(redo),
        dstboard = 1 << dstIndex;

    if (srcIndex < BOARD_SIZE) { // 之前是移動
        if (dstChess) { // 之前有吃子
			occupied[m_turn ^ 1] ^= dstboard; // 還原對方場上狀況
            bitboard[dstChess] ^= dstboard; // 還原對方手牌
            board[EatToHand[dstChess]]--; // 從該方手牌移除

            m_evaluate += CHESS_SCORE[dstChess] - HAND_SCORE[EatToHand[dstChess] - 25];
        }

        occupied[m_turn] ^= (1 << srcIndex) | dstboard; // 還原該方場上狀況
        bitboard[srcChess] ^= dstboard; // 移除該方手牌的目的位置

        if (pro) {
            srcChess ^= PROMOTE; // 之前有升變
            m_evaluate += CHESS_SCORE[srcChess] - CHESS_SCORE[srcChess ^ PROMOTE];
        }
        bitboard[srcChess] ^= 1 << srcIndex; // 還原該方手牌原有位置
        board[srcIndex] = srcChess; // 還原
	}
    else { // 之前是打入
        occupied[m_turn] ^= dstboard; // 取消打入場上的位置
        bitboard[srcChess] ^= dstboard; // 取消打入該手牌的位置
        board[srcIndex]++; // 收回該手牌
        m_evaluate += HAND_SCORE[srcIndex - 25] - CHESS_SCORE[srcChess];
    }
    board[dstIndex] = dstChess; // 還原目的棋
}

bool Board::SaveBoard(const string filename, const string comment) const {
	string filepath = BOARD_PATH + filename;
	fstream file(filepath, ios::out | ios::app);
	if (!file) {
		CreateDirectory(LBOARD_PATH, NULL);
		file.open(filepath, ios::out | ios::app);
	}
	if (file) {
		file << "+" << comment << endl;
		PrintNoncolorBoard(file);
		file.close();
		cout << "Success Save Board to " << filepath << endl;
		return true;
	}
	cout << "Fail Save Board to " << filepath << endl;
	return false;
}

bool Board::LoadBoard(const string filename, streamoff &offset) {
	string filepath = BOARD_PATH + filename;
	fstream file(filepath, ios::in);
	if (file) {
		stringstream ss;
		char str[128];
		file.seekg(offset, ios::beg);
		file.getline(str, 128);
		if (file.eof() || str[0] == '\0') {
			cout << "Fail Load Board from " << filepath << " It's eof.\n";
			return false;
		}
		if (str[0] != '+') {
			cout << "Fail Load Board from " << filepath << " It's error symbol.\n";
			return false;
		}

		for (int i = 0; i < 5; i++) {
			char chessStr[5] = "    ";
			file.getline(str, 128);
			for (int j = 0; j < 5; j++) {
				strncpy(chessStr, str + 4 * j, 4);
				for (int i = 0; i < 30; i++) {
					if (strcmp(SAVE_CHESS_WORD[i], chessStr) == 0) {
						ss << i << " ";
						break;
					}
				}
			}
		}
		file.getline(str, 128);
		for (int i = 0; i < 5; i++) ss << str[2 + 3 * i] - '0' << " ";
		file.getline(str, 128);
		for (int i = 0; i < 5; i++) ss << str[2 + 3 * i] - '0' << " ";
		
		offset = file.tellg();
		file.close();

		Initialize(ss.str().c_str());
		cout << "Success Load Board from " << filepath << endl;
		return true;
	}
	cout << "Fail Load Board from " << filepath << endl;
	return 0;
}

bool Board::SaveKifu(string filename, const string comment) const {
	string filepath = KIFU_PATH + filename;
	fstream file(filepath, ios::out);
	if (!file) {
		CreateDirectory(LKIFU_PATH, NULL);
		file.open(filepath, ios::out);
	}
	if (file) {
		file << "*" << comment << endl;
		file << "Zobrist Table Seed : " << Zobrist::SEED << "\n";
		file << "Kifu hash : " << setw(8) << hex << GetKifuHash() << "\n";
		file << "Initboard : " << setw(16) << hex << recordZobrist[0] << dec << "\n";
		for (int i = 0; i < m_step; i++) {
			file << setw(2) << i << " : " << (i % 2 ? "▼" : "△");
			file << Index2Input(ACTION_TO_SRCINDEX(recordAction[i]));
			file << Index2Input(ACTION_TO_DSTINDEX(recordAction[i]));
			file << (ACTION_TO_ISPRO(recordAction[i]) ? "+" : " ");
			file << setw(18) << hex << recordZobrist[i + 1] << dec << "\n";
		}
		file.close();
		cout << "Success Save Kifu to " << filepath << endl;
		return true;
	}
	cout << "Fail Save Kifu to " << filepath << endl;
	return false;
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
		cout << (m_turn ? "White" : "Black") << " Win\n";
		return true;
	}

	return false;
}

// 如果現在盤面曾經出現過 且距離為偶數(代表輪到同個人) 判定為千日手
// 需要先DoMove後才能判斷 在此不考慮被連將
bool Board::IsSennichite() const {
	for (int i = m_step - 4; i >= 0; i -= 2) {
		if (recordZobrist[i] == recordZobrist[m_step]) {
		//if (recordZobristWhite[i] == recordZobristWhite[m_step] && recordZobristBlack[i] == recordZobristBlack[m_step]) {
			return true;
		}
	}
	return false;
}

bool Board::IsCheckAfter(const int src, const int dst) {
	bool isStillChecking = false;
	U32 dstboard = 1 << dst, moveboard = (1 << src) | dstboard;

	/************ DoMove ************/
	if (src < BOARD_SIZE) {
		if (board[dst]) // 吃
			occupied[m_turn ^ 1] ^= dstboard;

		occupied[m_turn] ^= moveboard;
		bitboard[board[src]] ^= moveboard;
	}
	else {
		occupied[m_turn] ^= dstboard;
	}
	/************ DoMove ************/

	/* get the position of the checked king */
	U32 kingboard = bitboard[KING | (m_turn ? BLACKCHESS : 0)],
		kingpos = BitScan(kingboard);

	/* get the possible position which might attack king */
	U32 attackboard = (RookMove(*this, kingpos) | BishopMove(*this, kingpos)) & occupied[m_turn ^ 1];

	/* search the possible position */
	while (attackboard) {
		U32 attsrc = BitScan(attackboard);
		isStillChecking = Movable(*this, attsrc) & kingboard;
		if (isStillChecking) break;
		attackboard ^= 1 << attsrc;
	}

	/************ UnDoMove ************/
	if (src < BOARD_SIZE) {
		if (board[dst]) // 吃
			occupied[m_turn ^ 1] ^= dstboard;

		occupied[m_turn] ^= moveboard;
		bitboard[board[src]] ^= moveboard;
	}
	else {
		occupied[m_turn] ^= dstboard;
	}
	/************ UnDoMove ************/
	if (isStillChecking) {
		Observer::cutIllgalBranch++;
	}
	return isStillChecking;
}

unsigned int Board::GetKifuHash() const {
	unsigned int seed = m_step;
	for (int i = 0; i < m_step; i++) {
		seed ^= recordAction[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}