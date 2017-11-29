#ifndef _AI_
#define _AI_
#include "head.h"

struct TranspositNode;
class Board;

Action IDAS(Board& board, bool turn);
int NegaScout(Board& board, int alpha, int beta, int depth, bool turn, bool isFailHigh);
int QuiescenceSearch(Board& board, int alpha, int beta, bool turn);
int SEE(Board& board, int dstIndex, bool turn);

/* Transposition Table */
bool ReadTransposit(U32 hashcode, TranspositNode& bestNode);
void UpdateTransposit(U32 hashcode, int score, bool isExact, bool turn, int depth, Action action);

/* History Heuristic */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

Action IsomorphismAction(Action action);
bool PrintPV(Board& board, bool turn);

inline void PrintAction(Action action){
	printf("%2d => %2d\n", ACTION_TO_SRCINDEX(action), (ACTION_TO_DSTINDEX(action)));

}

#endif // !_AI_