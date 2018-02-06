#include "Zobrist.h"

Zobrist::HalfZobrist* Zobrist::key[37];

void Zobrist::Initialize() {
	srand(Zobrist::SEED);
	for (int i = 0; i < 25; i++) {
		Zobrist::key[i] = new HalfZobrist[30];
		for (int j = 0; j < 30; j++)
			Zobrist::key[i][j] = rand() | (rand() << 16);
	}
	for (int i = 25; i < 37; i++) {
		Zobrist::key[i] = new HalfZobrist[3];
		for (int j = 0; j < 3; j++)
			Zobrist::key[i][j] = rand() | (rand() << 16);
	}
	//rand() ^ (rand()<<8) ^ (rand()<<16);

	cout << "Zobrist Table Created. Random Seed : " << SEED << "\n";
}