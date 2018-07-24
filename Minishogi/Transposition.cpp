#include "Transposition.h"
using std::cout;

namespace Transposition {
	uint64_t TPSize = 1 << 27;
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
		cout << "Transposition Table Cleaned.\n";
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