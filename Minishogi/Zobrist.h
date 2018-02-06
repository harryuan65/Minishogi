#ifndef _ZOBRIST_
#define _ZOBRIST_
#include <iostream>
#include <random>
using namespace std;

namespace Zobrist {
	typedef unsigned __int64 Zobrist;
	typedef unsigned __int32 HalfZobrist;

	const int SEED = 11;
	extern HalfZobrist* key[37];

	void Initialize();
	inline Zobrist GetZobristHash(HalfZobrist whiteHashcode, HalfZobrist blackHashcode, bool turn) {
		return turn ? ((Zobrist)blackHashcode << 32) | whiteHashcode : 
					  ((Zobrist)whiteHashcode << 32) | blackHashcode;
	}
}

#endif