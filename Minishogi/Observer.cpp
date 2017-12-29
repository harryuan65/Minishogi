#include "Observer.h"

namespace Observer {
	// ��@�L���j�M���G
	unsigned long long totalNode;
	unsigned long long researchNode;
	unsigned long long quiesNode;
	unsigned long long scoutGeneNums;
	unsigned long long scoutSearchBranch;
	unsigned long long cutIllgalBranch;
	double searchTime;

	// �㧽���G
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

	// ����
	unsigned int gameNum;
	unsigned int whiteWinNum;

	// �]�w
	int Observer::depth = 10;
	bool Observer::isAutoSaveKifu = false;
	bool Observer::isAutoSaveDetail = false;
	bool Observer::isAutoSaveAIReport = false;
}