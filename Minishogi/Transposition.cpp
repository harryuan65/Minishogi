#include "Transposition.h"
using std::cout;

namespace Transposition {
	uint64_t TPSize = 1 << 27;
	uint64_t TPMask = TPSize - 1;
	TTentry* transpositTable = nullptr;

	void Initialize() {
#ifndef TRANSPOSITION_DISABLE
		transpositTable = new TTentry[TPSize];
		TPMask = TPSize - 1;
		Clean();
		cout << "Transposition Table Created. ";
		cout << "Used Size : " << ((TPSize * sizeof(TTentry)) >> 20) << "MiB\n";
#else
		TPSize = 1;
		TPMask = TPSize - 1;
		transpositTable = new TTentry[TPSize];
		cout << "Transposition Table disable.\n";
#endif
	}

	void Clean() {
#ifndef TRANSPOSITION_DISABLE
		cout << "Transposition Table Cleaned.\n";
		if (transpositTable == nullptr)
			return;
		memset(transpositTable, 0, TPSize * sizeof(TTentry));
#else
		if (transpositTable != nullptr) {
			transpositTable[0] = { 0 };
		}
		cout << "Transposition Table disable.\n";
#endif
	}

	TTentry* Probe(Key key, bool &ttHit) {
#ifndef TRANSPOSITION_DISABLE
        Observer::data[Observer::DataType::ttProbe]++;
		uint64_t index = ZobristToIndex(key);
		if (transpositTable[index].key32 != key >> 32) {
			Observer::data[Observer::DataType::ttCollision] += (transpositTable[index].key32 != 0);
			ttHit = false;
			return &transpositTable[index];
		}
		ttHit = true;
		return &transpositTable[index];
#else
		ttHit = false;
		return &transpositTable[0];
#endif
	}

	TTentry* Probe(Key key, int turn, bool &ttHit) {
#ifndef TRANSPOSITION_DISABLE
		Observer::data[Observer::DataType::ttProbe]++;
		uint64_t index = ZobristToIndex(key);
		if (transpositTable[index].key32 != key >> 32) {
			Observer::data[Observer::DataType::ttCollision] += (transpositTable[index].key32 != 0);
			ttHit = false;
			return &transpositTable[index];
		}
		ttHit = true;
		Observer::data[Observer::DataType::ttIsoNum] += (transpositTable[index].turn != turn);
		return &transpositTable[index];
#else
		ttHit = false;
		return &transpositTable[0];
#endif
	}
}