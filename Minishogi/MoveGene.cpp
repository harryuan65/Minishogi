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

	for (int i = 0; i < 10; i++) {
		srcboard = board.bitboard[AttackOrdering[i] | WB];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(board, src) & opboard;
			while (dstboard) {
				dst = board.GetTurn() ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;

				pro = (A_Promotable[i] && ((1 << src | 1 << dst) & ENEMYCAMP(board.GetTurn()))) ? PRO_MASK : 0;
				movelist[start++] = pro | (dst << 6) | src;
			}
		}
	}
}

void MoveGenerator(Board &board, Action *movelist, U32 &start) {
	U32 srcboard, dstboard, blankboard = blank_board, src, dst, pro;
	U32 kingpos = 0, WB = board.GetTurn() << 4;

	for (int i = 0; i < 10; i++) {
		srcboard = board.bitboard[MoveOrdering[i] | WB];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(board, src) & blankboard;
			while (dstboard) {
				dst = board.GetTurn() ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;

				pro = (M_Promotable[i] && ((1 << src | 1 << dst) & ENEMYCAMP(board.GetTurn()))) ? PRO_MASK : 0;
				movelist[start++] = pro | (dst << 6) | src;
			}
		}
	}
}

void HandGenerator(Board &board, Action *movelist, U32 &start) {
	U32 srcboard = 0, dstboard = blank_board, src = (board.GetTurn() ? 30 : 35), dst, nifu = 0;

	// �p�G�S�������� �����^��
	if (!board.board[src - 1] && !board.board[src - 2] && !board.board[src - 3] && !board.board[src - 4] && !board.board[src - 5])
		return;

	while (dstboard) {
		dst = BitScan(dstboard);
		dstboard ^= 1 << dst;
		//if (board.IsCheckAfter(src, dst)) continue;
		srcboard |= 1 << dst;
	}

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
		dstboard = board.bitboard[(board.GetTurn() << 4) | PAWN]; // �ڤ誺�B
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			nifu |= column_mask(dst); // �G�B
		}

		/************ ���B�� ************/
		U32 kingboard = board.bitboard[KING | (board.GetTurn() ? 0 : BLACKCHESS)];
		U32 pawnboard = board.GetTurn() ? kingboard >> 5 : kingboard << 5;

		if (pawnboard & srcboard) {
			U32 uchifuzume = pawnboard; // ���]�����B��
			U32 kingpos = BitScan(kingboard),
				pawnpos = BitScan(pawnboard);

			/************ DoMove ************/
			board.occupied[board.GetTurn()] ^= pawnboard;
			board.bitboard[PAWN | (board.GetTurn() << 4)] ^= pawnboard;
			board.NextTurn();
			/************ DoMove ************/

			// �����i �Y/���� ����m
			dstboard = Movement[KING][kingpos];
			dstboard &= dstboard ^ board.occupied[board.GetTurn()];
			while (dstboard) {
				dst = BitScan(dstboard);
				// �p�G�����ʫ�����Q����
				if (!board.IsCheckAfter(kingpos, dst)) {
					uchifuzume = 0; // �N��S�����B��
					break;
				}
				dstboard ^= 1 << dst;
			}

			// ���i�������B���Ѥl (���]�A��)
			if (uchifuzume) {
				U32 attackboard = ((RookMove(board, pawnpos) | BishopMove(board, pawnpos)) &board.occupied[board.GetTurn()]) ^ kingboard;
				while (attackboard) {
					U32 attsrc = BitScan(attackboard);
					// �p�G�u���Y�o��B �B �Y�F���ᤣ�|�Q����
					if ((Movable(board, attsrc) & pawnboard) && !board.IsCheckAfter(attsrc, pawnpos)) {
						uchifuzume = 0; // �N��S�����B��
						break;
					}
					attackboard ^= 1 << attsrc;
				}
			}

			/************ UnDoMove ************/
			board.NextTurn();
			board.occupied[board.GetTurn()] ^= pawnboard;
			board.bitboard[PAWN | (board.GetTurn() << 4)] ^= pawnboard;
			/************ UnDoMove ************/

			nifu |= uchifuzume;
		}
		/************ ���B�� ************/

		dstboard = srcboard & ~(ENEMYCAMP(board.GetTurn()) | nifu);
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			movelist[start++] = (dst << 6) | src;
		}
	}
}