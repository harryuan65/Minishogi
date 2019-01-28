#include "Transposition.h"
#include "Observer.h"

using std::cout;

Transposition GlobalTT;

Transposition::~Transposition() {
	if (transpositTable)
		delete transpositTable;
}

void Transposition::Resize(int ttBit) {
	if (transpositTable)
		delete transpositTable;
#ifndef TRANSPOSITION_DISABLE
	ttSize = (uint64_t)1 << ttBit;
	ttMask = ttSize - 1;
	transpositTable = new TTentry[ttSize];
	cout << "Transposition Table Created ";
	cout << "Use Size : " << ((ttSize * sizeof(TTentry)) >> 20) << " MiB\n";
	Clean();
#else
	ttSize = 1;
	ttMask = ttSize - 1;
	transpositTable = new TTentry[ttSize];
	cout << "Transposition Table disable\n";
#endif
}

void Transposition::Clean() {
	if (transpositTable) {
		memset(transpositTable, 0, ttSize * sizeof(TTentry));
		cout << "Transposition Table Cleaned\n";
	}
}

TTentry* Transposition::Probe(Key key, bool &ttHit) {
#ifdef TRANSPOSITION_DISABLE
	ttHit = false;
	return &transpositTable[0];
#endif
    Observer::data[Observer::DataType::ttProbe]++;
	uint64_t index = ZobristToIndex(key);
	if (transpositTable[index].key32 != key >> 32) {
		Observer::data[Observer::DataType::ttCollision] += (transpositTable[index].key32 != 0);
		ttHit = false;
		return &transpositTable[index];
	}
	ttHit = true;
	return &transpositTable[index];
}

/*TTentry* Transposition::Probe(Key key, int turn, bool &ttHit) {
#ifdef TRANSPOSITION_DISABLE
	ttHit = false;
	return &transpositTable[0];
#endif
	Observer::data[Observer::DataType::ttProbe]++;
	uint64_t index = ZobristToIndex(key);
	if (transpositTable[index].key32 != key >> 32) {
		Observer::data[Observer::DataType::ttCollision] += (transpositTable[index].key32 != 0);
		ttHit = false;
		return &transpositTable[index];
	}
	ttHit = true;
	//Observer::data[Observer::DataType::ttIsoNum] += (transpositTable[index].turn != turn);
	return &transpositTable[index];
}*/

int Transposition::HashFull() const {
	int cnt = 0;
	for (int i = 0; i < 1000; i++)
		cnt += (transpositTable[i].key32 != 0);
	return cnt;
}