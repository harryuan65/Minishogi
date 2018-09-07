#ifndef _ZOBRIST_H_
#define _ZOBRIST_H_

#include <random>
#include "Types.h"

namespace Zobrist {
	constexpr uint64_t SEED = std::mt19937_64::default_seed;
	extern Key table[SQUARE_NB][PIECE_NB];
	extern Key table2[SQUARE_NB][PIECE_NB];

	void Initialize();
}

#endif