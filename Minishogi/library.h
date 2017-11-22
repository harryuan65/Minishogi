#ifndef _LIBRARY_
#define _LIBRARY_

/*	player	*/
enum {
	WHITE = 0, BLACK = 1
};
/*

0	  1     2     3     4     5    6    7
"X","w步","w銀","w金","w角","w飛","王","X",

 8    9     10   11    12   13   14   15 
"X","wㄈ","w全","X"," w馬","w龍","X","X",

 16   17    18     19    20   21   22  23
"X","b步","b銀","b金","b角","b飛","玉","X",

24   25    26    27   28   29    30  31  
"X","wㄈ","b全","X","b馬","b龍","X","X"

*/
const int EatToHand[] = {
	0, 30, 31, 32, 33, 34, 0, 0,//8, 0-7
	0, 30, 31,  0, 33, 34, 0, 0,//16, 8-15
	0, 25, 26, 27, 28, 29, 0, 0,//24, 16-23
	0, 25, 26,  0, 28, 29
};

/*	board	*/
enum {
	// initialize board
	A5, A4, A3, A2, A1,
	B5, B4, B3, B2, B1,
	C5, C4, C3, C2, C1,
	D5, D4, D3, D2, D1,
	E5, E4, E3, E2, E1,

	//initialize white_hand
	F5, F4, F3, F2, F1, // R, R, B, B, G
	G5, G4, G3, G2, G1,	// G, S, S, P, P

	//initialize black_hand
	H5, H4, H3, H2, H1, // R, R, B, B, G
	I5, I4, I3, I2, I1	// G, S, S, P, P
};
#define BOARD_SIZE 25
#define TOTAL_BOARD_SIZE 35

/*	chess	*/
enum {				
	//Print ,identify chess usage
	BLANK,	//0
	PAWN,	//1
	SILVER,	//2
	GOLD,	//3
	BISHOP, //4
	ROOK,	//5
	KING,	//6
	//7
	//8
	//E_PAWN,//9
	//E_SILVER,//10
	//11
	//E_BISHOP,//12
	//E_ROOK,//13
	//14
	//15
	//16
	//bPAWN //17
	//bSILVER,18
	//bGOLD,19
	//bBISHOP,20
	//bROOK,21
	//bKING,22
	//23
	//24
	//b_E_PAWN,25
	//b_E_SILVER,26
	//27
	//b_E_BISHOP,28
	//b_E_ROOK,29
	//30
	//31
};




/*	all chess move	*/
/*	king bit move	*/


const U32 king_move[BOARD_SIZE] = {
	0x0000062, 	0x00000e5, 	0x00001ca, 	0x0000394, 	0x0000308,
	0x0000c43, 	0x0001ca7, 	0x000394e, 	0x000729c, 	0x0006118,
	0x0018860, 	0x00394e0, 	0x00729c0, 	0x00e5380, 	0x00c2300,
	0x0310c00, 	0x0729c00, 	0x0e53800, 	0x1ca7000, 	0x1846000,
	0x0218000, 	0x0538000, 	0x0a70000, 	0x14e0000, 	0x08c0000
};

/*	white gold bit move	*/
const U32 w_gold_move[BOARD_SIZE] = {
	0x0000022, 	0x0000045, 	0x000008a, 	0x0000114, 	0x0000208,
	0x0000443, 	0x00008a7, 	0x000114e, 	0x000229c, 	0x0004118,
	0x0008860, 	0x00114e0, 	0x00229c0, 	0x0045380,	0x0082300,
	0x0110c00, 	0x0229c00, 	0x0453800, 	0x08a7000,	0x1046000,
	0x0218000, 	0x0538000, 	0x0a70000, 	0x14e0000,	0x08c0000
};

/*	black gold bit move	*/
const U32 b_gold_move[BOARD_SIZE] = {
	0x0000062, 	0x00000e5, 	0x00001ca, 	0x0000394, 	0x0000308,
	0x0000c41, 	0x0001ca2, 	0x0003944, 	0x0007288, 	0x0006110,
	0x0018820, 	0x0039440, 	0x0072880, 	0x00e5100,	0x00c2200,
	0x0310400, 	0x0728800, 	0x0e51000, 	0x1ca2000,	0x1844000,
	0x0208000, 	0x0510000, 	0x0a20000, 	0x1440000,	0x0880000
};

/*	white silver bit move	*/
const U32 w_silver_move[BOARD_SIZE] = {
	0x0000040, 	0x00000a0, 	0x0000140, 	0x0000280, 	0x0000100,
	0x0000803, 	0x0001407, 	0x000280e, 	0x000501c, 	0x0002018,
	0x0010060, 	0x00280e0, 	0x00501c0, 	0x00a0380,	0x0040300,
	0x0200c00, 	0x0501c00, 	0x0a03800, 	0x1407000,	0x0806000,
	0x0018000, 	0x0038000, 	0x0070000, 	0x00e0000,	0x00c0000
};

/*	black silver bit move	*/
const U32 b_silver_move[BOARD_SIZE] = {
	0x0000060, 	0x00000e0, 	0x00001c0, 	0x0000380, 	0x0000300,
	0x0000c02, 	0x0001c05, 	0x000380a, 	0x0007014, 	0x0006008,
	0x0018040, 	0x00380a0, 	0x0070140, 	0x00e0280,	0x00c0100,
	0x0300800, 	0x0701400, 	0x0e02800, 	0x1c05000,	0x1802000,
	0x0010000, 	0x0028000, 	0x0050000, 	0x00a0000,	0x0040000
};

/*	white pawn bit move	*/
const U32 w_pawn_move[BOARD_SIZE] = {
	0x0000000, 	0x0000000, 	0x0000000, 	0x0000000, 	0x0000000,
	0x0000001, 	0x0000002, 	0x0000004, 	0x0000008, 	0x0000010,
	0x0000020, 	0x0000040, 	0x0000080, 	0x0000100,	0x0000200,
	0x0000400, 	0x0000800, 	0x0001000, 	0x0002000,	0x0004000,
	0x0008000, 	0x0010000, 	0x0020000, 	0x0040000,	0x0080000
};

/*	black pawn bit move	*/
const U32 b_pawn_move[BOARD_SIZE] = {
	0x0000020, 	0x0000040, 	0x0000080, 	0x0000100, 	0x0000200,
	0x0000400, 	0x0000800, 	0x0001000, 	0x0002000, 	0x0004000,
	0x0008000, 	0x0010000, 	0x0020000, 	0x0040000,	0x0080000,
	0x0100000, 	0x0200000, 	0x0400000, 	0x0800000,	0x1000000,
	0x0000000, 	0x0000000, 	0x0000000, 	0x0000000,	0x0000000
};

/*	rook attack which it needs	*/
/*	file upper	*/
const U32 column_upper[BOARD_SIZE] = {
	0x0108420,	0x0210840,  0x0421080, 	0x0842100,	0x1084200,
	0x0108400,	0x0210800,	0x0421000,	0x0842000,	0x1084000,
	0x0108000,	0x0210000,	0x0420000,	0x0840000,	0x1080000,
	0x0100000,	0x0200000,	0x0400000,	0x0800000,	0x1000000,
	0x0000000,	0x0000000,	0x0000000,	0x0000000,	0x0000000
};


/*	file lower	*/
const U32 column_lower[BOARD_SIZE] = {
	0x0000000,	0x0000000,	0x0000000,	0x0000000,	0x0000000,
	0x0000001,	0x0000002,	0x0000004,	0x0000008,	0x0000010,
	0x0000021,	0x0000042,	0x0000084,	0x0000108,	0x0000210,
	0x0000421,	0x0000842,	0x0001084,	0x0002108,	0x0004210,
	0x0008421,	0x0010842,	0x0021084,	0x0042108,	0x0084210

};

/*	rank upper	*/
const U32 row_upper[BOARD_SIZE] = {
	0x000001e, 	0x000001c, 	0x0000018, 	0x0000010, 	0x0000000,
	0x00003c0, 	0x0000380, 	0x0000300, 	0x0000200, 	0x0000000,
	0x0007800, 	0x0007000, 	0x0006000, 	0x0004000,	0x0000000,
	0x00f0000, 	0x00e0000, 	0x00c0000, 	0x0080000,	0x0000000,
	0x1e00000, 	0x1c00000, 	0x1800000,  0x1000000,	0x0000000
};

/*	rank lower	*/
const U32 row_lower[BOARD_SIZE] = {
	0x0000000, 	0x0000001, 	0x0000003, 	0x0000007, 	0x000000f,
	0x0000000, 	0x0000020, 	0x0000060,  0x00000e0, 	0x00001e0,
	0x0000000, 	0x0000400, 	0x0000c00, 	0x0001c00,	0x0003c00,
	0x0000000, 	0x0008000, 	0x0018000, 	0x0038000,	0x0078000,
	0x0000000, 	0x0100000, 	0x0300000, 	0x0700000,	0x0f00000
};

/*	bishop attack which it needs	*/
/*	slope1 -> '/'	*/
/*	slope1 upper	*/
const U32 slope1_upper[BOARD_SIZE] = {
	0x0000000,	0x0000020,	0x0000440,	0x0008880,	0x0111100,
	0x0000000,	0x0000400,	0x0008800,	0x0111000,	0x0222000,
	0x0000000,	0x0008000,	0x0110000,	0x0220000,	0x0440000,
	0x0000000,	0x0100000,	0x0200000,	0x0400000,	0x0800000,
	0x0000000,	0x0000000,	0x0000000,	0x0000000,	0x0000000
};

/*	slope1 lower	*/
const U32 slope1_lower[BOARD_SIZE] = {
	0x0000000,	0x0000000,	0x0000000,	0x0000000,	0x0000000,
	0x0000002,	0x0000004,	0x0000008,	0x0000010,	0x0000000,
	0x0000044,	0x0000088,	0x0000110,	0x0000200,	0x0000000,
	0x0000888,	0x0001110,	0x0002200,	0x0004000,	0x0000000,
	0x0011110,	0x0022200,	0x0044000,	0x0080000,	0x0000000
};

/*	slope2 -> '\'	*/
/*	slope2 upper	*/
const U32 slope2_upper[BOARD_SIZE] = {
	0x1041040,	0x0082080,	0x0004100,	0x0000200,	0x0000000,
	0x0820800,	0x1041000,	0x0082000,	0x0004000,	0x0000000,
	0x0410000,	0x0820000,	0x1040000,	0x0080000,	0x0000000,
	0x0200000,	0x0400000,	0x0800000,	0x1000000,	0x0000000,
	0x0000000,	0x0000000,	0x0000000,	0x0000000,	0x0000000
};

/*	slope2 lower	*/
const U32 slope2_lower[BOARD_SIZE] = {
	0x0000000,	0x0000000,	0x0000000,	0x0000000,	0x0000000,
	0x0000000,	0x0000001,	0x0000002,	0x0000004,	0x0000008,
	0x0000000,	0x0000020,	0x0000041,	0x0000082,	0x0000104,
	0x0000000,	0x0000400,	0x0000820,	0x0001041,	0x0002082,
	0x0000000,	0x0008000,	0x0010400,	0x0020820,	0x0041041
};

// board[35]
// 
#endif