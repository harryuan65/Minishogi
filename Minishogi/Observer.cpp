#include "Observer.h"

namespace Observer {
	// ��@�L���j�M���G
	uint64_t data[DataType::COUNT];

	// �㧽���G
	uint64_t game_data[DataType::COUNT];

	uint64_t startZobristHash;
	Winner winner;
	double gamePlayTime;

	// �������G
	uint64_t total_data[DataType::COUNT] = { 0 };
	uint32_t gameNum = 0;
	uint32_t player1WinNum = 0;
	uint32_t player2WinNum = 0;
	std::vector<Winner> winnerTable1;
	std::vector<Winner> winnerTable2;
	std::vector<uint64_t> initKey;
	std::vector<uint32_t> kifuHash1;
	std::vector<uint32_t> kifuHash2;

	// �]�w
	int depth = 10;
	int ttBit = 27;
	int limitTime = 0;
	bool isSaveRecord = true;
	std::string kpptName = "";
}