#ifndef _AI_
#define _AI_
#include "head.h"


/*typedef struct line_t
{
	int pv_count;
	Action pv[11];
}line;*/
struct TranspositNode;
class Board;

Action IDAS(Board& board, bool turn);
int NegaScout(/*line *mPVAction, */Board& board, int alpha, int beta, int depth, bool turn, bool isFailHigh);
int QuiescenceSearch(Board& board, int alpha, int beta, bool turn);
int SEE(Board& board, int dstIndex, bool turn);

/* Transposition Table */
bool ReadTransposit(U64 hashcode, TranspositNode& bestNode);
void UpdateTransposit(U64 hashcode, int score, bool isExact, bool turn, int depth, Action action);

/* History Heuristic */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

Action IsomorphismAction(Action action);
void PrintPV(Board& board, bool turn, int depth);

inline void PrintAction(Action action){
	printf("%d: %2d => %2d isPro:%d\n",
		ACTION_TO_DEPTH(action), 
		ACTION_TO_SRCINDEX(action), 
		ACTION_TO_DSTINDEX(action), 
		(bool)ACTION_TO_ISPRO(action));

}

#endif // !_AI_