#include "Transposition.h"
using std::cout;

#ifdef DOUBLETP
static TTnode** transpositTable = nullptr;
void InitializeTP() {
#ifndef TRANSPOSITION_DISABLE
	transpositTable = new TTnode*[2];
	transpositTable[0] = new TTnode[TPSize];
	transpositTable[1] = new TTnode[TPSize];
	CleanTP();
	cout << "TTnode Table Created. ";
	cout << "Used Size : 2 * " << ((TPSize * sizeof(TTnode)) >> 20) << "MiB\n";
#else
	cout << "TTnode Table disable.\n";
#endif
}

void CleanTP() {
#ifndef TRANSPOSITION_DISABLE
	if (transpositTable == nullptr)
		return;
	for (int i = 0; i < TPSize; i++) {
		transpositTable[0][i].zobrist = 0;
		transpositTable[1][i].zobrist = 0;
	}
#else
	cout << "TTnode Table disable.\n";
#endif
}

inline bool ReadTP(Key zobrist, int turn, int depth, int& alpha, int& beta, int& value) {
#ifndef TRANSPOSITION_DISABLE
	uint64_t index = ZobristToIndex(zobrist);
	TTnode *tp = &transpositTable[turn][index];
	if (tp->zobrist != zobrist >> 32) {
		Observer::data[Observer::DataType::indexCollisionNums]++;
		return false;
	}
	if (tp->depth < depth) {
		Observer::data[Observer::DataType::indexCollisionNums]++;
		return false;
	}
	Observer::data[Observer::DataType::totalTPDepth] += depth;

	switch (tp->state) {
	case TTnode::Exact:
		value = tp->value;
		return true;
	case TTnode::Unknown:
		beta = min(tp->value, beta);
		break;
	case TTnode::FailHigh:
		alpha = max(tp->value, alpha);
		break;
	}
	if (alpha >= beta) {
		value = tp->value;
		return true;
	}
	return false;
#else
	return false;
#endif
}

inline void UpdateTP(Key zobrist, int turn, int depth, int alpha, int beta, int value) {
#ifndef TRANSPOSITION_DISABLE
	uint64_t index = ZobristToIndex(zobrist);
	TTnode *tp = &transpositTable[turn][index];
	//if (transpositTable[index].zobrist == (zobrist >> 32) && transpositTable[index].depth > depth)
	//	return;
	tp->zobrist = zobrist >> 32;
	tp->value = value;
	tp->depth = depth;
	//tp->depth = (value >= CHECKMATE ? SCHAR_MAX : depth);
	if (value < alpha) {
		tp->state = TTnode::Unknown;
	}
	else if (value >= beta) {
		tp->state = TTnode::FailHigh;
	}
	else {
		tp->state = TTnode::Exact;
	}
#endif
}
#else

namespace Transposition {
	uint64_t TPSize = 1 << 26;
	uint64_t TPMask = TPSize - 1;
	TTnode* transpositTable = nullptr;

	void Initialize() {
#ifndef TRANSPOSITION_DISABLE
		transpositTable = new TTnode[TPSize];
		TPMask = TPSize - 1;
		Clean();
		cout << "Transposition Table Created. ";
		cout << "Used Size : " << ((TPSize * sizeof(TTnode)) >> 20) << "MiB\n";
#else
		TPSize = 1;
		TPMask = TPSize - 1;
		transpositTable = new TTnode[TPSize];
		cout << "Transposition Table disable.\n";
#endif
	}

	void Clean() {
#ifndef TRANSPOSITION_DISABLE
		if (transpositTable == nullptr)
			return;
		memset(transpositTable, 0, TPSize * sizeof(TTnode));
#else
		if (transpositTable != nullptr) {
			transpositTable[0] = { 0 };
		}
		cout << "Transposition Table disable.\n";
#endif
	}

	TTnode* Probe(Key key, bool &ttHit) {
#ifndef TRANSPOSITION_DISABLE
        Observer::data[Observer::DataType::ttProbe]++;
		uint64_t index = ZobristToIndex(key);
		if (transpositTable[index].key32 != key >> 32) {
			ttHit = false;
			return &transpositTable[index];
		}
        Observer::data[Observer::DataType::ttHit]++;
		ttHit = true;
		return &transpositTable[index];
#else
		ttHit = false;
		return &transpositTable[0];
#endif
	}
}
#endif