#ifndef _DEFINE_
#define _DEFINE_

typedef unsigned __int64 U64;
typedef unsigned __int32 U32;
typedef unsigned __int16 U16;
typedef unsigned __int8   U8;
typedef unsigned __int32 Action;
typedef unsigned char BYTE;

/* SIZE */
#define BOARD_SIZE 25
#define TOTAL_BOARD_SIZE 35

/* TURN */
#define TURN_WHITE 0
#define TURN_BLACK 1
#define PROMOTE 0x08
#define BLACKCHESS 0x10
#define EMPTY 0

/*    move    */
#define blank_board (~(board.occupied[BLACK] | board.occupied[WHITE]) & BOARD_MASK)

/*    mask    */
#define row_mask(pos)    (row_upper[pos] | row_lower[pos])
#define column_mask(pos) (column_upper[pos] | column_lower[pos])
#define slope1_mask(pos) (slope1_upper[pos] | slope1_lower[pos])
#define slope2_mask(pos) (slope2_upper[pos] | slope2_lower[pos])

/*    move mask    */
#define SRC_INDEX_MASK 0x003f
#define DST_INDEX_MASK 0x0fc0
#define SRC_CHESS_MASK 0x3f000
#define DST_CHESS_MASK 0xfc0000
#define PRO_MASK 0x1000000
#define BOARD_MASK 0x01ffffff

// max move number including attack (21) and move (29)
// and the hand chess (112) (23 * 4 + 20)
#define MAX_MOVE_NUM 162 // max 162 ; testing max 

/*    chess bitboard initial    */
#define WHITE_INIT 0x1f08000
#define BLACK_INIT 0x000021f

/*    every chess bitboard initial    */
#define B_KING_INIT   0x0000010 
#define B_GOLD_INIT   0x0000008 
#define B_SILVER_INIT 0x0000004 
#define B_BISHOP_INIT 0x0000002 
#define B_ROOK_INIT   0x0000001 
#define B_PAWN_INIT   0x0000200 
#define W_KING_INIT   0x0100000 
#define W_GOLD_INIT   0x0200000 
#define W_SILVER_INIT 0x0400000 
#define W_BISHOP_INIT 0x0800000 
#define W_ROOK_INIT   0x1000000 
#define W_PAWN_INIT   0x0008000 

/*    camp    */
#define WHITE_CAMP 0x1f00000
#define BLACK_CAMP 0x000001f

/*    board    */
#define HIGHTEST_BOARD_POS 0x1000000
#define LOWEST_BOARD_POS   0x0000001

/*    search    */
#define INF             100000
#define CHECKMATE       32767
#define LIMIT_DEPTH     11
#define DEPTH_UCHI      3
#define COLOR_BOUND     10

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)

#endif