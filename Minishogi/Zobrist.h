#ifndef _ZOBRIST_
#define _ZOBRIST_
#include <iostream>
#include <random>

#include "define.h"
using namespace std;

namespace Zobrist {
	typedef unsigned __int64 Zobrist;

	static const U64 SEED = std::mt19937_64::default_seed;
	extern Zobrist table[TOTAL_BOARD_SIZE][30];
	extern Zobrist table2[TOTAL_BOARD_SIZE][30];

	void Initialize();
}

#endif