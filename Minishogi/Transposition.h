#ifndef _TRANSPOSITION_
#define _TRANSPOSITION_

#include "Types.h"
#include "Zobrist.h"
#include "Observer.h"
#include "Move.h"
#include <assert.h>

//#define DOUBLETP
//#define TRANSPOSITION_DISABLE

namespace Transposition {
	struct TTnode {
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
			if (k >> 32 != key32 || d >= depth) {
				key32 = k >> 32;
				value = v;
				depth = d;
				move = m;
				bound = b;
				assert(value > -VALUE_INFINITE && value < VALUE_INFINITE);
			}
#endif
		}
	};

#ifndef TRANSPOSITION_DISABLE
#ifdef DOUBLETP
	const uint64_t TPSize = 1 << 29;
#else
	const uint64_t TPSize = 1 << 27;
#endif
#else
    const uint64_t TPSize = 1;
#endif

	extern TTnode* transpositTable;
	const uint64_t TPMask = TPSize - 1;

	inline uint64_t ZobristToIndex(Key zobrist) { return zobrist & TPMask; }

	void Initialize();
	void Clean();
	TTnode* Probe(Key key, bool &ttHit);
}
#endif

