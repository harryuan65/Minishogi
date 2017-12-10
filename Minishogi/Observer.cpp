#include "Observer.h"

// Global Variables 
unsigned int Observer::searchNum = 0;

unsigned long long Observer::totalNode = 0;
unsigned long long Observer::failedHighNode = 0;
unsigned long long Observer::quieNode = 0;
unsigned long long Observer::scoutGeneNums = 0;
unsigned long long Observer::scoutSearchBranch = 0;
unsigned long long Observer::cutIllgalBranch = 0;
double Observer::searchTime = 0;

extern unsigned int Observer::gameNum = 0;
extern unsigned int Observer::whiteWinNum = 0;

unsigned long long Observer::all_totalNode = 0;
unsigned long long Observer::all_failedHighNode = 0;
unsigned long long Observer::all_quieNode = 0;
unsigned long long Observer::all_scoutGeneNums = 0;
unsigned long long Observer::all_scoutSearchBranch = 0;
unsigned long long Observer::all_cutIllgalBranch = 0;
double Observer::all_searchTime = 0;

bool Observer::isAutoSaveKifu = false;
bool Observer::isAutoSaveDetail = false;
bool Observer::isAutoSaveAIReport = false;
