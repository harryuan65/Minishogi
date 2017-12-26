#include "MoveGene.h"

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

void AttackGenerator(Board &board, Action *movelist, U32 &start) {
	U32 srcboard, dstboard, opboard = board.occupied[board.GetTurn() ^ 1], src, dst, pro;
	U32 kingpos = 0, WB = board.GetTurn() << 4;
	//if (board.IsnonBlockable())
	//    kingpos = board.bitboard[KING | WB];

	for (int i = 0; i < 10; i++) {
		srcboard = board.bitboard[AttackOrdering[i] | WB];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(board, src) & opboard;
			while (dstboard) {
				dst = board.GetTurn() ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;

				//if (kingpos && !(Movement[KING][dst] & kingpos)) continue;
				//if (board.IsChecking() && 
				//if (board.IsCheckAfter(src, dst)) continue;
				pro = (A_Promotable[i] && ((1 << src | 1 << dst) & ENEMYCAMP(board.GetTurn()))) ? PRO_MASK : 0;
				movelist[start++] = pro | (dst << 6) | src;
			}
		}
	}
}

void MoveGenerator(Board &board, Action *movelist, U32 &start) {
	U32 srcboard, dstboard, blankboard = blank_board, src, dst, pro;
	U32 kingpos = 0, WB = board.GetTurn() << 4;
	//if (board.IsnonBlockable())
	//    kingpos = board.bitboard[KING | WB];

	for (int i = 0; i < 10; i++) {
		srcboard = board.bitboard[MoveOrdering[i] | WB];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(board, src) & blankboard;
			while (dstboard) {
				dst = board.GetTurn() ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;

				//if (kingpos && !(Movement[KING][dst] & kingpos)) continue;
				//if (board.IsChecking() && 
				//if (board.IsCheckAfter(src, dst)) continue;
				pro = (M_Promotable[i] && ((1 << src | 1 << dst) & ENEMYCAMP(board.GetTurn()))) ? PRO_MASK : 0;
				movelist[start++] = pro | (dst << 6) | src;
			}
		}
	}
}

void HandGenerator(Board &board, Action *movelist, U32 &start) {
	/* TODO: 利用查找上一步快速判斷有沒有王手，未完成 */
	//if (board.IsnonBlockable()) return;

	U32 srcboard = 0, dstboard = blank_board, src = (board.GetTurn() ? 30 : 35), dst, nifu = 0;
	U32 handcnt = 0;
	for (int i = 1; i <= 5; ++i)
		if (!board.board[src - i])
			++handcnt;
	if (handcnt == 5) return;

	while (dstboard) {
		dst = BitScan(dstboard);
		dstboard ^= 1 << dst;
		//if (board.IsCheckAfter(src, dst)) continue;
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

	for (int i = 1; i < 5; i++) { // 步以外的手排
		if (board.board[--src]) {
			dstboard = srcboard;
			while (dstboard) {
				dst = BitScan(dstboard);
				dstboard ^= 1 << dst;
				movelist[start++] = (dst << 6) | src;
			}
		}
	}

	if (board.board[--src]) { // 步
		dstboard = board.bitboard[(board.GetTurn() << 4) | PAWN]; // 我方的步
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			nifu |= column_mask(dst); // 二步
		}

		/* TODO: 打步詰  (暫時規定王的前方不能打入) */
		if (board.GetTurn() == BLACK_TURN) nifu |= board.bitboard[KING] >> 5;
		else nifu |= board.bitboard[KING | BLACKCHESS] << 5;

		dstboard = srcboard & ~(ENEMYCAMP(board.GetTurn()) | nifu);
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			movelist[start++] = (dst << 6) | src;
		}
	}
}