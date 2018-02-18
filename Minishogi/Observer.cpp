#include "Observer.h"

namespace Observer {
	// 單一盤面搜尋結果
	unsigned long long data[DataType::COUNT];

	// 整局結果
	unsigned long long game_data[DataType::COUNT];

	unsigned long long startZobristHash;
	unsigned int kifuHash;
	bool isPlayer1Win;
	double gamePlayTime;

	// 全部結果
	unsigned long long total_data[DataType::COUNT] = { 0 };
	unsigned int gameNum = 0;
	unsigned int player1WinNum = 0;

	// 設定
	int Observer::depth = 10;
	bool Observer::isSaveRecord = true;
}