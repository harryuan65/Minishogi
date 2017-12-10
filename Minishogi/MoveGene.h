#ifndef _MOVEGENE_
#define _MOVEGENE_
#include "head.h"

U32 RookMove(const Board &board, const int pos);

U32 BishopMove(const Board &board, const int pos);

inline U32 Movable(const Board &board, const int srcIndex);

void AttackGenerator(Board &board, Action *movelist, U32 &start);

void MoveGenerator(Board &board, Action *movelist, U32 &start);

void HandGenerator(Board &board, Action *movelist, U32 &start);

#endif
