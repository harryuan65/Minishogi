#ifndef _AI_
#define _AI_
#include "head.h"

Action IDAS(Board& board, PV &pv);
int NegaScout(PV &pv, Board& board, int alpha, int beta, int depth, bool isFailHigh);
int QuiescenceSearch(Board& board, int alpha, int beta);
int SEE(const Board &board, int dstIndex);

/* Transposition Table */
bool ReadTransposit(U32 hashcode, TranspositNode& bestNode);
void UpdateTransposit(U32 hashcode, int score, bool isExact, U32 depth, Action action);

/* History Heuristic */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

#endif // !_AI_