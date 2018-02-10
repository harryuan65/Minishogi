#include "Observer.h"

namespace Observer {
	// 單一盤面搜尋結果
	unsigned long long data[DataType::COUNT];
	double searchTime;

	// 整局結果
	unsigned long long startZobristHash;
	unsigned int kifuHash;
	unsigned int searchNum;
	bool winner;
	double gamePlayTime;

	unsigned long long game_data[DataType::COUNT];
	double game_searchTime;

	// 全部
	unsigned int gameNum;
	unsigned int whiteWinNum;

	// 設定
	int Observer::depth = 10;
	bool Observer::isSaveRecord = true;
}