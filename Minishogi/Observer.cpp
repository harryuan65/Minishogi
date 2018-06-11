#include "Observer.h"

namespace Observer {
	// 單一盤面搜尋結果
	unsigned long long data[DataType::COUNT];

	// 整局結果
	unsigned long long game_data[DataType::COUNT];

	unsigned long long startZobristHash;
	unsigned int kifuHash;
	Winner winner;
	double gamePlayTime;

	// 全部結果
	unsigned long long total_data[DataType::COUNT] = { 0 };
	unsigned int gameNum = 0;
	unsigned int player1WinNum = 0;
	unsigned int player2WinNum = 0;
	vector<Winner> winnerTable1;
	vector<Winner> winnerTable2;
	vector<unsigned __int64> initHash;

	// 設定
	int depth = 10;
	int limitTime = 2500;
	bool isSaveRecord = true;
	string playDetailStr = "";
}