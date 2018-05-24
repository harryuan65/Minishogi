#ifndef _AI_
#define _AI_

#include "MovePick.h"

//#define ITERATIVE_DEEPENING_DISABLE
//#define ASPIRE_WINDOW_DISABLE
//#define PVS_DISABLE
//#define QUIES_DISABLE

namespace Search {
	extern ButterflyHistory mainHistory;
	extern CapturePieceToHistory captureHistory;
	extern PieceToHistory contHistory[CHESS_NB][SQUARE_NB];
	extern CounterMoveHistory counterMoves;

	/// Threshold used for countermoves based pruning
	constexpr int CounterMovePruneThreshold = 0;

	struct Stack {
		Move pv[MAX_PLY + 1];
		PieceToHistory* contHistory;
		int ply;
		Move currentMove;
		//Move excludedMove;
		Move killers[2];
		//Value staticEval;
		//int statScore;
		int moveCount;
	};

	void Initialize();
	Value IDAS(Minishogi& pos, Move &bestMove, Move *pv);
	Value NegaScout(bool pvNode, Minishogi& pos, Stack *ss, Value alpha, Value beta, int depth, bool isResearch);
	Value QuietSearch(bool pvNode, Minishogi& pos, Stack *ss, Value alpha, Value beta, int depth);

	void UpdatePv(Move* pv, Move move, Move* childPv);
	void UpdateAttackHeuristic(const Minishogi &minishogi, Move move, Move *attackMove, int attackCnt, int bouns);
	void UpdateQuietHeuristic(const Minishogi &minishogi, Stack* ss, Move move, Move *quietMove, int quietCnt, int bouns);
	void UpdateContinousHeuristic(Stack* ss, Chess pc, Square to, int bonus);

	void PrintPV(std::ostream &os, Move *move);
}
#endif