#ifndef _DEFINE_
#define _DEFINE_

typedef unsigned __int64 U64;
typedef unsigned __int32 U32;
typedef unsigned __int16 U16;
typedef unsigned __int32 Action;

/*     TURN     */
#define WHITE_TURN 0
#define BLACK_TURN 1

/*     Chess	*/
#define PROMOTE          0x08
#define BLACKCHESS       0x10
#define BOARD_SIZE       25
#define TOTAL_BOARD_SIZE 37

/*     Bitboard Mask    */
#define HIGHTEST_BOARD_POS 0x1000000
#define LOWEST_BOARD_POS   0x0000001
#define WHITE_CAMP         0x1f00000
#define BLACK_CAMP         0x000001f
#define BOARD_MASK         0x01ffffff
#define blank_board      (~(board.occupied[BLACK_TURN] | board.occupied[WHITE_TURN]) & BOARD_MASK)
#define ENEMYCAMP(turn)  (turn ? WHITE_CAMP : BLACK_CAMP)
#define row_mask(pos)    (row_upper[pos] | row_lower[pos])
#define column_mask(pos) (column_upper[pos] | column_lower[pos])
#define slope1_mask(pos) (slope1_upper[pos] | slope1_lower[pos])
#define slope2_mask(pos) (slope2_upper[pos] | slope2_lower[pos])

/*    Action Mask    */
#define PRO_MASK 0x1000000
#define ACTION_TO_SRCINDEX(action) (action & 0x0000003f)
#define ACTION_TO_DSTINDEX(action) (action & 0x00000fc0) >> 6
#define ACTION_TO_SRCCHESS(action) (action & 0x0003f000) >> 12
#define ACTION_TO_DSTCHESS(action) (action & 0x00fc0000) >> 18
#define ACTION_TO_ISPRO(action)    (action & 0x01000000) >> 24

/*    Search    */
// max move number including attack (21) and move (29)
// and the hand chess (112) (23 * 4 + 20)
#define MAX_MOVE_NUM 112 // max 162 ; testing max
#define MAX_DEPTH    18  // PV°}¦C¥Î
#define DEPTH_UCHI   3

#endif