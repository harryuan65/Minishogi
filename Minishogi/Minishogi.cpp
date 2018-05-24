#include <assert.h>
#include <atlstr.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "Minishogi.h"
#include "Zobrist.h"

#ifdef WINDOWS_10
#define LINE_STRING "—┼ —┼ —┼ —┼ —┼ —┼ —"
#else
#define LINE_STRING "—┼—┼—┼—┼—┼—┼—"
#endif
using std::cout;

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

/// 輸入格式: 前25個輸入棋子ID [後10個輸入手排個數0~2(可選)]
void Minishogi::Initialize(const char *s) {
	memset(occupied, 0, COLOR_NB * sizeof(Bitboard));
	memset(bitboard, 0, CHESS_NB * sizeof(Bitboard));
	memset(board, EMPTY, SQUARE_NB * sizeof(int));
	turn = WHITE;
	ply = 0;
	evalHist[ply] = VALUE_ZERO;
	keyHist[ply] = 0;
	key2Hist[ply] = 0;

	std::stringstream input(s);
	int i = 0, chess;
	for (; i < BOARD_NB && input >> chess; ++i) {
		if (0 < chess && chess < CHESS_NB && CHESS_WORD[chess] != "  ") {
			board[i] = chess;
			bitboard[chess] |= 1 << i;
			occupied[chess > BLACKCHESS] |= 1 << i;
			evalHist[ply] += CHESS_SCORE[chess];
			keyHist[ply] ^= Zobrist::table[i][chess];
			key2Hist[ply] ^= Zobrist::table2[i][chess];
		}
	}
	if (!input.eof()) {
		for (; i < SQUARE_NB && input >> chess; ++i)
			if (chess == 1 || chess == 2) {
				board[i] = chess;
				evalHist[ply] += HAND_SCORE[i] * chess;
				keyHist[ply] ^= Zobrist::table[i][chess];
				key2Hist[ply] ^= Zobrist::table2[i][chess];
			}
	}
	checker_BB();
}

bool Minishogi::Initialize(const std::string* str) {
	int index = 0;
	memset(occupied, 0, COLOR_NB * sizeof(Bitboard));
	memset(bitboard, 0, CHESS_NB * sizeof(Bitboard));
	memset(board, EMPTY, SQUARE_NB * sizeof(int));
	if (str[index][0] == '+') {
		turn = WHITE;
	}
	else if (str[index][0] == '-') {
		turn = BLACK;
	}
	else {
		cerr << "Error : Fail to Load Board. There is a unrecognized symbol in TURN section.\n";
		return false;
	}
	ply = 0;
	evalHist[ply] = VALUE_ZERO;
	keyHist[ply] = 0;
	key2Hist[ply] = 0;

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++, index++) {
			for (int chess = 0; chess < CHESS_NB; chess++) {
				if (strcmp(SAVE_CHESS_WORD[chess], str[i+1].substr(j * 4, 4).c_str()) == 0) {
					if (chess != 0) {
						board[index] = chess;
						bitboard[chess] |= 1 << index;
						occupied[chess > BLACKCHESS] |= 1 << index;
						evalHist[ply] += CHESS_SCORE[chess];
						keyHist[ply] ^= Zobrist::table[index][chess];
						key2Hist[ply] ^= Zobrist::table2[index][chess];
					}
					break;
				}
				if (chess == CHESS_NB - 1) {
					cerr << "Error : Fail to Load Board. There is a unrecognized symbol in BOARD section.\n";
					return false;
				}
			}
		}
	}
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 5; j++, index++) {
			int num = str[i + 6][2 + 3 * j] - '0';
			if (num == 1 || num == 2) {
				board[index] = num;
				evalHist[ply] += HAND_SCORE[index] * num;
				keyHist[ply] ^= Zobrist::table[index][num];
				key2Hist[ply] ^= Zobrist::table2[index][num];
			}
			else if (num != 0) {
				cerr << "Error : Fail to Load Board. There is a unrecognized symbol in HAND section.\n";
				return false;
			}
		}
	}
	checker_BB();
	return true;
}

void Minishogi::DoMove(Move m) {
	ply++;
	assert(ply < MAX_HISTORY_PLY);
	Square from = from_sq(m);
	Square to = to_sq(m);
	bool pro = is_pro(m);
	Chess pc = GetChessOn(from);
	Chess captured = captureHist[ply] = GetChessOn(to);
	Bitboard dstboard = 1 << to;
	evalHist[ply] = evalHist[ply - 1];
	keyHist[ply] = keyHist[ply - 1];
	key2Hist[ply] = key2Hist[ply - 1];

	assert(from < SQUARE_NB);
	assert(to < BOARD_NB);
	assert(pc < CHESS_NB);
	assert(captured < CHESS_NB);
	assert(type_of(pc) != KING || !pro);

	if (from < BOARD_NB) { // 移動
		if (captured) { // 吃
			assert(type_of(captured) != KING);
			int toHand = EatToHand[captured];
			occupied[~turn] ^= dstboard; // 更新對方場上狀況
			bitboard[captured] ^= dstboard;   // 更新對方手牌
			board[toHand]++;     // 轉為該方手牌

			evalHist[ply] += HAND_SCORE[toHand] - CHESS_SCORE[captured];
			keyHist[ply] ^= Zobrist::table[from][captured]
						 ^ Zobrist::table[toHand][board[toHand]];
			key2Hist[ply] ^= Zobrist::table2[to][captured]
					      ^ Zobrist::table2[toHand][board[toHand]];
		}

		occupied[turn] ^= (1 << from) | dstboard; // 更新該方場上狀況
		bitboard[pc] ^= 1 << from;                // 移除該方手牌原有位置

		keyHist[ply] ^= Zobrist::table[from][pc];
		key2Hist[ply] ^= Zobrist::table2[from][pc];
		if (pro) { // 升變
			evalHist[ply] += CHESS_SCORE[promote(pc)] - CHESS_SCORE[pc];
			pc = promote(pc);
		}
		bitboard[pc] ^= dstboard; // 更新該方手牌至目的位置
		board[from] = EMPTY;      // 原本清空
	}
	else { // 打入
		occupied[turn] ^= dstboard;    // 打入場上的位置
		bitboard[pc] ^= dstboard;      // 打入該手牌的位置
		board[from]--;                 // 減少該手牌

		evalHist[ply] += CHESS_SCORE[pc] - HAND_SCORE[from];
		keyHist[ply] ^= Zobrist::table[from][pc];
		key2Hist[ply] ^= Zobrist::table2[from][pc];
	}
	board[to] = pc;  // 放置到目的
	assert(board[0] < CHESS_NB && board[0] >= 0);
	assert(pc < CHESS_NB);
	assert(captured < CHESS_NB);

	turn = ~turn;
	keyHist[ply] ^= Zobrist::table[to][pc];
	key2Hist[ply] ^= Zobrist::table2[to][pc];
	moveHist[ply] = m;

	checker_BB(); // 移動後 會不會造成對方處於被將狀態
}

void Minishogi::UndoMove() {
	assert(ply > 0);
	Move m = moveHist[ply];
	Square from = from_sq(m);
	Square to = to_sq(m);
	bool pro = is_pro(m);
	Chess pc = GetChessOn(to);
	Chess captured = captureHist[ply];
	Bitboard dstboard = 1 << to;
	turn = ~turn;
	ply--;

	assert(from < SQUARE_NB);
	assert(to < BOARD_NB);
	assert(pc < CHESS_NB);
	assert(captured < CHESS_NB);

	if (from < BOARD_NB) { // 之前是移動或攻擊
		if (captured) {
			occupied[~turn] ^= dstboard;      // 還原對方場上狀況
			bitboard[captured] ^= dstboard;   // 還原對方手牌
			board[EatToHand[captured]]--;     // 從該方手牌移除
		}
		occupied[turn] ^= (1 << from) | dstboard; // 還原該方場上狀況
		bitboard[pc] ^= dstboard;                 // 移除該方手牌的目的位置

		if (pro)
			pc = promote(pc);
		bitboard[pc] ^= 1 << from; // 還原該方手牌原有位置
		board[from] = pc;          // 還原棋子
	}
	else { // 之前是打入
		occupied[turn] ^= dstboard; // 取消打入場上的位置
		bitboard[pc] ^= dstboard;   // 取消打入該手牌的位置
		board[from]++;              // 手牌數量加一
	}
	board[to] = captured;           // 還原目的棋子
}

bool Minishogi::PseudoLegal(const Move m) const {
	Square from = from_sq(m), to = to_sq(m);
	// 不超出移動範圍
	if (from >= SQUARE_NB || to >= BOARD_NB)
		return false;

	Chess pc = GetChessOn(from), capture = GetChessOn(to);
	// 只能出己方的手排
	if (pc == EMPTY || color_of(pc) != turn)
		return false;

	// 如果是吃子，不能是打入，且只能吃對方
	if (capture != EMPTY && (from >= BOARD_NB || color_of(capture) == turn))
		return false;

	// 理論上吃不到王，也不能讓自己被將
	if (type_of(capture) == KING || IsCheckedAfter(m))
		return false;

	// 如果是移動(吃子)，驗證這顆棋子真的走的到
	if (from < BOARD_NB && !(Movable(from) & (1 << to)))
		return false;

	// 只允許某些棋子升變
	if (is_pro(m) && !(Promotable[pc] && ((1 << from | 1 << to) & EnemyCampMask[turn])))
		return false;

	return true;
}

ExtMove* Minishogi::AttackGenerator(ExtMove *moveList) const {
	Bitboard srcboard, dstboard, opboard = occupied[~turn], attackboard = checker_bb[ply];
	Square src, dst;
	int turnBit = turn << 4;

	if (!attackboard)
		attackboard = BitboardMask;

	for (int i = 0; i < 10; i++) {
		srcboard = bitboard[AttackerOrder[i] | turnBit];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(src) & opboard;
			if (attackboard && AttackerOrder[i] != KING) dstboard &= attackboard;
			while (dstboard) {
				dst = turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;
				*moveList++ = make_move(src, dst,
					Promotable[AttackerOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[turn]));

				if (AttackerOrder[i] == SILVER && is_pro(*(moveList - 1))) {
					*moveList++ = make_move(src, dst, 0);
				}
			}
		}
	}

	return moveList;
}

ExtMove* Minishogi::MoveGenerator(ExtMove *moveList) const {
	Bitboard srcboard, dstboard, blankboard = BlankOccupied(occupied), attackboard = checker_bb[ply];
	Square src, dst;
	int turnBit = turn << 4;

	for (int i = 0; i < 10; i++) {
		srcboard = bitboard[MoverOrder[i] | turnBit];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(src) & blankboard;
			if (attackboard && MoverOrder[i] != KING) dstboard &= attackboard;
			while (dstboard) {
				dst = turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;
				*moveList++ = make_move(src, dst,
					Promotable[MoverOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[turn]));
				
				if (MoverOrder[i] == SILVER && is_pro(*(moveList - 1))) {
					*moveList++ = make_move(src, dst, 0);
				}
			}
		}
	}
	return moveList;
}

ExtMove* Minishogi::HandGenerator(ExtMove *moveList) {
	Square src = (turn ? SQ_F1 : SQ_G1), dst;

	// 如果沒有任何手排 直接回傳
	if (!board[src] && !board[src - 1] && !board[src - 2] && !board[src - 3] && !board[src - 4])
		return moveList;

	Bitboard srcboard = BlankOccupied(occupied), dstboard, nifu = 0;
	Bitboard checker = checker_bb[ply] & occupied[~turn];
	if (checker) {                    
		if (checker & (checker - 1)) // if there are more than one checkers
			return moveList;         // no need to generate

		// if there is only one checker, only the path from checker to my king can be blocked
		srcboard &= checker_bb[ply]; 
	}

	++src;
	for (int i = 0; i < 4; i++) { // 步以外的手排
		if (board[--src]) {
			dstboard = srcboard;
			while (dstboard) {
				dst = BitScan(dstboard);
				dstboard ^= 1 << dst;
				*moveList++ = make_move(src, dst, false);
			}
		}
	}

	if (board[--src]) { // 步
		dstboard = bitboard[(turn << 4) | PAWN]; // 我方的步
		if (dstboard)
			nifu |= ColMask(BitScan(dstboard)); // 二步

		/************ 打步詰 ************/
		Bitboard kingboard = bitboard[KING | (turn ? 0 : BLACKCHESS)];
		Bitboard pawnboard = turn ? kingboard >> 5 : kingboard << 5;

		if (checker == 0 && (pawnboard & srcboard)) {
			Bitboard uchifuzume = pawnboard; // 假設有打步詰
			Square kingpos = BitScan(kingboard), pawnpos = BitScan(pawnboard);

			/************ DoMove ************/
			occupied[turn] ^= pawnboard;
			bitboard[PAWN | (turn << 4)] ^= pawnboard;
			turn = ~turn;
			/************ DoMove ************/

			// 對方王可 吃/移動 的位置
			dstboard = Movement[KING][kingpos];
			dstboard &= dstboard ^ occupied[turn];
			while (dstboard) {
				dst = BitScan(dstboard);
				// 如果王移動後脫離被王手
				if (!IsCheckedAfter(kingpos, dst)) {
					uchifuzume = 0; // 代表沒有打步詰
					break;
				}
				dstboard ^= 1 << dst;
			}

			// 對方可能攻擊到步的棋子 (不包括王)
			if (uchifuzume) {
				Bitboard attackboard = ((RookMovable(pawnpos) | BishopMovable(pawnpos)) &occupied[turn]) ^ kingboard;
				while (attackboard) {
					Square attsrc = BitScan(attackboard);
					// 如果真的吃得到步 且 吃了之後不會被王手
					if ((Movable(attsrc) & pawnboard) && !IsCheckedAfter(attsrc, pawnpos)) {
						uchifuzume = 0; // 代表沒有打步詰
						break;
					}
					attackboard ^= 1 << attsrc;
				}
			}

			/************ UnDoMove ************/
			turn = ~turn;
			occupied[turn] ^= pawnboard;
			bitboard[PAWN | (turn << 4)] ^= pawnboard;
			/************ UnDoMove ************/

			nifu |= uchifuzume;
		}
		/************ 打步詰 ************/

		dstboard = srcboard & ~(EnemyCampMask[turn] | nifu);
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			*moveList++ = make_move(src, dst, false);
		}
	}
	return moveList;
}

inline Bitboard Minishogi::Movable(const int srcIndex, const Bitboard Occupied) const {
	const int srcChess = board[srcIndex];

	if ((srcChess & 7) == BISHOP) {
		if (srcChess & PROMOTE)
			return BishopMovable(srcIndex, Occupied) | Movement[KING][srcIndex];
		return BishopMovable(srcIndex, Occupied);
	}
	else if ((srcChess & 7) == ROOK) {
		if (srcChess & PROMOTE)
			return RookMovable(srcIndex, Occupied) | Movement[KING][srcIndex];
		return RookMovable(srcIndex, Occupied);
	}
	else if (srcChess & PROMOTE) {
		return Movement[GOLD | (srcChess & BLACKCHESS)][srcIndex];
	}
	return Movement[srcChess][srcIndex];
}

inline Bitboard Minishogi::RookMovable(const int srcIndex, Bitboard Occupied) const {
	// upper (find LSB) ; lower (find MSB)
	if (!Occupied) Occupied = occupied[0] | occupied[1];
	Bitboard rank, file, upper, lower;

	// row "-"
	upper = (Occupied & RowUpper[srcIndex]) | HighestPosMask;
	lower = (Occupied & RowLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	rank = (upper - lower) & RowMask(srcIndex);

	// column "|"
	upper = (Occupied & ColUpper[srcIndex]) | HighestPosMask;
	lower = (Occupied & ColLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	file = (upper - lower) & ColMask(srcIndex);

	return rank | file;
}

inline Bitboard Minishogi::BishopMovable(const int srcIndex, Bitboard Occupied) const {
	// upper (find LSB) ; lower (find MSB)
	if (!Occupied) Occupied = occupied[0] | occupied[1];
	Bitboard slope1, slope2, upper, lower;

	// slope1 "/"
	upper = (Occupied & Slope1Upper[srcIndex]) | HighestPosMask;
	lower = (Occupied & Slope1Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope1 = (upper - lower) & (Slope1Upper[srcIndex] | Slope1Lower[srcIndex]);

	// slope2 "\"
	upper = (Occupied & Slope2Upper[srcIndex]) | HighestPosMask;
	lower = (Occupied & Slope2Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope2 = (upper - lower) & (Slope2Upper[srcIndex] | Slope2Lower[srcIndex]);

	return slope1 | slope2;
}

inline Bitboard Minishogi::attackers_to(const int dstIndex, const Bitboard occupied) const {
	return RookMovable(dstIndex, occupied) | BishopMovable(dstIndex, occupied);
}

inline void Minishogi::checker_BB() {
	checker_bb[ply] = 0;

	/* get the position of my king */
	const Bitboard kingboard = bitboard[KING | (turn << 4)];
	const int kingpos = BitScan(kingboard);

	/* get the possible position which might attack king */
	const Bitboard Occupied = occupied[0] | occupied[1];
	Bitboard upper, lower, attackboard, attack_path;

	// row "-"
	upper = (Occupied & RowUpper[kingpos]) | HighestPosMask;
	lower = (Occupied & RowLower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard rank = (upper - lower) & RowMask(kingpos);
	attackboard = rank & occupied[~turn];

	/* search the rook direction possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & rank) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}

	// column "|"
	upper = (Occupied & ColUpper[kingpos]) | HighestPosMask;
	lower = (Occupied & ColLower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard file = (upper - lower) & ColMask(kingpos);
	attackboard = file & occupied[~turn];

	/* search the rook direction possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & file) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}

	// slope1 "/"
	upper = (Occupied & Slope1Upper[kingpos]) | HighestPosMask;
	lower = (Occupied & Slope1Lower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard slope1 = (upper - lower) & (Slope1Upper[kingpos] | Slope1Lower[kingpos]);
	attackboard = slope1 & occupied[~turn];

	/* search the rook direction possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & slope1) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}

	// slope2 "\"
	upper = (Occupied & Slope2Upper[kingpos]) | HighestPosMask;
	lower = (Occupied & Slope2Lower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard slope2 = (upper - lower) & (Slope2Upper[kingpos] | Slope2Lower[kingpos]);
	attackboard = slope2 & occupied[~turn];

	/* search the rook direction possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & slope2) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}
	/*if (checker_bb[ply]) {
		PrintChessBoard();
		cout << hex << checker_bb[ply];
		system("pause");
	}*/
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
			if (board_count < BOARD_NB)
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
	cout << "Zobrist : " << std::setw(16) << std::hex << GetKey() << std::dec << "\n";
	cout << "Evaluate : " << std::setw(15) << GetEvaluate() << "\n";
	cout << (turn ? "[▼ Turn]\n" : "[△ Turn]\n") << "\n";
}

void Minishogi::PrintNoncolorBoard(std::ostream &os) const {
	os << (GetTurn() ? "-" : "+") << "\n";
	for (int i = 0; i < BOARD_NB; i++) {
		if (board[i] == EMPTY)
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
	os << "Zobrist : " << std::setw(16) << std::hex << GetKey() << std::dec << "\n";
	os << "Evaluate : " << std::setw(15) << GetEvaluate() << "\n";
	os << (turn ? "[▼ Turn]\n" : "[△ Turn]\n") << "\n";
}

/// 第一行 +代表白方先 -代表黑方先
/// 第二~六行 5*5的棋盤 旗子符號請見Chess.h的SAVE_CHESS_WORD
/// 第七~八行 ▼0步0銀0金0角0飛 △0步0銀0金0角0飛
bool Minishogi::SaveBoard(std::string filename) const {
	std::fstream file(BOARD_PATH + filename, std::ios::app);
	if (file) {
		file << (GetTurn() ? "-" : "+") << "Step : " << std::to_string(GetStep()) << "\n";
		PrintNoncolorBoard(file);
		file.close();
		cout << "Succeed to Save Board.\n";
		return true;
	} 
	cerr << "Error : Fail to Save Board.\n";
	return false;
}

bool Minishogi::LoadBoard(std::string filename, std::streamoff &offset) {
	std::fstream file(BOARD_PATH + filename, std::ios::in);
	if (file) {
		char str[128];
		std::string boardStr[8];
		file.seekg(offset, std::ios::beg);
		for (int i = 0; i < 8; i++) {
			if (file.eof()) {
				cerr << "Error : Fail to Load Board. It's eof.\n";
				file.close();
				return false;
			}
			file.getline(str, 128);
			boardStr[i] = str;
		}
		offset = file.tellg();
		file.close();
		if (!Initialize(boardStr)) {
			return false;
		}
		return true;
	}
	cerr << "Error : Fail to Load Board. Cannot find file.\n";
	return false;
}

void Minishogi::PrintKifu(std::ostream &os) const {
	os << "Kifu hash : " << std::setw(8) << std::hex << GetKifuHash() << "\n";
	os << "Initboard : " << std::setw(18) << std::hex << GetKey(0) << std::dec << "\n";
	for (int i = 0; i < ply; i++) {
		os << std::setw(2) << i << " : " << (i % 2 ? "▼" : "△");
		os << moveHist[i] << std::setw(18) << std::hex << GetKey(i + 1) << std::dec << "\n";
	}
}

Move* Minishogi::GetLegalMoves(Move* legalMoves) {
	ExtMove moveList[TOTAL_GENE_MAX_ACTIONS], *end;
	end = AttackGenerator(moveList);
	end = MoveGenerator(end);
	end = HandGenerator(end);
	for (ExtMove* i = moveList; i < end; i++) {
		if (!IsCheckedAfter(*i)) {
			*legalMoves++ = i->move;
		}
	}
	return legalMoves;
}

bool Minishogi::IsGameOver() {
	Move moveList[TOTAL_GENE_MAX_ACTIONS];
	return moveList == GetLegalMoves(moveList);
}

bool Minishogi::IsLegelAction(Move m) {
	Move moveList[TOTAL_GENE_MAX_ACTIONS], *end = GetLegalMoves(moveList);
	return end != std::find(moveList, end, m);
}


inline bool Minishogi::IsCheckedAfter(const Move m) const {
	return IsCheckedAfter(from_sq(m), to_sq(m));
}

bool Minishogi::IsCheckedAfter(const Square srcIndex, const Square dstIndex) const {
	const Bitboard dstboard = 1 << dstIndex;
    Bitboard op_occupied = occupied[~turn];
    if (board[dstIndex]) // eat
        op_occupied ^= dstboard;

    const Bitboard tmp_occupied = (occupied[turn] | op_occupied) ^ ((srcIndex < BOARD_NB ? (1 << srcIndex) : 0) | dstboard);

    /* get the position of the checked king */
    Bitboard kingboard = dstboard;
    int kingpos = dstIndex;
    if (type_of(board[srcIndex]) != KING) {
        kingboard = bitboard[KING | (turn << 4)];
        kingpos = BitScan(kingboard);
    }

	/* get the possible position which might attack king */
    Bitboard attackboard = attackers_to(kingpos, tmp_occupied) & op_occupied;

	/* search the possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		if (Movable(attsrc, tmp_occupied) & kingboard) return true;
		attackboard ^= 1 << attsrc;
	}

	return false;
}

bool Minishogi::IsCheckingAfter(const Move m) {
	const Square srcIndex = from_sq(m), dstIndex = to_sq(m);
	const int dstChess = board[dstIndex];
	board[dstIndex] = GetChessOn(srcIndex);
	const bool isCheckable = Movable(dstIndex) & bitboard[KING | ((~turn) << 4)];
	board[dstIndex] = dstChess;

	if (srcIndex >= BOARD_NB) return isCheckable;
	else if (isCheckable) return true;

	const Bitboard my_occupied = occupied[turn] ^ (1 << srcIndex);
	const Bitboard tmp_occupied = occupied[~turn] | my_occupied;

	/* get the position of the checking king */
	const int kingpos = BitScan(bitboard[KING | ((~turn) << 4)]);

	/* get my possible position which might attack the opponent king */
	Bitboard attackboard = RookMovable(kingpos, tmp_occupied) & my_occupied;

	/* search the possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		if (board[attsrc] & 7 == ROOK) return true;
		attackboard ^= 1 << attsrc;
	}

	attackboard = BishopMovable(kingpos, tmp_occupied) & my_occupied;
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		if (board[attsrc] & 7 == BISHOP) return true;
		attackboard ^= 1 << attsrc;
	}

	return false;
}

/// 如果現在盤面曾經出現過 且距離為偶數(同個人) 判定為千日手 需要先DoMove後才能判斷 在此不考慮被連將
bool Minishogi::IsSennichite() const {
	for (int i = ply - 4; i >= 0; i -= 2) {
		if (keyHist[i] == keyHist[ply]) {
			return true;
		}
	}
	return false;
}

unsigned int Minishogi::GetKifuHash() const {
	unsigned int seed = ply;
	for (int i = 0; i < ply; i++) {
		seed ^= toU32(moveHist[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}

bool Minishogi::SEE(const Move move, Value threshold) const {
	Square srcIndex = from_sq(move), dstIndex = to_sq(move);

	int balance = CHESS_SCORE[type_of(board[dstIndex])] - threshold;
	int myChessValue[8], opChessValue[8];
	int my_count = 0, op_count = 0;
	const Bitboard moveboard = srcIndex < BOARD_NB ? (1 << srcIndex) : 0;
	const Bitboard tem_board = (occupied[0] | occupied[1]) ^ moveboard;
	const Bitboard psbboard = attackers_to(dstIndex, tem_board);

	/************ Add opChessValue ************/
	Bitboard srcboard = psbboard & occupied[turn ^ 1];
	Bitboard dstboard = 1 << dstIndex;
	while (srcboard) {
		int attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (Movable(attsrc, tem_board) & dstboard)
			opChessValue[op_count++] = CHESS_SCORE[type_of(board[attsrc])];
	}
	/************ End opChessValue ************/
	if (op_count == 0) return true; // 對方都吃不到我下的位置

	/************ Add myChessValue ************/
	srcboard = psbboard & (occupied[turn] ^ moveboard);
	dstboard = 1 << dstIndex;
	while (srcboard) {
		int attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (Movable(attsrc) & dstboard)
			myChessValue[my_count++] = CHESS_SCORE[type_of(board[attsrc])];
	}
	/************ End myChessValue ************/
	if (my_count == 0) return false; // 在對方能吃到我的前提，我方都無法還擊

	/************ Sorting ************/
	std::sort(opChessValue, opChessValue + op_count, [](const int &a, const int &b) { return a < b; });
	std::sort(myChessValue, myChessValue + my_count, [](const int &a, const int &b) { return a < b; });

	balance -= CHESS_SCORE[type_of(GetChessOn(srcIndex))];
	for (int op = 0, my = 0; my < my_count;) {
		balance += opChessValue[op++];
		if (op < op_count)
			balance -= myChessValue[my++];
		else break;
	}

	return balance >= threshold;
}