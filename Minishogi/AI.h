#ifndef _AI_
#define _AI_
#include "head.h"

struct TranspositNode;
class Board;

Action IDAS(Board& board, bool turn);
int NegaScout(Board& board, int alpha, int beta, int depth, bool turn, bool isFailHigh);
int QuiescenceSearch(Board& board, int alpha, int beta, bool turn);
int SEE(int dstIndex, bool turn);

/* Transposition Table */
bool ReadTransposit(U32 hashcode, TranspositNode& bestNode);
void UpdateTransposit(U32 hashcode, int score, bool isExact, int depth, Action action);

/* History Heuristic */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

#endif // !_AI_