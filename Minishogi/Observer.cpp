#include "Observer.h"

namespace Observer {
	// ��@�L���j�M���G
	unsigned long long data[DataType::COUNT];
	double searchTime;

	// �㧽���G
	unsigned long long startZobristHash;
	unsigned int kifuHash;
	unsigned int searchNum;
	bool winner;
	double gamePlayTime;

	unsigned long long game_data[DataType::COUNT];
	double game_searchTime;

	// ����
	unsigned int gameNum;
	unsigned int whiteWinNum;

	// �]�w
	int Observer::depth = 10;
	bool Observer::isSaveRecord = true;
}