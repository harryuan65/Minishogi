#ifndef _AI_
#define _AI_

#include "Minishogi.h"
#include "Observer.h"
//#define BEST_ENDGAME_SEARCH
//#define DOUBLETP
//#define TRANSPOSITION_DISABLE

typedef unsigned __int64 U64;
typedef unsigned __int32 U32;

/*    Negascout Algorithm    */
void InitializeNS();
int IDAS(Minishogi& board, Action &bestAction);
int NegaScout(Minishogi& board, Action &bestAction, int alpha, int beta, int depth, bool isResearch, bool isTop);
int QuiescenceSearch(Minishogi& board, int alpha, int beta);
int SEE(const Minishogi &board, int dstIndex);

/*    TransPosition Table    */ 
#ifdef DOUBLETP
const U64 TPSize = 1 << 29;
#else
const U64 TPSize = 1 << 30;
#endif
const U64 TPMask = TPSize - 1;

#pragma pack(push)
#pragma pack(1)
struct TransPosition {
	U32 zobrist;        //4Bytes
	short value;        //2Bytes -32767~32767
	BYTE depth;         //1Bytes 0~15
	enum : BYTE {       //1Bytes 0~2
		Exact,
		Unknown,
		FailHigh
	} state;
};
#pragma pack(pop)

inline U64 ZobristToIndex(Zobrist::Zobrist zobrist);
void InitializeTP();
void CleanTP();
bool ReadTP(Zobrist::Zobrist zobrist, int turn, int depth, int& alpha, int& beta, int& value);
void UpdateTP(Zobrist::Zobrist zobrist, int turn, int depth, int alpha, int beta, int value);
void PrintPV(ostream &os, Minishogi &board, int depth);

/*    History Heuristic    */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

/*    PV   */
/*struct PV {
	Action action[18];
	int evaluate[18];
	int count;
	int leafEvaluate;

	void Print(ostream& os, bool turn) {
		os << "PV: (depth | turn | action | my evaluate)" << "\n";
		for (int i = 0; i < count; ++i) {
			os << i << " : " << (((turn + i) & 1) ? "▼" : "△");
			os << action[i];
			os << setw(7) << (i % 2 ? -evaluate[i] : evaluate[i]) << "\n";
		}
		if (leafEvaluate <= -CHECKMATE || CHECKMATE <= leafEvaluate) {
			os << count << " : " << (((turn + count) & 1) ? "▼" : "△") << "Lose " << setw(7) << leafEvaluate << "\n";
		}
		os << "PV leaf : " << setw(8) << leafEvaluate << "\n";
	}
};*/
#endif