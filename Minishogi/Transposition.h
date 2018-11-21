#ifndef _TRANSPOSITION_H_
#define _TRANSPOSITION_H_

#include <assert.h>

#include "Types.h"
#include "Zobrist.h"

struct TTentry {
	uint32_t key32;    //4Bytes
	int16_t  value;    //2Bytes -32767~32767
	int8_t   depth;    //1Bytes -1~15
	Move     move;     //4Bytes
	enum Bound : uint8_t {   //1Bytes 1~3
		UNKNOWN = 1,
		FAILHIGH = 2,
		EXACT = 3
	} bound;

	void save(Key k, int d, Value v, Move m, Bound b) {
#ifndef TRANSPOSITION_DISABLE
		if (k >> 32 != key32 || d >= depth - 4 || b == EXACT) {
			key32 = k >> 32;
			value = v;
			depth = d;
			move  = m;
			bound = b;
			assert(value > -VALUE_INFINITE && value < VALUE_INFINITE);
		}
#endif
	}

	// Debug : Enemy Isomorphic
	/*int turn; //4Bytes debug¥Î
	void save(Key k, int d, Value v, Move m, Bound b, int t) {
#ifndef TRANSPOSITION_DISABLE
			if (k >> 32 != key32 || d >= depth) {
				key32 = k >> 32;
				value = v;
				depth = d;
				move = m;
				bound = b;
				turn = t;
				assert(value > -VALUE_INFINITE && value < VALUE_INFINITE);
			}
#endif
		}*/
};

class Transposition {
public:
	~Transposition();
	void Initialize(int ttBit);
	void Clean();
	TTentry* Probe(Key key, bool &ttHit);
	//TTentry* Probe(Key key, int turn, bool &ttHit);
	uint64_t ZobristToIndex(Key zobrist);
	int HashFull() const;

private:
	TTentry* transpositTable;

	uint64_t ttSize;
	uint64_t ttMask;
};

inline uint64_t Transposition::ZobristToIndex(Key zobrist) { 
	return zobrist & ttMask;
}

#endif

