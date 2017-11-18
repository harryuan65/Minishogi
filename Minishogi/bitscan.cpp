#include "bitscan.h"

const int index32[32] = {
	0, 14,  1, 15,  9, 18,  2, 12,
	16, 10, 26, 28, 19, 22,  3, 30,
	13,  8, 17, 11, 25, 27, 21, 29,
	7, 24, 20,  6, 23,  5,  4, 31
};

int BitScan(U32 *board) // LSB
{
	const U32 debrujin32 = U32(0x7ca26eb);
	U32 bitboard = *board;
	*board = bitboard & (bitboard - 1);
	return index32[((bitboard ^ (bitboard - 1)) * debrujin32) >> 27]; // 27 = 32 - 5
}

int BitScanRev(U32 board) // MSB
{
	const U32 debrujin32 = U32(0x7ca26eb);
	board |= board >> 1;
	board |= board >> 2;
	board |= board >> 4;
	board |= board >> 8;
	board |= board >> 16;
	return index32[(board * debrujin32) >> 27]; // 27 = 32 - 5
}