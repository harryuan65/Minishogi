#include "Observer.h"

namespace Observer {
	// ��@�L���j�M���G
	unsigned long long data[DataType::COUNT];

	// �㧽���G
	unsigned long long game_data[DataType::COUNT];

	unsigned long long startZobristHash;
	unsigned int kifuHash;
	Winner winner;
	double gamePlayTime;

	// �������G
	unsigned long long total_data[DataType::COUNT] = { 0 };
	unsigned int gameNum = 0;
	unsigned int player1WinNum = 0;
	unsigned int player2WinNum = 0;
	vector<Winner> winnerTable1;
	vector<Winner> winnerTable2;
	vector<unsigned __int64> initHash;

	// �]�w
	int depth = 10;
	int limitTime = 2500;
	bool isSaveRecord = true;
	string playDetailStr = "";
}