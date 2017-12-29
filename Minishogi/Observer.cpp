#include "Observer.h"

namespace Observer {
	// 單一盤面搜尋結果
	unsigned long long totalNode;
	unsigned long long researchNode;
	unsigned long long quiesNode;
	unsigned long long scoutGeneNums;
	unsigned long long scoutSearchBranch;
	unsigned long long cutIllgalBranch;
	double searchTime;

	// 整局結果
	unsigned long long startZobristHash;
	unsigned int kifuHash;
	unsigned int searchNum;
	bool winner;
	double gamePlayTime;

	unsigned long long game_totalNode;
	unsigned long long game_researchNode;
	unsigned long long game_quiesNode;
	unsigned long long game_scoutGeneNums;
	unsigned long long game_scoutSearchBranch;
	unsigned long long game_cutIllgalBranch;
	double game_searchTime;

	// 全部
	unsigned int gameNum;
	unsigned int whiteWinNum;

	// 設定
	int Observer::depth = 10;
	bool Observer::isAutoSaveKifu = false;
	bool Observer::isAutoSaveDetail = false;
	bool Observer::isAutoSaveAIReport = false;
}