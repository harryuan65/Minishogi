#ifndef _ZOBRIST_
#define _ZOBRIST_
#include <iostream>
#include <random>
#include "library.h"
using namespace std;

namespace Zobrist {
	typedef unsigned __int64 Zobrist;

	static const unsigned __int64 SEED = mt19937_64::default_seed;
	extern Zobrist table[][30];
	extern Zobrist table2[][30];

	void Initialize();
}
#endif