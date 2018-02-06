#ifndef _AI_
#define _AI_
#include "head.h"

/*    Negascout Algorithm    */
Action IDAS(Board& board, PV &pv);
int NegaScout(PV &pv, Board& board, int alpha, int beta, int depth, bool isFailHigh);
int QuiescenceSearch(Board& board, int alpha, int beta);
int SEE(const Board &board, int dstIndex);

/*    TransPosition Table    */ 
const U64 TPSize = 0x0000000001000000ULL;
const U64 TPMask = 0x0000000000ffffffULL;
const U64 TPShift = 0;

struct TransPosition {
	Zobrist::Zobrist zobrist = 0;  //64bits
	short value;                   //16bits -32767~32767
	BYTE depth;                    // 8bits 0~15
	enum : BYTE {                  // 8bits 0~2
		Exact,
		Unknown,
		FailHigh
	} state;
};

void InitializeTP();
bool ReadTP(Zobrist::Zobrist zobrist, int depth, int& alpha, int& beta, int& value);
void UpdateTP(Zobrist::Zobrist zobrist, int depth, int alpha, int beta, int value);

/*    History Heuristic    */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

#endif