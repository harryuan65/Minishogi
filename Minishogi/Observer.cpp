#include "Observer.h"

namespace Observer {
	// ��@�L���j�M���G
	unsigned long long data[DataType::COUNT];

	// �㧽���G
	unsigned long long game_data[DataType::COUNT];

	unsigned long long startZobristHash;
	unsigned int kifuHash;
	bool isPlayer1Win;
	double gamePlayTime;

	// �������G
	unsigned long long total_data[DataType::COUNT] = { 0 };
	unsigned int gameNum = 0;
	unsigned int player1WinNum = 0;

	// �]�w
	int Observer::depth = 10;
	bool Observer::isSaveRecord = true;
}