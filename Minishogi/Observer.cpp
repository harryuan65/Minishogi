#include "Observer.h"

namespace Observer {
	// 單一盤面搜尋結果
	unsigned long long data[DataType::COUNT];

	// 整局結果
	unsigned long long game_data[DataType::COUNT];

	unsigned long long startZobristHash;
	Winner winner;
	double gamePlayTime;

	// 全部結果
	unsigned long long total_data[DataType::COUNT] = { 0 };
	unsigned int gameNum = 0;
	unsigned int player1WinNum = 0;
	unsigned int player2WinNum = 0;
	vector<Winner> winnerTable1;
	vector<Winner> winnerTable2;
	vector<uint64_t> initHash;
	vector<uint32_t> kifuHash1;
	vector<uint32_t> kifuHash2;

	// 設定
	int depth = 10;
	int limitTime = 0;
	bool isSaveRecord = true;
}