#ifndef _ZOBRIST_
#define _ZOBRIST_

#include <random>

#include "Types.h"

namespace Zobrist {
	static const uint64_t SEED = std::mt19937_64::default_seed;
	extern Key table[SQUARE_NB][CHESS_NB];
	extern Key table2[SQUARE_NB][CHESS_NB];

	void Initialize();
}

#endif