#ifndef _AI_
#define _AI_
#include "head.h"

/*    Negascout Algorithm    */
Action IDAS(Board& board, PV &pv);
int NegaScout(PV &pv, Board& board, int alpha, int beta, int depth, bool isFailHigh);
int QuiescenceSearch(Board& board, int alpha, int beta);
int SEE(const Board &board, int dstIndex);

/*    TransPosition Table    */ 
const U64 TPSize = 0x0000000008000000ULL;
const U64 TPMask = 0x0000000007ffffffULL;
const U64 TPShift = 0;
//const int TPLimit = 3;

#pragma pack(push)
#pragma pack(1)
struct TransPosition {
	U32 zobrist;              //8Bytes
	short value;              //2Bytes -32767~32767
	BYTE depth;               //1Bytes 0~15
	enum : BYTE {             //1Bytes 0~2
		Exact,
		Unknown,
		FailHigh
	} state;

	//Debug
	//BYTE board[35];
};
#pragma pack(pop)

void InitializeTP();
void CleanTP();
bool ReadTP(Zobrist::Zobrist zobrist, int depth, int& alpha, int& beta, int& value, Board &board);
void UpdateTP(Zobrist::Zobrist zobrist, int depth, int alpha, int beta, int value, Board &board);

/*    History Heuristic    */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

#endif