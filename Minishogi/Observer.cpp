#include "Observer.h"

// Global Variables 
unsigned int Observer::searchNum = 0;

unsigned long long Observer::totalNode = 0;
unsigned long long Observer::researchNode = 0;
unsigned long long Observer::quieNode = 0;
unsigned long long Observer::scoutGeneNums = 0;
unsigned long long Observer::scoutSearchBranch = 0;
unsigned long long Observer::cutIllgalBranch = 0;
double Observer::searchTime = 0;

unsigned int Observer::gameNum = 0;
unsigned int Observer::whiteWinNum = 0;
vector<bool> Observer::winner;
vector<unsigned int> Observer::kifuHash;

unsigned long long Observer::all_totalNode = 0;
unsigned long long Observer::all_researchNode = 0;
unsigned long long Observer::all_quieNode = 0;
unsigned long long Observer::all_scoutGeneNums = 0;
unsigned long long Observer::all_scoutSearchBranch = 0;
unsigned long long Observer::all_cutIllgalBranch = 0;
double Observer::all_searchTime = 0;

int Observer::depth = 10;
bool Observer::isAutoSaveKifu = false;
bool Observer::isAutoSaveDetail = false;
bool Observer::isAutoSaveAIReport = false;
