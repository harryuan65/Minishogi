#ifndef _DEFINE_
#define _DEFINE_

typedef unsigned __int64 U64;
typedef unsigned __int32 U32;
typedef unsigned __int16 U16;
typedef unsigned __int8   U8;
typedef unsigned __int32 Action;

struct TranspositNode {
    TranspositNode() {}
    TranspositNode(int score, bool isExact, int depth, Action action) {
        bestScore = score;
        bestAction = (isExact << 31) | (depth << 25) | action;
    }

    short bestScore;
    Action bestAction;
};

#define HUMAN_CTRL 0
#define AI_CTRL    1
#define OTHERAI_CTRL 2

/*     SIZE     */
#define BOARD_SIZE 25
#define TOTAL_BOARD_SIZE 37

/*     TURN     */
#define TURN_WHITE 0
#define TURN_BLACK 1
#define PROMOTE 0x08
#define BLACKCHESS 0x10
#define EMPTY 0

/*    move    */
#define blank_board (~(board.occupied[BLACK_TURN] | board.occupied[WHITE_TURN]) & BOARD_MASK)
#define ENEMYCAMP(turn)  (turn ? WHITE_CAMP : BLACK_CAMP)

/*    mask    */
#define row_mask(pos)    (row_upper[pos] | row_lower[pos])
#define column_mask(pos) (column_upper[pos] | column_lower[pos])
#define slope1_mask(pos) (slope1_upper[pos] | slope1_lower[pos])
#define slope2_mask(pos) (slope2_upper[pos] | slope2_lower[pos])

/*    move mask    */
#define ACTION_TO_SRCINDEX(action) (action & 0x0000003f)
#define ACTION_TO_DSTINDEX(action) (action & 0x00000fc0) >> 6
#define ACTION_TO_SRCCHESS(action) (action & 0x0003f000) >> 12
#define ACTION_TO_DSTCHESS(action) (action & 0x00fc0000) >> 18
#define ACTION_TO_ISPRO(action)    (action & 0x01000000) >> 24
#define ACTION_TO_DEPTH(action)    (action & 0x7e000000) >> 25
#define ACTION_TO_ISEXACT(action)  (action >> 31)


#define SRC_INDEX_MASK 0x003f
#define DST_INDEX_MASK 0x0fc0
#define SRC_CHESS_MASK 0x3f000
#define DST_CHESS_MASK 0xfc0000

#define PRO_MASK 0x1000000
#define BOARD_MASK 0x01ffffff
#define ACTION_MASK 0x01ffffff

// max move number including attack (21) and move (29)
// and the hand chess (112) (23 * 4 + 20)
#define MAX_MOVE_NUM 112 // max 162 ; testing max

/*    camp    */
#define WHITE_CAMP 0x1f00000
#define BLACK_CAMP 0x000001f

/*    board    */
#define HIGHTEST_BOARD_POS 0x1000000
#define LOWEST_BOARD_POS   0x0000001

/* search depth */
#define IDAS_START_DEPTH 11
#define IDAS_END_DEPTH   8

/*    search    */
#define CHECKMATE       SHRT_MAX
#define DEPTH_UCHI      3

struct PV {
    Action action[IDAS_END_DEPTH];
    unsigned int count = 0;
};
#endif