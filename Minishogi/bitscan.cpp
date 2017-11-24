#include "bitscan.h"

int BitScan(U32 board) {// LSB
	return index32[((board ^ (board - 1)) * debrujin32) >> 27];
}

int BitScanRev(U32 board) { // MSB
	board |= board >> 1;
	board |= board >> 2;
	board |= board >> 4;
	board |= board >> 8;
	board |= board >> 16;
	return index32[(board * debrujin32) >> 27];
}