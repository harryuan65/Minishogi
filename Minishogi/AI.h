#ifndef _AI_
#define _AI_
#include "Minishogi.h"

struct PV;

/*    Negascout Algorithm    */
Action IDAS(Minishogi& board, PV &pv);
int NegaScout(PV &pv, Action &bestAction, Minishogi& board, int alpha, int beta, int depth, bool isResearch, bool isTop);
int QuiescenceSearch(Minishogi& board, int alpha, int beta);
int SEE(const Minishogi &board, int dstIndex);

/*    TransPosition Table    */ 
const U64 TPSize = 1 << 30;
const U64 TPMask = TPSize - 1;

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
};
#pragma pack(pop)

inline U64 ZobristToIndex(Zobrist::Zobrist zobrist);
void InitializeTP();
void CleanTP();
bool ReadTP(Zobrist::Zobrist zobrist, int depth, int& alpha, int& beta, int& value);
void UpdateTP(Zobrist::Zobrist zobrist, int depth, int alpha, int beta, int value);
void PrintPV(ostream &os, Minishogi &board, int depth);

/*    History Heuristic    */
//void SortByHistoryTable(vector<Action>& moveList);
//void UpdateHistoryTable(Action moveAction, int delta); 

struct PV {
	Action action[18];
	int evaluate[18];
	int count = 0;
	int leafEvaluate;

	void Print(ostream& os, bool turn) {
#ifndef PV_DISABLE
		os << "PV: (depth | turn | action | my evaluate)" << "\n";
		for (int i = 0; i < count; ++i) {
			os << i << " : " << (((turn + i) & 1) ? "▼" : "△");
			os << Index2Input(ACTION_TO_SRCINDEX(action[i])) << Index2Input(ACTION_TO_DSTINDEX(action[i]));
			os << (ACTION_TO_ISPRO(action[i]) ? "+" : " ");
			os << setw(7) << (i % 2 ? -evaluate[i] : evaluate[i]) << "\n";
		}
		if (leafEvaluate <= -CHECKMATE || CHECKMATE <= leafEvaluate) {
			os << count << " : " << (((turn + count) & 1) ? "▼" : "△") << "Lose " << setw(7) << leafEvaluate << "\n";
		}
		os << "PV leaf : " << setw(8) << leafEvaluate << "\n";
#else
		os << "PV leaf : " << setw(8) << leafEvaluate << "\n";
#endif
	}
};
#endif