#include "Zobrist.h"

Zobrist::Zobrist Zobrist::table[35][30];
Zobrist::Zobrist Zobrist::table2[35][30];

void Zobrist::Initialize() {
    std::mt19937_64 gen(Zobrist::SEED);
    std::uniform_int_distribution<Zobrist> dist;

    int i = 0, j;
    for (; i < 25; ++i) {
        for (j = 0; j < 30; ++j) {
			table[i][j] = dist(gen);
			if ((j^BLACKCHESS) < 30) {
				table2[24 - i][j^BLACKCHESS] = table[i][j];
			}
        }
    }
    for (; i < 35; ++i) {
        for (j = 0; j < 3; ++j) {
			table[i][j] = dist(gen);
			if (i < 30) {
				table2[i + 5][j] = table[i][j];
			}
			else {
				table2[i - 5][j] = table[i][j];
			}
        }
    }
	std::cout << "Zobrist Table Created. Random Seed : " << Zobrist::SEED << "\n";
}