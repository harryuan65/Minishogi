#include "Observer.h"

namespace Observer {
	// 單一盤面搜尋結果
	uint64_t data[DataType::COUNT];

	// 整局結果
	uint64_t game_data[DataType::COUNT];

	uint64_t startZobristHash;
	Winner winner;
	double gamePlayTime;

	// 全部結果
	uint64_t total_data[DataType::COUNT] = { 0 };
	uint32_t gameNum = 0;
	uint32_t player1WinNum = 0;
	uint32_t player2WinNum = 0;
	std::vector<Winner> winnerTable1;
	std::vector<Winner> winnerTable2;
	std::vector<uint64_t> initKey;
	std::vector<uint32_t> kifuHash1;
	std::vector<uint32_t> kifuHash2;

	// 設定
	int depth = 10;
	int ttBit = 27;
	int limitTime = 0;
	bool isSaveRecord = true;
	std::string kpptName = "";
}