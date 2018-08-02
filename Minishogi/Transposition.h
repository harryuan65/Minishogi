#ifndef _TRANSPOSITION_
#define _TRANSPOSITION_

#include "Types.h"
#include "Zobrist.h"
#include "Observer.h"
#include "Move.h"
#include <assert.h>

namespace Transposition {
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
		//int turn; //4Bytes debug¥Î

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

		/*void save(Key k, int d, Value v, Move m, Bound b, int t) {
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

	extern uint64_t TPSize;
	extern uint64_t TPMask;
	extern TTentry* transpositTable;

	inline uint64_t ZobristToIndex(Key zobrist) { return zobrist & TPMask; }
	inline bool IsEnable() {
#ifndef TRANSPOSITION_DISABLE
		return true;
#else
		return false;
#endif
	}

	void Initialize();
	void Clean();
	TTentry* Probe(Key key, bool &ttHit);
	TTentry* Probe(Key key, int turn, bool &ttHit);
}
#endif

