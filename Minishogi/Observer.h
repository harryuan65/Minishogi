#ifndef _OBSERVER_
#define _OBSERVER_
#include <iostream>
#include <time.h>
#include <iomanip>
using namespace std;

namespace Observer {
	extern unsigned int searchNum;

	extern unsigned long long totalNode;
	extern unsigned long long failedHighNode;
	extern unsigned long long quieNode;
	extern unsigned long long scoutGeneNums;
	extern unsigned long long scoutSearchBranch;
	extern unsigned	long long cutIllgalBranch;
	extern double searchTime;

	extern unsigned int gameNum;
	extern unsigned int whiteWinNum;

	extern unsigned long long all_totalNode;
	extern unsigned long long all_failedHighNode;
	extern unsigned long long all_quieNode;
	extern unsigned long long all_scoutGeneNums;
	extern unsigned long long all_scoutSearchBranch;
	extern unsigned	long long all_cutIllgalBranch;
	extern double all_searchTime;

	extern bool isAutoSaveKifu;
	extern bool isAutoSaveDetail;
	extern bool isAutoSaveAIReport;


	static clock_t beginTime;

	inline void StartSearching() {
		totalNode = 0;
		failedHighNode = 0;
		quieNode = 0;
		scoutGeneNums = 0;
		scoutSearchBranch = 0;
		cutIllgalBranch = 0;
		searchTime = 0;

		beginTime = clock();
	}

	inline void EndSearching() {
		searchTime = double(clock() - beginTime) / 1000;

		searchNum++;

		all_totalNode += totalNode;
		all_failedHighNode += failedHighNode;
		all_quieNode += quieNode;
		all_scoutGeneNums += scoutGeneNums;
		all_scoutSearchBranch += scoutSearchBranch;
		all_cutIllgalBranch += cutIllgalBranch;
		all_searchTime += searchTime;
	}

	inline void GameOver(bool isWhiteWin) {
		gameNum++;
		whiteWinNum += isWhiteWin;
	}

	inline void PrintReport(std::ostream &os) {
		if (scoutGeneNums == 0)
			scoutGeneNums = 1;
		os << "Search Report :" << endl;
		os << "Total search nodes      : " << setw(10) << totalNode << "\n";
		os << "Failed-high nodes       : " << setw(10) << failedHighNode << "\n";
		os << "Quie search nodes       : " << setw(10) << quieNode << "\n";
		os << "Avg scout search branch : " << setw(13) << (float)scoutSearchBranch / scoutGeneNums << setiosflags(ios::fixed) << setprecision(2) << "\n";
		os << "Cut illgal branch       : " << setw(10) << cutIllgalBranch << "\n";
		os << "Search time             : " << setw(13) << searchTime << setiosflags(ios::fixed) << setprecision(2) << "\n";
	}


	inline void PrintAvgReport(ostream &os) {
		if (searchNum == 0)
			return;
		os << "Average Report :" << endl;
		os << "Search nums             : " << setw(10) << searchNum << "\n";
		os << "Total search nodes      : " << setw(10) << all_totalNode / searchNum << "\n";
		os << "Failed-high nodes       : " << setw(10) << all_failedHighNode / searchNum << "\n";
		os << "Quie search nodes       : " << setw(10) << all_quieNode / searchNum << "\n";
		os << "Avg scout search branch : " << setw(13) << (float)all_scoutSearchBranch / all_scoutGeneNums << setiosflags(ios::fixed) << setprecision(2) << "\n";
		os << "Cut illgal branch       : " << setw(10) << all_cutIllgalBranch / searchNum << "\n";
		os << "Search time             : " << setw(13) << all_searchTime / searchNum << setiosflags(ios::fixed) << setprecision(2) << "\n";
	}
}
#endif
