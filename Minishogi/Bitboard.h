#ifndef _BITBOARD_H_
#define _BITBOARD_H_
#include <intrin.h>
#include "Types.h"

typedef unsigned __int32 Bitboard;

/*    Piece Move Area    */
const Bitboard Movement[][BOARD_NB] = {
	/* 0 */{ 0 },
	/* 1 W_PAWN */
	{
		0x0000000,     0x0000000,     0x0000000,     0x0000000,    0x0000000,
		0x0000001,     0x0000002,     0x0000004,     0x0000008,    0x0000010,
		0x0000020,     0x0000040,     0x0000080,     0x0000100,    0x0000200,
		0x0000400,     0x0000800,     0x0001000,     0x0002000,    0x0004000,
		0x0008000,     0x0010000,     0x0020000,     0x0040000,    0x0080000
	},
	/* 2 W_SILVER */
	{
		0x0000040,     0x00000a0,     0x0000140,     0x0000280,    0x0000100,
		0x0000803,     0x0001407,     0x000280e,     0x000501c,    0x0002018,
		0x0010060,     0x00280e0,     0x00501c0,     0x00a0380,    0x0040300,
		0x0200c00,     0x0501c00,     0x0a03800,     0x1407000,    0x0806000,
		0x0018000,     0x0038000,     0x0070000,     0x00e0000,    0x00c0000
	},
	/* 3 W_GOLD */
	{
		0x0000022,     0x0000045,     0x000008a,     0x0000114,    0x0000208,
		0x0000443,     0x00008a7,     0x000114e,     0x000229c,    0x0004118,
		0x0008860,     0x00114e0,     0x00229c0,     0x0045380,    0x0082300,
		0x0110c00,     0x0229c00,     0x0453800,     0x08a7000,    0x1046000,
		0x0218000,     0x0538000,     0x0a70000,     0x14e0000,    0x08c0000
	},
	/* 4 */{ 0 },
	/* 5 */{ 0 },
	/* 6 W_KING */
	{
		0x0000062,     0x00000e5,     0x00001ca,     0x0000394,     0x0000308,
		0x0000c43,     0x0001ca7,     0x000394e,     0x000729c,     0x0006118,
		0x0018860,     0x00394e0,     0x00729c0,     0x00e5380,     0x00c2300,
		0x0310c00,     0x0729c00,     0x0e53800,     0x1ca7000,     0x1846000,
		0x0218000,     0x0538000,     0x0a70000,     0x14e0000,     0x08c0000
	},
	/* 7 */{ 0 },
	/* 8 */{ 0 },
	/* 9 */{ 0 },
	/* 10 */{ 0 },
	/* 11 */{ 0 },
	/* 12 */{ 0 },
	/* 13 */{ 0 },
	/* 14 */{ 0 },
	/* 15 */{ 0 },
	/* 16 */{ 0 },
	/* 17 B_PAWN */
	{
		0x0000020,     0x0000040,     0x0000080,     0x0000100,     0x0000200,
		0x0000400,     0x0000800,     0x0001000,     0x0002000,     0x0004000,
		0x0008000,     0x0010000,     0x0020000,     0x0040000,    0x0080000,
		0x0100000,     0x0200000,     0x0400000,     0x0800000,    0x1000000,
		0x0000000,     0x0000000,     0x0000000,     0x0000000,    0x0000000
	},
	/* 18 B_SILVER */
	{
		0x0000060,     0x00000e0,     0x00001c0,     0x0000380,     0x0000300,
		0x0000c02,     0x0001c05,     0x000380a,     0x0007014,     0x0006008,
		0x0018040,     0x00380a0,     0x0070140,     0x00e0280,    0x00c0100,
		0x0300800,     0x0701400,     0x0e02800,     0x1c05000,    0x1802000,
		0x0010000,     0x0028000,     0x0050000,     0x00a0000,    0x0040000
	},
	/* 19 B_GOLD */
	{
		0x0000062,     0x00000e5,     0x00001ca,     0x0000394,     0x0000308,
		0x0000c41,     0x0001ca2,     0x0003944,     0x0007288,     0x0006110,
		0x0018820,     0x0039440,     0x0072880,     0x00e5100,    0x00c2200,
		0x0310400,     0x0728800,     0x0e51000,     0x1ca2000,    0x1844000,
		0x0208000,     0x0510000,     0x0a20000,     0x1440000,    0x0880000
	},
	/* 20 */{ 0 },
	/* 21 */{ 0 },
	/* 22 B_KING */
	{
		0x0000062,     0x00000e5,     0x00001ca,     0x0000394,     0x0000308,
		0x0000c43,     0x0001ca7,     0x000394e,     0x000729c,     0x0006118,
		0x0018860,     0x00394e0,     0x00729c0,     0x00e5380,     0x00c2300,
		0x0310c00,     0x0729c00,     0x0e53800,     0x1ca7000,     0x1846000,
		0x0218000,     0x0538000,     0x0a70000,     0x14e0000,     0x08c0000
	},
};

/*    Rook Attack Area    */
const Bitboard ColUpper[] = {
	0x0108420,    0x0210840,    0x0421080,    0x0842100,    0x1084200,
	0x0108400,    0x0210800,    0x0421000,    0x0842000,    0x1084000,
	0x0108000,    0x0210000,    0x0420000,    0x0840000,    0x1080000,
	0x0100000,    0x0200000,    0x0400000,    0x0800000,    0x1000000,
	0x0000000,    0x0000000,    0x0000000,    0x0000000,    0x0000000
};

const Bitboard ColLower[] = {
	0x0000000,    0x0000000,    0x0000000,    0x0000000,    0x0000000,
	0x0000001,    0x0000002,    0x0000004,    0x0000008,    0x0000010,
	0x0000021,    0x0000042,    0x0000084,    0x0000108,    0x0000210,
	0x0000421,    0x0000842,    0x0001084,    0x0002108,    0x0004210,
	0x0008421,    0x0010842,    0x0021084,    0x0042108,    0x0084210

};

const Bitboard RowUpper[] = {
	0x000001e,    0x000001c,    0x0000018,    0x0000010,    0x0000000,
	0x00003c0,    0x0000380,    0x0000300,    0x0000200,    0x0000000,
	0x0007800,    0x0007000,    0x0006000,    0x0004000,    0x0000000,
	0x00f0000,    0x00e0000,    0x00c0000,    0x0080000,    0x0000000,
	0x1e00000,    0x1c00000,    0x1800000,    0x1000000,    0x0000000
};

const Bitboard RowLower[] = {
	0x0000000,    0x0000001,    0x0000003,    0x0000007,    0x000000f,
	0x0000000,    0x0000020,    0x0000060,    0x00000e0,    0x00001e0,
	0x0000000,    0x0000400,    0x0000c00,    0x0001c00,    0x0003c00,
	0x0000000,    0x0008000,    0x0018000,    0x0038000,    0x0078000,
	0x0000000,    0x0100000,    0x0300000,    0x0700000,    0x0f00000
};

/*    Bishop Attack Area    */
// slope1 -> '/'
const Bitboard Slope1Upper[] = {
	0x0000000,    0x0000020,    0x0000440,    0x0008880,    0x0111100,
	0x0000000,    0x0000400,    0x0008800,    0x0111000,    0x0222000,
	0x0000000,    0x0008000,    0x0110000,    0x0220000,    0x0440000,
	0x0000000,    0x0100000,    0x0200000,    0x0400000,    0x0800000,
	0x0000000,    0x0000000,    0x0000000,    0x0000000,    0x0000000
};

const Bitboard Slope1Lower[] = {
	0x0000000,    0x0000000,    0x0000000,    0x0000000,    0x0000000,
	0x0000002,    0x0000004,    0x0000008,    0x0000010,    0x0000000,
	0x0000044,    0x0000088,    0x0000110,    0x0000200,    0x0000000,
	0x0000888,    0x0001110,    0x0002200,    0x0004000,    0x0000000,
	0x0011110,    0x0022200,    0x0044000,    0x0080000,    0x0000000
};

// slope2 -> '\'
const Bitboard Slope2Upper[] = {
	0x1041040,    0x0082080,    0x0004100,    0x0000200,    0x0000000,
	0x0820800,    0x1041000,    0x0082000,    0x0004000,    0x0000000,
	0x0410000,    0x0820000,    0x1040000,    0x0080000,    0x0000000,
	0x0200000,    0x0400000,    0x0800000,    0x1000000,    0x0000000,
	0x0000000,    0x0000000,    0x0000000,    0x0000000,    0x0000000
};

const Bitboard Slope2Lower[] = {
	0x0000000,    0x0000000,    0x0000000,    0x0000000,    0x0000000,
	0x0000000,    0x0000001,    0x0000002,    0x0000004,    0x0000008,
	0x0000000,    0x0000020,    0x0000041,    0x0000082,    0x0000104,
	0x0000000,    0x0000400,    0x0000820,    0x0001041,    0x0002082,
	0x0000000,    0x0008000,    0x0010400,    0x0020820,    0x0041041
};


// Bitboard Mask
const Bitboard BitboardMask = 0x1ffffff;
const Bitboard HighestPosMask = 0x1000000;
const Bitboard LowestPosMask = 0x0000001;
const Bitboard EnemyCampMask[] = { 0x000001f, 0x1f00000 };

inline Bitboard RowMask(int pos) { return RowUpper[pos] | RowLower[pos]; }
inline Bitboard ColMask(int pos) { return ColUpper[pos] | ColLower[pos]; }
inline Bitboard BlankOccupied(Bitboard occupied) { return ~occupied & BitboardMask; }

const Bitboard RookMask[] = {
	0x010843e,    0x021085d,    0x042109b,    0x0842117,    0x108420f,
	0x01087c1,    0x0210ba2,    0x0421364,    0x08422e8,    0x10841f0,
	0x010f821,    0x0217442,    0x0426c84,    0x0845d08,    0x1083e10,
	0x01f0421,    0x02e8842,    0x04d9084,    0x08ba108,    0x107c210,
	0x1e08421,    0x1d10842,    0x1b21084,    0x1742108,    0x0f84210
};

const Bitboard BishopMask[] = {
	0x1041040,    0x00820a0,    0x0004540,    0x0008a80,    0x0111100,
	0x0820802,    0x1041405,    0x008a80a,    0x0115014,    0x0222008,
	0x0410044,    0x08280a8,    0x1150151,    0x02a0282,    0x0440104,
	0x0200888,    0x0501510,    0x0a02a20,    0x1405041,    0x0802082,
	0x0011110,    0x002a200,    0x0054400,    0x00a0820,    0x0041041
};

// Debrujin
const unsigned int debrujin32 = 0x7ca26eb;

const int index32[] = {
	0, 14,  1, 15,  9, 18,  2, 12,
	16, 10, 26, 28, 19, 22,  3, 30,
	13,  8, 17, 11, 25, 27, 21, 29,
	7, 24, 20,  6, 23,  5,  4, 31
};

// LSB 最右邊的1是第幾個bit 最右邊是0 
inline Square BitScan(Bitboard bitboard) {
	//return Square(index32[((bitboard ^ (bitboard - 1)) * debrujin32) >> 27]);
	return Square(_tzcnt_u32(bitboard));
}

// MSB 最左邊的1是第幾個bit 最右邊是0
inline Square BitScanRev(Bitboard bitboard) {
	/*Bitboard o = bitboard;
	bitboard |= bitboard >> 1;
	bitboard |= bitboard >> 2;
	bitboard |= bitboard >> 4;
	bitboard |= bitboard >> 8;
	bitboard |= bitboard >> 16;
	return Square(index32[(bitboard * debrujin32) >> 27]);*/
	return Square(31 - _lzcnt_u32(bitboard));
}

// pin value
#define more_than_one(x) (x & (x - 1))

const Bitboard BetweenBB[][BOARD_NB] = {
	{ 0, 0, 2, 6, 14, 0, 0, 0, 0, 0, 32, 0, 64, 0, 0, 1056, 0, 0, 4160, 0, 33824, 0, 0, 0, 266304 },
	{ 0, 0, 0, 4, 12, 0, 0, 0, 0, 0, 0, 64, 0, 128, 0, 0, 2112, 0, 0, 8320, 0, 67648, 0, 0, 0 },
	{ 2, 0, 0, 0, 8, 0, 0, 0, 0, 0, 64, 0, 128, 0, 256, 0, 0, 4224, 0, 0, 0, 0, 135296, 0, 0 },
	{ 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 256, 0, 2176, 0, 0, 8448, 0, 0, 0, 0, 270592, 0 },
	{ 14, 12, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 512, 0, 4352, 0, 0, 16896, 69888, 0, 0, 0, 541184 },
	{ 0, 0, 0, 0, 0, 0, 0, 64, 192, 448, 0, 0, 0, 0, 0, 1024, 0, 2048, 0, 0, 33792, 0, 0, 133120, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 128, 384, 0, 0, 0, 0, 0, 0, 2048, 0, 4096, 0, 0, 67584, 0, 0, 266240 },
	{ 0, 0, 0, 0, 0, 64, 0, 0, 0, 256, 0, 0, 0, 0, 0, 2048, 0, 4096, 0, 8192, 0, 0, 135168, 0, 0 },
	{ 0, 0, 0, 0, 0, 192, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4096, 0, 8192, 0, 69632, 0, 0, 270336, 0 },
	{ 0, 0, 0, 0, 0, 448, 384, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8192, 0, 16384, 0, 139264, 0, 0, 540672 },
	{ 32, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2048, 6144, 14336, 0, 0, 0, 0, 0, 32768, 0, 65536, 0, 0 },
	{ 0, 64, 0, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4096, 12288, 0, 0, 0, 0, 0, 0, 65536, 0, 131072, 0 },
	{ 64, 0, 128, 0, 256, 0, 0, 0, 0, 0, 2048, 0, 0, 0, 8192, 0, 0, 0, 0, 0, 65536, 0, 131072, 0, 262144 },
	{ 0, 128, 0, 256, 0, 0, 0, 0, 0, 0, 6144, 4096, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131072, 0, 262144, 0 },
	{ 0, 0, 256, 0, 512, 0, 0, 0, 0, 0, 14336, 12288, 8192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 262144, 0, 524288 },
	{ 1056, 0, 0, 2176, 0, 1024, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65536, 196608, 458752, 0, 0, 0, 0, 0 },
	{ 0, 2112, 0, 0, 4352, 0, 2048, 0, 4096, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131072, 393216, 0, 0, 0, 0, 0 },
	{ 0, 0, 4224, 0, 0, 2048, 0, 4096, 0, 8192, 0, 0, 0, 0, 0, 65536, 0, 0, 0, 262144, 0, 0, 0, 0, 0 },
	{ 4160, 0, 0, 8448, 0, 0, 4096, 0, 8192, 0, 0, 0, 0, 0, 0, 196608, 131072, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 8320, 0, 0, 16896, 0, 0, 8192, 0, 16384, 0, 0, 0, 0, 0, 458752, 393216, 262144, 0, 0, 0, 0, 0, 0, 0 },
	{ 33824, 0, 0, 0, 69888, 33792, 0, 0, 69632, 0, 32768, 0, 65536, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2097152, 6291456, 14680064 },
	{ 0, 67648, 0, 0, 0, 0, 67584, 0, 0, 139264, 0, 65536, 0, 131072, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4194304, 12582912 },
	{ 0, 0, 135296, 0, 0, 0, 0, 135168, 0, 0, 65536, 0, 131072, 0, 262144, 0, 0, 0, 0, 0, 2097152, 0, 0, 0, 8388608 },
	{ 0, 0, 0, 270592, 0, 133120, 0, 0, 270336, 0, 0, 131072, 0, 262144, 0, 0, 0, 0, 0, 0, 6291456, 4194304, 0, 0, 0 },
	{ 266304, 0, 0, 0, 541184, 0, 266240, 0, 0, 540672, 0, 0, 262144, 0, 524288, 0, 0, 0, 0, 0, 14680064, 12582912, 8388608, 0, 0 }
};

// Moveable
inline Bitboard RookMovable(Square srcIndex, Bitboard occupied) {
	// upper (find LSB) ; lower (find MSB)
	Bitboard rank, file, upper, lower;

	// row "-"
	upper = (occupied & RowUpper[srcIndex]) | HighestPosMask;
	lower = (occupied & RowLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	rank = (upper - lower) & RowMask(srcIndex);

	// column "|"
	upper = (occupied & ColUpper[srcIndex]) | HighestPosMask;
	lower = (occupied & ColLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	file = (upper - lower) & ColMask(srcIndex);

	return rank | file;
}

inline Bitboard BishopMovable(Square srcIndex, Bitboard occupied) {
	// upper (find LSB) ; lower (find MSB)
	Bitboard slope1, slope2, upper, lower;

	// slope1 "/"
	upper = (occupied & Slope1Upper[srcIndex]) | HighestPosMask;
	lower = (occupied & Slope1Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope1 = (upper - lower) & (Slope1Upper[srcIndex] | Slope1Lower[srcIndex]);

	// slope2 "\"
	upper = (occupied & Slope2Upper[srcIndex]) | HighestPosMask;
	lower = (occupied & Slope2Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope2 = (upper - lower) & (Slope2Upper[srcIndex] | Slope2Lower[srcIndex]);

	return slope1 | slope2;
}

inline Bitboard Movable(Square srcIndex, Piece srcPiece, Bitboard occupied) {
	if ((srcPiece & 7) == BISHOP) {
		if (is_promote(srcPiece))
			return BishopMovable(srcIndex, occupied) | Movement[KING][srcIndex];
		return BishopMovable(srcIndex, occupied);
	}
	else if ((srcPiece & 7) == ROOK) {
		if (is_promote(srcPiece))
			return RookMovable(srcIndex, occupied) | Movement[KING][srcIndex];
		return RookMovable(srcIndex, occupied);
	}
	else if (is_promote(srcPiece)) {
		return Movement[GOLD | (srcPiece & BLACKCHESS)][srcIndex];
	}
	return Movement[srcPiece][srcIndex];
}

inline Bitboard AttackersTo(Square dstIndex, Bitboard occupied) {
	return RookMovable(dstIndex, occupied) | BishopMovable(dstIndex, occupied);
}

#endif
