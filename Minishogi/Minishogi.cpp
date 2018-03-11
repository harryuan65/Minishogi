#include "Minishogi.h"
#ifdef WINDOWS_10
#define LINE_STRING "—┼ —┼ —┼ —┼ —┼ —┼ —"
#else
#define LINE_STRING "—┼—┼—┼—┼—┼—┼—"
#endif

// AttackGene moveList order by attacker
static const int AttackerOrder[] = {
	PAWN,
	SILVER,
	GOLD,
	PAWN | PROMOTE,
	SILVER | PROMOTE,
	BISHOP,
	ROOK,
	BISHOP | PROMOTE,
	ROOK | PROMOTE,
	KING
};

// MoveGene moveList order by mover 
static const int MoverOrder[] = {
	ROOK | PROMOTE,
	BISHOP | PROMOTE,
	ROOK,
	BISHOP,
	SILVER | PROMOTE,
	PAWN | PROMOTE,
	GOLD,
	SILVER,
	PAWN,
	KING
};


void Minishogi::Initialize() {
    Initialize(
        "21 20 18 19 22"
        " 0  0  0  0 17"
        " 0  0  0  0  0"
        " 1  0  0  0  0"
        " 6  3  2  4  5");
}

void Minishogi::Initialize(const char *s) {
	memset(occupied, BLANK, 2 * sizeof(Bitboard));
	memset(bitboard, BLANK, 32 * sizeof(Bitboard));
	memset(board, BLANK, TOTAL_BOARD_SIZE * sizeof(int));
	m_turn = 0;
	m_evaluate = 0;
	m_step = 0;
	m_hashcode = 0;
	m_hashcode2 = 0;

	stringstream input(s);
	int i = 0, chess;
	for (; i < BOARD_SIZE && input >> chess; ++i) {
		if (0 < chess && chess < 32 && CHESS_WORD[chess] != "  ") {
			board[i] = chess;
			bitboard[chess] |= 1 << i;
			occupied[chess > BLACKCHESS] |= 1 << i;
			m_evaluate += CHESS_SCORE[chess];
			m_hashcode ^= Zobrist::table[i][chess];
			m_hashcode2 ^= Zobrist::table2[i][chess];
		}
	}
	if (!input.eof()) {
		for (; i < TOTAL_BOARD_SIZE && input >> chess; ++i)
			if (chess == 1 || chess == 2) {
				board[i] = chess;
				m_evaluate += HAND_SCORE[i] * chess;
				m_hashcode ^= Zobrist::table[i][chess];
				m_hashcode2 ^= Zobrist::table2[i][chess];
			}
	}
	recordZobrist[0] = m_hashcode;
	recordZobrist2[0] = m_hashcode2;
}

void Minishogi::DoMove(Action &action) {
	Bitboard dstboard = 1 << action.dstIndex;

	if (action.srcIndex < BOARD_SIZE) { // 移動
		action.srcChess = board[action.srcIndex];
		action.dstChess = board[action.dstIndex];
		if (action.dstChess) { // 吃
			occupied[m_turn ^ 1] ^= dstboard; // 更新對方場上狀況
			bitboard[action.dstChess] ^= dstboard;   // 更新對方手牌
			board[EatToHand[action.dstChess]]++;     // 轉為該方手牌
			m_evaluate += HAND_SCORE[EatToHand[action.dstChess]] - CHESS_SCORE[action.dstChess];

			m_hashcode ^= Zobrist::table[action.dstIndex][action.dstChess];
			m_hashcode ^= Zobrist::table[EatToHand[action.dstChess]][board[EatToHand[action.dstChess]]];
			m_hashcode2 ^= Zobrist::table2[action.dstIndex][action.dstChess];
			m_hashcode2 ^= Zobrist::table2[EatToHand[action.dstChess]][board[EatToHand[action.dstChess]]];
		}

		occupied[m_turn] ^= (1 << action.srcIndex) | dstboard; // 更新該方場上狀況
		bitboard[action.srcChess] ^= 1 << action.srcIndex;     // 移除該方手牌原有位置

		m_hashcode ^= Zobrist::table[action.srcIndex][action.srcChess];
		m_hashcode2 ^= Zobrist::table2[action.srcIndex][action.srcChess];
		if (action.isPro) {
			action.srcChess ^= PROMOTE;        // 升變
			m_evaluate += CHESS_SCORE[action.srcChess] - CHESS_SCORE[action.srcChess ^ PROMOTE];
		}
		bitboard[action.srcChess] ^= dstboard; // 更新該方手牌至目的位置
		board[action.srcIndex] = BLANK;        // 原本清空
	}
	else { // 打入
		m_hashcode ^= Zobrist::table[action.srcIndex][board[action.srcIndex]];
		m_hashcode2 ^= Zobrist::table2[action.srcIndex][board[action.srcIndex]];

		action.srcChess = HandToChess[action.srcIndex];
		action.dstChess = 0;
		occupied[m_turn] ^= dstboard;          // 打入場上的位置
		bitboard[action.srcChess] ^= dstboard; // 打入該手牌的位置
		board[action.srcIndex]--;              // 減少該手牌
		m_evaluate += CHESS_SCORE[action.srcChess] - HAND_SCORE[action.srcIndex];

	}
	board[action.dstIndex] = action.srcChess;  // 放置到目的
	m_hashcode ^= Zobrist::table[action.dstIndex][action.srcChess];
	m_hashcode2 ^= Zobrist::table2[action.dstIndex][action.srcChess];

	recordAction[m_step++] = action;
	recordZobrist[m_step] = m_hashcode;
	recordZobrist2[m_step] = m_hashcode2;
	m_turn ^= 1;
	if (m_hashcode == m_hashcode2)
		Observer::data[Observer::DataType::iosmorphy]++;
}

void Minishogi::UndoMove() {
	if (m_step == 0) { // 保險一點 避免出錯
		cout << "Can not undo any more!" << endl;
		return;
	}
	m_turn ^= 1;
	m_step--;
	m_hashcode = recordZobrist[m_step];
	m_hashcode2 = recordZobrist2[m_step];

	Action redo = recordAction[m_step];
	Bitboard dstboard = 1 << redo.dstIndex;

	if (redo.srcIndex < BOARD_SIZE) { // 之前是移動
		if (redo.dstChess) { // 之前有吃子
			occupied[m_turn ^ 1] ^= dstboard; // 還原對方場上狀況
			bitboard[redo.dstChess] ^= dstboard;   // 還原對方手牌
			board[EatToHand[redo.dstChess]]--;     // 從該方手牌移除

			m_evaluate += CHESS_SCORE[redo.dstChess] - HAND_SCORE[EatToHand[redo.dstChess]];
		}

		occupied[m_turn] ^= (1 << redo.srcIndex) | dstboard; // 還原該方場上狀況
		bitboard[redo.srcChess] ^= dstboard; // 移除該方手牌的目的位置

		if (redo.isPro) {
			redo.srcChess ^= PROMOTE; // 之前有升變
			m_evaluate += CHESS_SCORE[redo.srcChess] - CHESS_SCORE[redo.srcChess ^ PROMOTE];
		}
		bitboard[redo.srcChess] ^= 1 << redo.srcIndex; // 還原該方手牌原有位置
		board[redo.srcIndex] = redo.srcChess; // 還原
	}
	else { // 之前是打入
		occupied[m_turn] ^= dstboard; // 取消打入場上的位置
		bitboard[redo.srcChess] ^= dstboard; // 取消打入該手牌的位置
		board[redo.srcIndex]++; // 收回該手牌
		m_evaluate += HAND_SCORE[redo.srcIndex] - CHESS_SCORE[redo.srcChess];
	}
	board[redo.dstIndex] = redo.dstChess; // 還原目的棋
}

void Minishogi::AttackGenerator(Action *movelist, int &start) const {
	Bitboard srcboard, dstboard, opboard = occupied[m_turn ^ 1];
	int src, dst, turnBit = m_turn << 4;

	for (int i = 0; i < 10; i++) {
		srcboard = bitboard[AttackerOrder[i] | turnBit];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(src) & opboard;
			while (dstboard) {
				dst = m_turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;
				movelist[start++].Set(src, dst,
					Promotable[AttackerOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[m_turn]));
			}
		}
	}
}

void Minishogi::MoveGenerator(Action *movelist, int &start) const {
	Bitboard srcboard, dstboard, blankboard = BlankOccupied(occupied);
	int src, dst, turnBit = m_turn << 4;

	for (int i = 0; i < 10; i++) {
		srcboard = bitboard[MoverOrder[i] | turnBit];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(src) & blankboard;
			while (dstboard) {
				dst = m_turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;
				movelist[start++].Set(src, dst,
					Promotable[MoverOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[m_turn]));
			}
		}
	}
}

void Minishogi::HandGenerator(Action *movelist, int &start) {
	int src = (m_turn ? 30 : 35), dst;

	// 如果沒有任何手排 直接回傳
	if (!board[src - 1] && !board[src - 2] && !board[src - 3] && !board[src - 4] && !board[src - 5])
		return;

	Bitboard srcboard = BlankOccupied(occupied), dstboard, nifu = 0;
	for (int i = 1; i < 5; i++) { // 步以外的手排
		if (board[--src]) {
			dstboard = srcboard;
			while (dstboard) {
				dst = BitScan(dstboard);
				dstboard ^= 1 << dst;
				movelist[start++].Set(src, dst, false);
			}
		}
	}

	if (board[--src]) { // 步
		dstboard = bitboard[(m_turn << 4) | PAWN]; // 我方的步
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			nifu |= ColMask(dst); // 二步
		}

		/************ 打步詰 ************/
		Bitboard kingboard = bitboard[KING | (m_turn ? 0 : BLACKCHESS)];
		Bitboard pawnboard = m_turn ? kingboard >> 5 : kingboard << 5;

		if (pawnboard & srcboard) {
			Bitboard uchifuzume = pawnboard; // 假設有打步詰
			int kingpos = BitScan(kingboard), pawnpos = BitScan(pawnboard);

			/************ DoMove ************/
			occupied[m_turn] ^= pawnboard;
			bitboard[PAWN | (m_turn << 4)] ^= pawnboard;
			m_turn ^= 1;
			/************ DoMove ************/

			// 對方王可 吃/移動 的位置
			dstboard = Movement[KING][kingpos];
			dstboard &= dstboard ^ occupied[m_turn];
			while (dstboard) {
				dst = BitScan(dstboard);
				// 如果王移動後脫離被王手
				if (!IsCheckAfter(kingpos, dst)) {
					uchifuzume = 0; // 代表沒有打步詰
					break;
				}
				dstboard ^= 1 << dst;
			}

			// 對方可能攻擊到步的棋子 (不包括王)
			if (uchifuzume) {
				Bitboard attackboard = ((RookMovable(pawnpos) | BishopMovable(pawnpos)) &occupied[m_turn]) ^ kingboard;
				while (attackboard) {
					int attsrc = BitScan(attackboard);
					// 如果真的吃得到步 且 吃了之後不會被王手
					if ((Movable(attsrc) & pawnboard) && !IsCheckAfter(attsrc, pawnpos)) {
						uchifuzume = 0; // 代表沒有打步詰
						break;
					}
					attackboard ^= 1 << attsrc;
				}
			}

			/************ UnDoMove ************/
			m_turn ^= 1;
			occupied[m_turn] ^= pawnboard;
			bitboard[PAWN | (m_turn << 4)] ^= pawnboard;
			/************ UnDoMove ************/

			nifu |= uchifuzume;
		}
		/************ 打步詰 ************/

		dstboard = srcboard & ~(EnemyCampMask[m_turn] | nifu);
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			movelist[start++].Set(src, dst, false);
		}
	}
}

inline Bitboard Minishogi::Movable(const int srcIndex) const {
	const int srcChess = board[srcIndex];

	if ((srcChess & 7) == BISHOP) {
		if (srcChess & PROMOTE)
			return BishopMovable(srcIndex) | Movement[KING][srcIndex];
		return BishopMovable(srcIndex);
	}
	else if ((srcChess & 7) == ROOK) {
		if (srcChess & PROMOTE)
			return RookMovable(srcIndex) | Movement[KING][srcIndex];
		return RookMovable(srcIndex);
	}
	else if (srcChess & PROMOTE) {
		return Movement[GOLD | (srcChess & BLACKCHESS)][srcIndex];
	}
	return Movement[srcChess][srcIndex];
}

inline Bitboard Minishogi::RookMovable(const int srcIndex) const {
	// upper (find LSB) ; lower (find MSB)
	Bitboard totalOccupied = occupied[0] | occupied[1];
	Bitboard rank, file, upper, lower;

	// row
	upper = (totalOccupied & RowUpper[srcIndex]) | HighestPosMask;
	lower = (totalOccupied & RowLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	rank = (upper - lower) & RowMask(srcIndex);

	// column
	upper = (totalOccupied & ColUpper[srcIndex]) | HighestPosMask;
	lower = (totalOccupied & ColLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	file = (upper - lower) & ColMask(srcIndex);

	return rank | file;
}

inline Bitboard Minishogi::BishopMovable(const int srcIndex) const {
	// upper (find LSB) ; lower (find MSB)
	Bitboard totalOccupied = occupied[0] | occupied[1];
	Bitboard slope1, slope2, upper, lower;

	// slope1 "/"
	upper = (totalOccupied & Slope1Upper[srcIndex]) | HighestPosMask;
	lower = (totalOccupied & Slope1Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope1 = (upper - lower) & (Slope1Upper[srcIndex] | Slope1Lower[srcIndex]);

	// slope2 "\"
	upper = (totalOccupied & Slope2Upper[srcIndex]) | HighestPosMask;
	lower = (totalOccupied & Slope2Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope2 = (upper - lower) & (Slope2Upper[srcIndex] | Slope2Lower[srcIndex]);

	return slope1 | slope2;
}

void Minishogi::PrintChessBoard() const {
	static const char *RANK_NAME[] = { " A", " B", " C", " D", " E", " F", "  ", " G", "  " };
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int chess, rank_count = 0, board_count = 0;

	SetConsoleTextAttribute(hConsole, 7);
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
				if (chess & PROMOTE)
					SetConsoleTextAttribute(hConsole, 13);
				else
					SetConsoleTextAttribute(hConsole, 12);
			}
			else {
				if (chess & PROMOTE)
					SetConsoleTextAttribute(hConsole, 11);
				else
					SetConsoleTextAttribute(hConsole, 10);
			}

			printf("%s", CHESS_WORD[chess]);
			SetConsoleTextAttribute(hConsole, 7);
		}
		printf("｜%s\n", RANK_NAME[rank_count++]);
	}
	cout << "Zobrist : " << setw(16) << hex << GetZobristHash() << dec << "\n";
	cout << "Evaluate : " << setw(15) << GetEvaluate() << "\n";
	cout << (m_turn ? "[▼ Turn]\n" : "[△ Turn]\n") << "\n";
}

void Minishogi::PrintNoncolorBoard(ostream &os) const {
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
	os << "Evaluate : " << setw(15) << GetEvaluate() << "\n";
	os << (m_turn ? "[▼ Turn]\n" : "[△ Turn]\n") << "\n";
}

bool Minishogi::SaveBoard(string filename, string comment) const {
	string filepath = BOARD_PATH + filename;
	fstream file(filepath, ios::out | ios::app);
	if (!file) {
		CreateDirectory(CA2W(BOARD_PATH), NULL);
		file.open(filepath, ios::out | ios::app);
	}
	if (file) {
		file << "+" << comment << endl;
		PrintNoncolorBoard(file);
		file.close();
		cout << "Success Save Minishogi to " << filepath << endl;
		return true;
	}
	cout << "Fail Save Minishogi to " << filepath << endl;
	return false;
}

bool Minishogi::LoadBoard(string filename, streamoff &offset) {
	fstream file(BOARD_PATH + filename, ios::in);
	if (file) {
		char str[128], chessStr[5] = "    ";
		int index = 0;
		file.seekg(offset, ios::beg);
		file.getline(str, 128);
		if (file.eof() || str[0] == '\0') {
			cout << "Error : Fail to Load Board. It's eof.\n";
			return false;
		}
		memset(occupied, BLANK, 2 * sizeof(Bitboard));
		memset(bitboard, BLANK, 32 * sizeof(Bitboard));
		memset(board, BLANK, TOTAL_BOARD_SIZE * sizeof(int));
		if (str[0] == '+') {
			m_turn = 0; // 先手是白
		}
		else if(str[0] == '-') {
			m_turn = 1; // 先手是黑
		}
		else {
			cout << "Error : Fail to Load Board. There is a unrecognized symbol (turn).\n";
			return false;
		}
		m_evaluate = 0;
		m_step = 0;
		m_hashcode = 0;
		m_hashcode2 = 0;

		for (int i = 0; i < 5; i++) {
			file.getline(str, 128);
			for (int j = 0; j < 5; j++, index++) {
				strncpy(chessStr, str + 4 * j, 4);
				for (int chess = 0; chess < CHESSCOUNT; chess++) {
					if (strcmp(SAVE_CHESS_WORD[chess], chessStr) == 0) {
						if (chess != 0) {
							board[index] = chess;
							bitboard[chess] |= 1 << index;
							occupied[chess > BLACKCHESS] |= 1 << index;
							m_evaluate += CHESS_SCORE[chess];
							m_hashcode ^= Zobrist::table[index][chess];
							m_hashcode2 ^= Zobrist::table2[index][chess];
						}
						break;
					}
					if (chess == CHESSCOUNT - 1) {
						cout << "Error : Fail to Load Board. There is a unrecognized symbol (board).\n";
						return false;
					}
				}
			}
		}
		for (int i = 0; i < 2; i++) {
			file.getline(str, 128);
			for (int j = 0; j < 5; j++, index++) {
				int num = str[2 + 3 * j] - '0';
				if (num == 1 || num == 2) {
					board[index] = num;
					m_evaluate += HAND_SCORE[index] * num;
					m_hashcode ^= Zobrist::table[index][num];
					m_hashcode2 ^= Zobrist::table2[index][num];
				}
				else if (num != 0) {
					cout << "Error : Fail to Load Board. There is a unrecognized symbol (hand).\n";
					return false;
				}
			}
		}
		recordZobrist[0] = m_hashcode;
		recordZobrist2[0] = m_hashcode2;
		offset = file.tellg();
		file.close();
		return true;
	}
	cout << "Error : Fail to Load Board. Cannot find file.\n";
	return false;
}

bool Minishogi::SaveKifu(string filename) const {
	string filepath = KIFU_PATH + filename;
	fstream file(filepath, ios::out | ios::app);
	if (!file) {
		CreateDirectory(CA2W(KIFU_PATH), NULL);
		file.open(filepath, ios::out | ios::app);
	}
	if (file) {
		file << "Kifu hash : " << setw(8) << hex << GetKifuHash() << "\n";
		file << "Initboard : " << setw(18) << hex << recordZobrist[0] << dec << "\n";
		for (int i = 0; i < m_step; i++) {
			file << setw(2) << i << " : " << (i % 2 ? "▼" : "△");
			file << recordAction[i] << setw(18) << hex << recordZobrist[i + 1] << dec << "\n";
		}
		file.close();
		cout << "Success Save Kifu to " << filepath << endl;
		return true;
	}
	cout << "Fail Save Kifu to " << filepath << endl;
	return false;
}

bool Minishogi::IsGameOver() {
	Action moveList[TOTAL_GENE_MAX_ACTIONS];
	int cnt = 0;
	AttackGenerator(moveList, cnt);
	MoveGenerator(moveList, cnt);
	HandGenerator(moveList, cnt);
	for (int i = 0; i < cnt; i++) {
		if (!IsCheckAfter(moveList[i].srcIndex, moveList[i].dstIndex)) {
			return false;
		}
	}
	return true;
}

bool Minishogi::IsLegelAction(Action action) {
	Action moveList[TOTAL_GENE_MAX_ACTIONS];
	int cnt = 0;
	if (((board[action.srcIndex] & BLACKCHESS) >> 4) != GetTurn()) {
		return false;
	}
	else {
		AttackGenerator(moveList, cnt);
		MoveGenerator(moveList, cnt);
		HandGenerator(moveList, cnt);
		for (int i = 0; i < cnt; i++) {
			if (action == moveList[i]) {
				if (IsCheckAfter(action.srcIndex, action.dstIndex)) {
					cout << "Error : You have been checked." << endl;
					return false;
				}
				return true;
			}
		}
	}
	return false;
}

bool Minishogi::IsCheckAfter(const int srcIndex, const int dstIndex) {
	bool isStillChecking = false;
	Bitboard dstboard = 1 << dstIndex, moveboard = (1 << srcIndex) | dstboard;

	/************ DoMove ************/
	if (srcIndex < BOARD_SIZE) {
		if (board[dstIndex]) // 吃
			occupied[m_turn ^ 1] ^= dstboard;

		occupied[m_turn] ^= moveboard;
		bitboard[board[srcIndex]] ^= moveboard;
	}
	else {
		occupied[m_turn] ^= dstboard;
	}
	/************ DoMove ************/

	/* get the position of the checked king */
	Bitboard kingboard = bitboard[KING | (m_turn ? BLACKCHESS : 0)];
	int kingpos = BitScan(kingboard);

	/* get the possible position which might attack king */
	Bitboard attackboard = (RookMovable(kingpos) | BishopMovable(kingpos)) & occupied[m_turn ^ 1];

	/* search the possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		isStillChecking = Movable(attsrc) & kingboard;
		if (isStillChecking) break;
		attackboard ^= 1 << attsrc;
	}

	/************ UnDoMove ************/
	if (srcIndex < BOARD_SIZE) {
		if (board[dstIndex]) // 吃
			occupied[m_turn ^ 1] ^= dstboard;

		occupied[m_turn] ^= moveboard;
		bitboard[board[srcIndex]] ^= moveboard;
	}
	else {
		occupied[m_turn] ^= dstboard;
	}
	/************ UnDoMove ************/
	return isStillChecking;
}

bool Minishogi::IsSennichite() const {
	for (int i = m_step - 4; i >= 0; i -= 2) {
		if (recordZobrist[i] == recordZobrist[m_step]) {
			return true;
		}
	}
	return false;
}

unsigned int Minishogi::GetKifuHash() const {
	unsigned int seed = m_step;
	for (int i = 0; i < m_step; i++) {
		seed ^= recordAction[i].ToU32() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}