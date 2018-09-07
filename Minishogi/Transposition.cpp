#include "Transposition.h"
#include "Observer.h"
using std::cout;

Transposition::~Transposition() {
	if (transpositTable != nullptr)
		delete transpositTable;
}

void Transposition::Initialize(int ttBit) {
	assert(ttBit > 0);
	if (transpositTable != nullptr)
		delete transpositTable;
#ifndef TRANSPOSITION_DISABLE
	ttSize = 1 << (uint64_t)ttBit;
	ttMask = ttSize - 1;
	transpositTable = new TTentry[ttSize];
	Clean();
	cout << "Transposition Table Created. ";
	cout << "Used Size : " << ((ttSize * sizeof(TTentry)) >> 20) << "MiB\n";
#else
	tpSize = 1;
	tpMask = tpSize - 1;
	transpositTable = new TTentry[tpSize];
	cout << "Transposition Table disable.\n";
#endif
}

void Transposition::Clean() {
	cout << "Transposition Table Cleaned.\n";
	if (transpositTable != nullptr)
		memset(transpositTable, 0, ttSize * sizeof(TTentry));
}

/*TTentry* Transposition::Probe(Key key, bool &ttHit) {
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
}*/

TTentry* Transposition::Probe(Key key, int turn, bool &ttHit) {
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
}