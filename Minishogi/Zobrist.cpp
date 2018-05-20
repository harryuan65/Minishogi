#include <iostream>
#include "Zobrist.h"

namespace Zobrist {
	Key table[SQUARE_NB][CHESS_NB];
	Key table2[SQUARE_NB][CHESS_NB];

	void Initialize() {
		std::mt19937_64 gen(SEED);
		std::uniform_int_distribution<Key> dist;

		int i = 0, j;
		for (; i < BOARD_NB; ++i) {
			for (j = 0; j < CHESS_NB; ++j) {
				table[i][j] = dist(gen);
				if ((j^BLACKCHESS) < CHESS_NB) {
					table2[BOARD_NB - 1 - i][j^BLACKCHESS] = table[i][j];
				}
			}
		}
		for (; i < SQUARE_NB; ++i) {
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
		std::cout << "Zobrist Table Created. Random Seed : " << SEED << "\n";
	}
}