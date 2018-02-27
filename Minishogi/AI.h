#ifndef _AI_
#define _AI_
#include "head.h"

/*    Negascout Algorithm    */
Action IDAS(Board& board, PV &pv);
int NegaScout(PV &pv, Board& board, int alpha, int beta, int depth, bool isFailHigh);
int QuiescenceSearch(Board& board, int alpha, int beta);
int SEE(const Board &board, int dstIndex);

/*    TransPosition Table    */ 
const U64 TPSize = 1 << 30;
const U64 TPMask = TPSize - 1;
//const int TPLimit = 3;

#pragma pack(push)
#pragma pack(1)
struct TransPosition {
	U32 zobrist;              //4Bytes
	short value;              //2Bytes -32767~32767
	BYTE depth;               //1Bytes 0~15
	enum : BYTE {             //1Bytes 0~2
		Exact,
		Unknown,
		FailHigh
	} state;

	//Action action;			  //4Bytes for PV
};
#pragma pack(pop)

inline U64 ZobristToIndex(Zobrist::Zobrist zobrist);
void InitializeTP();
void CleanTP();
bool ReadTP(Zobrist::Zobrist zobrist, int depth, int& alpha, int& beta, int& value, Board &board);
void UpdateTP(Zobrist::Zobrist zobrist, int depth, int alpha, int beta, int value, Board &board);
void PrintPV(ostream &os, Board &board, int depth);

/*    History Heuristic    */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

#endif