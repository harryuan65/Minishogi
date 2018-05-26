#ifndef _OBSERVER_
#define _OBSERVER_
#include <iomanip>
#include <vector>
using namespace std;

namespace Observer {
	enum DataType {
		searchNum,
		totalNode,
		researchNode,
		quiesNode,
		scoutGeneNums,
		scoutSearchBranch,
		totalTPDepth,
		ios_read,
		ios_write,
		ttProbe,
        ttHit,
		searchTime,
		COUNT
	};

	enum Winner {
		PLAYER1,
		PLAYER2
	};

	// 單一盤面搜尋結果
	extern unsigned long long data[DataType::COUNT];
	static clock_t beginTime;

	// 整局結果
	extern unsigned long long game_data[DataType::COUNT];
	extern unsigned int kifuHash;
	extern Winner winner;

	// 全部結果
	extern unsigned long long total_data[DataType::COUNT];
	extern unsigned int gameNum;
	extern unsigned int player1WinNum;
	extern unsigned int player2WinNum;
	extern vector<Winner> winnerTable1;
	extern vector<Winner> winnerTable2;
	extern vector<uint64_t> initHash;

	// 設定
	extern int DEPTH;
	extern int depth;
	extern bool isSaveRecord;

	inline void StartSearching() {
		for (int i = 0; i < DataType::COUNT; i++)
			data[i] = 0;
		beginTime = clock();
	}

	inline void PauseSearching() {
		data[DataType::searchTime] = clock() - beginTime;
	}

	inline void ResumeSearching() {
		beginTime = clock();
	}

	inline void EndSearching() {
		data[DataType::searchNum]++;
		data[DataType::searchTime] = clock() - beginTime;
		for (int i = 0; i < DataType::COUNT; i++)
			game_data[i] += data[i];
	}

	inline void GameStart() {
		for (int i = 0; i < DataType::COUNT; i++)
			game_data[i] = 0;
	}

	inline void GameOver(bool _winner, bool isSwap, unsigned __int64 _initHash, unsigned int _kifuHash) {
		gameNum++;
		winner = (Winner)(_winner != isSwap);
		kifuHash = _kifuHash;
		if (winner == PLAYER1)
			player1WinNum++;
		else if (winner == PLAYER2)
			player2WinNum++;
		if (!isSwap) {
			winnerTable1.push_back(winner);
			initHash.push_back(_initHash);
		}
		else {
			winnerTable2.push_back(winner);
		}
		for (int i = 0; i < DataType::COUNT; i++)
			total_data[i] += game_data[i];
	}

	inline void PrintSearchReport(std::ostream &os) {
		if (data[DataType::searchNum] == 0) return;
		if (data[DataType::scoutGeneNums] == 0) data[DataType::scoutGeneNums] = 1;
		os << setiosflags(ios::fixed) << setprecision(2);
		os << "Search Report :\n";
		os << " Search Deapth           : " << setw(10) << depth << "\n";
		os << " Total search nodes      : " << setw(10) << data[DataType::totalNode] << "\n";
		os << " Research nodes          : " << setw(10) << data[DataType::researchNode] << "\n";
		os << " Quies search nodes      : " << setw(10) << data[DataType::quiesNode] << "\n";
		os << " Avg scout search branch : " << setw(13) << (float)data[DataType::scoutSearchBranch] / data[DataType::scoutGeneNums] << "\n";
		os << " Total TP Depth          : " << setw(10) << data[DataType::totalTPDepth] << "\n";
		os << " Isomorphic  (read)      : " << setw(10) << data[DataType::ios_read] << "\n";
		os << " Isomorphic  (write)     : " << setw(10) << data[DataType::ios_write] << "\n";
		os << " Index Hit nums          : " << setw(10) << data[DataType::ttHit] << "\n";
        os << " Hit rate                : " << setw(13) <<
            (100.0f * data[DataType::ttHit] / data[DataType::ttProbe]) << " %\n";
		os << " Search time             : " << setw(13) << (float)data[DataType::searchTime] / 1000 << "\n";
		os << endl;
	}


	inline void PrintGameReport(ostream &os) {
		os << "Game" << setw(3) << gameNum - 1 << "\n";
		os << "Game Result :\n";
		os << " Winner                  : " << setw(10) << (winner ? "Player2" : "Player1") << "\n";
		os << " Kifu hashcode           : " << setw(10) << hex << kifuHash << dec << "\n";
		os << " Search depths           : " << setw(10) << DEPTH << "\n";
		os << " Search nums             : " << setw(10) << game_data[DataType::searchNum] << "\n";

		if (game_data[DataType::searchNum] != 0) {
			os << setiosflags(ios::fixed) << setprecision(2);
			os << "Average Report (per search) :\n";
			os << " Total search nodes      : " << setw(10) << game_data[DataType::totalNode] / game_data[DataType::searchNum] << "\n";
			os << " Research nodes          : " << setw(10) << game_data[DataType::researchNode] / game_data[DataType::searchNum] << "\n";
			os << " Quies search nodes      : " << setw(10) << game_data[DataType::quiesNode] / game_data[DataType::searchNum] << "\n";
			os << " Avg scout search branch : " << setw(13) << (float)game_data[DataType::scoutSearchBranch] / game_data[DataType::scoutGeneNums] << "\n";
			os << " Total TP Depth          : " << setw(10) << game_data[DataType::totalTPDepth] / game_data[DataType::searchNum] << "\n";
			os << " Isomorphic  (read)      : " << setw(10) << game_data[DataType::ios_read] / game_data[DataType::searchNum] << "\n";
			os << " Isomorphic  (write)     : " << setw(10) << game_data[DataType::ios_write] / game_data[DataType::searchNum] << "\n";
			os << " Index Hit nums          : " << setw(10) << game_data[DataType::ttHit] / game_data[DataType::searchNum] << "\n";
			os << " Hit rate                : " << setw(13) <<
                (100.0f * game_data[DataType::ttHit] / game_data[DataType::ttProbe]) << " %\n";
			os << " Search time             : " << setw(13) << (float)game_data[DataType::searchTime] / game_data[DataType::searchNum] / 1000 << "\n";
		}
		os << endl;
	}

	inline void PrintTotalReport(ostream &os) {
		if (gameNum == 0) return;
		os << "Game Result :\n";
		os << " Search depths           : " << setw(10) << DEPTH << "\n";
		os << " Search nums             : " << setw(10) << total_data[DataType::searchNum] << "\n";
		if (total_data[DataType::searchNum] != 0) {
			os << setiosflags(ios::fixed) << setprecision(2);
			os << "Average Report (per search) :\n";
			os << " Total search nodes      : " << setw(10) << total_data[DataType::totalNode] / total_data[DataType::searchNum] << "\n";
			os << " Research nodes          : " << setw(10) << total_data[DataType::researchNode] / total_data[DataType::searchNum] << "\n";
			os << " Quies search nodes      : " << setw(10) << total_data[DataType::quiesNode] / total_data[DataType::searchNum] << "\n";
			os << " Avg scout search branch : " << setw(13) << (float)total_data[DataType::scoutSearchBranch] / total_data[DataType::scoutGeneNums] << "\n";
			os << " Total TP Depth          : " << setw(10) << total_data[DataType::totalTPDepth] / total_data[DataType::searchNum] << "\n";
			os << " Isomorphic  (read)      : " << setw(10) << total_data[DataType::ios_read] / total_data[DataType::searchNum] << "\n";
			os << " Isomorphic  (write)     : " << setw(10) << total_data[DataType::ios_write] / total_data[DataType::searchNum] << "\n";
			os << " Index Hit nums          : " << setw(10) << total_data[DataType::ttHit] / total_data[DataType::searchNum] << "\n";
			os << " Hit rate                : " << setw(13) <<
                (100.0f * total_data[DataType::ttHit] / total_data[DataType::ttProbe]) << " %\n";
			os << " Search time             : " << setw(13) << (float)total_data[DataType::searchTime] / total_data[DataType::searchNum] / 1000 << "\n";
		}
		os << endl;
	}

	inline void PrintWinnerReport(ostream &os) {
		if (gameNum == 0) return;
		os << "Total Result :\n";
		os << " Game play nums          : " << setw(10) << gameNum << "\n";
		os << " Player 1 win nums       : " << setw(10) << player1WinNum << "\n";
		os << " Player 2 win nums       : " << setw(10) << player2WinNum << "\n";
		os << "Game |    Init Borad    | A | B | Game\n";
		for (int i = 0; i < winnerTable1.size(); i++) {
			os << setw(4) << i << " | " << hex << setw(16) << initHash[i] << dec << " | ";
			os << (winnerTable1[i] ? "-" : "+") << " | ";
			if (i < winnerTable2.size()) {
				os << (winnerTable2[i] ? "-" : "+") << " | ";
				os << setw(4) << i + winnerTable1.size();
			}
			os << "\n";
		}
		os << endl;
	}
}
#endif
