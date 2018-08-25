#ifndef _SEARCH_H_
#define _SEARCH_H_
#include "Thread.h"
#include "Minishogi.h"
#include "Zobrist.h"

namespace Search {
	Value NegaScout(bool pvNode, Minishogi& pos, Stack *ss, Move rootMove, Value alpha, Value beta, int depth, bool isResearch);
	Value QuietSearch(bool pvNode, Minishogi& pos, Stack *ss, Move rootMove, Value alpha, Value beta, int depth);

	void UpdatePv(Move* pv, Move move, Move* childPv);
	void UpdateAttackHeuristic(const Minishogi &minishogi, Move move, Move *attackMove, int attackCnt, int bouns);
	void UpdateQuietHeuristic(const Minishogi &minishogi, Stack* ss, Move move, Move *quietMove, int quietCnt, int bouns);
	void UpdateContinousHeuristic(Stack* ss, Chess pc, Square to, int bonus);

	string GetSettingStr();
}
#endif