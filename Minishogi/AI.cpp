#include "AI.h"

static TransPosition* transpositTable = nullptr;

/*    Negascout Algorithm    */
Action IDAS(Minishogi& minishogi, PV &pv) {
	Action bestAction = ACTION_SURRENDER;
	cout << "IDAS Searching " << Observer::depth << " Depth..." << endl;
#ifdef BEST_ENDGAME_SEARCH
	pv.leafEvaluate = NegaScout(pv, bestAction, minishogi,
		-CHECKMATE - 10 * Observer::depth, CHECKMATE + 10 * Observer::depth, Observer::depth, false, true);
#else
	pv.leafEvaluate = NegaScout(pv, bestAction, minishogi, -CHECKMATE, CHECKMATE, Observer::depth, false, true);
#endif
	if (pv.leafEvaluate <= -CHECKMATE && Observer::isCanSurrender) {
		return ACTION_SURRENDER;
	}
	return bestAction;
}

int NegaScout(PV &pv, Action &bestAction, Minishogi &minishogi, int alpha, int beta, int depth, bool isResearch, bool isTop) {
	Observer::data[Observer::DataType::totalNode]++;
	Observer::data[Observer::DataType::researchNode] += isResearch;
	Observer::data[Observer::DataType::scoutGeneNums]++;
	pv.count = 0;

	int bestScore = -CHECKMATE;
	int n = beta;
	PV tempPV;
	int accCnt = 0;
	Zobrist::Zobrist zobrist = minishogi.GetZobristHash();

	if (!isTop && ReadTP(zobrist, depth, alpha, beta, bestScore)) {
		return bestScore;
	}

	if (depth == 0) {
		//Observer::data[Observer::DataType::totalNode]--; //不然會重複計數
		return minishogi.GetEvaluate();//QuiescenceSearch(minishogi, alpha, beta, turn);
	}

	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 3; i++) {
		Action moveList[MAX_MOVE_NUM], tempAction;
		int cnt = 0;
		switch (i) {
		case 0:
			minishogi.AttackGenerator(moveList, cnt);
			break;
		case 1:
			minishogi.MoveGenerator(moveList, cnt);
			break;
		case 2:
			minishogi.HandGenerator(moveList, cnt);
			break;
		}
		accCnt += cnt;

		for (int j = 0; j < cnt; ++j) {
			if (minishogi.IsCheckAfter(ACTION_TO_SRCINDEX(moveList[j]), ACTION_TO_DSTINDEX(moveList[j]))) {
				accCnt--;
				continue;
			}

			minishogi.DoMove(moveList[j]);
			if (minishogi.IsSennichite() /*&& !(DoMove()前被將軍)*/) {
				/* 千日手 且 沒有被將軍 (連將時攻擊者判輸) TODO : 判斷是否被將軍 */
				minishogi.UndoMove();
				accCnt--;
				continue;
			}
			int score = -NegaScout(tempPV, tempAction, minishogi, -n, -max(alpha, bestScore), depth - 1, isResearch, false);
			if (score > bestScore) {
				if (depth < 3 || score >= beta || n == beta)
					bestScore = score;
				else
					bestScore = -NegaScout(tempPV, tempAction, minishogi, -beta, -score + 1, depth - 1, true, false);
				bestAction = moveList[j];
#ifndef PV_DISABLE
				pv.action[0] = moveList[j];
				pv.evaluate[0] = -minishogi.GetEvaluate();
				memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
				memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
				pv.count = tempPV.count + 1;
#else
				if (depth == Observer::depth) {
					pv.action[0] = moveList[j];
					pv.evaluate[0] = -minishogi.GetEvaluate();
					memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
					memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
					pv.count = tempPV.count + 1;
				}
#endif
			}
#ifndef PV_DISABLE
			else if (pv.count == 0 && score == -CHECKMATE) {
				/* moveList的第一個action是必輸的話照樣儲存pv 才能在必輸下得到pv */
				bestAction = moveList[j];
				pv.action[0] = moveList[j];
				pv.evaluate[0] = -minishogi.GetEvaluate();
				memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
				memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
				pv.count = tempPV.count + 1;
		}
#endif
			minishogi.UndoMove();

			if (bestScore >= beta) {
				/* Beta cutoff */
				Observer::data[Observer::DataType::scoutSearchBranch] += accCnt + j;
				UpdateTP(zobrist, depth, alpha, beta, bestScore);
				return bestScore;
			}
			n = max(alpha, bestScore) + 1; // Set up a null window
	}
}
	if (accCnt == 0) {
#ifdef BEST_ENDGAME_SEARCH
		bestScore = -CHECKMATE - 10 * depth;
#else
		bestScore = -CHECKMATE;
#endif
	}
	UpdateTP(zobrist, depth, alpha, beta, bestScore);
	Observer::data[Observer::DataType::scoutSearchBranch] += accCnt;
	return bestScore;
}

int QuiescenceSearch(Minishogi& minishogi, int alpha, int beta) {
	Observer::data[Observer::DataType::totalNode]++;
	Observer::data[Observer::DataType::quiesNode]++;

	//int bestScore = minishogi.Evaluate();
	//if (bestScore >= beta) return bestScore;

	int bestScore = -CHECKMATE;
	int n = beta;
	int accCnt = 0;

	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 3; i++) {
		Action moveList[MAX_MOVE_NUM];
		int cnt = 0;
		switch (i) {
		case 0:
			minishogi.AttackGenerator(moveList, cnt);
			break;
		case 1:
			minishogi.MoveGenerator(moveList, cnt);
			break;
		case 2:
			minishogi.HandGenerator(moveList, cnt);
			break;
		}
		accCnt += cnt;

		for (int j = 0; j < cnt; ++j) {
			if (minishogi.IsCheckAfter(ACTION_TO_SRCINDEX(moveList[j]), ACTION_TO_DSTINDEX(moveList[j]))
				/*|| !我是否被將軍 || !動完是否將軍對方(moveList[j]) || !(i == 0 && SEE >= 0)*/) {
				accCnt--;
				continue;
			}

			/* Search Depth */
			minishogi.DoMove(moveList[j]);
			int score = -QuiescenceSearch(minishogi, -n, -max(alpha, bestScore));
			if (score > bestScore) {
				if (score >= beta || n == beta)
					bestScore = score;
				else
					bestScore = -QuiescenceSearch(minishogi, -beta, -score + 1);
			}
			minishogi.UndoMove();
			if (bestScore >= beta) {
				return bestScore; // cut off
			}
			n = max(alpha, bestScore) + 1; // set up a null window
		}
	}
	/*U32 cnt = 0;
	AttackGenerator(minishogi, moveList, cnt);
	for (U32 i = 0; i < cnt; i++) {
	minishogi.DoMove(moveList[i]);
	int score = -QuiescenceSearch(minishogi, -n, -max(alpha, bestScore), 1 - turn);
	if (score > bestScore)
	bestScore = ((n == beta) || (score >= beta)) ? score : \
	- QuiescenceSearch(minishogi, -beta, -score + 1, 1 - turn);
	minishogi.UndoMove();
	if (bestScore >= beta) {
	return bestScore; // cut off
	}
	n = max(alpha, bestScore) + 1; // set up a null window
	}*/
	if (!accCnt) return -CHECKMATE;
	return bestScore;
}

int SEE(const Minishogi &minishogi, int dstIndex) {
	int exchangeScore = CHESS_SCORE[minishogi.board[dstIndex]];
	vector<int> myMoveChess, opMoveChess;

	// [ Start Add opMoveChess ]
	const Bitboard psbboard = (minishogi.RookMove(dstIndex) | minishogi.BishopMove(dstIndex));
	Bitboard srcboard = psbboard & minishogi.occupied[minishogi.GetTurn() ^ 1];
	Bitboard dstboard = 1 << dstIndex;
	while (srcboard) {
		int attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (minishogi.Movable(attsrc) & dstboard)
			opMoveChess.push_back(CHESS_SCORE[minishogi.board[attsrc]]);
	}
	if (opMoveChess.size() == 0) return minishogi.GetTurn() ? -exchangeScore : exchangeScore;
	// [ End Add opMoveChess ]

	// [ Start myMoveChess ]
	srcboard = psbboard & minishogi.occupied[minishogi.GetTurn()];
	dstboard = 1 << dstIndex;
	while (srcboard) {
		int attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (minishogi.Movable(attsrc) & dstboard)
			myMoveChess.push_back(CHESS_SCORE[minishogi.board[attsrc]]);
	}
	if (myMoveChess.size() == 0) return minishogi.GetTurn() ? -exchangeScore : exchangeScore;
	// [ End myMoveChess ]

	// [ Start sort ]
	sort(opMoveChess.begin(), opMoveChess.end(), [](const int &a, const int &b) { return abs(a) < abs(b); });
	sort(myMoveChess.begin(), myMoveChess.end(), [](const int &a, const int &b) { return abs(a) < abs(b); });
	// [ End sort ]

	std::vector<int>::iterator op = opMoveChess.begin();
	std::vector<int>::iterator my = myMoveChess.begin();
	for (; op != opMoveChess.end(); op++) {
		exchangeScore += *my;
		if (++my != myMoveChess.end())
			exchangeScore += *op;
		else break;
	}
	// debug 用
	/*if (opMoveChess.size() > 2 && myMoveChess.size() > 2) {
	minishogi.PrintChessBoard();
	cout << " to: " << dstIndex << " see: " << (minishogi.GetTurn() ? -exchangeScore : exchangeScore) << endl;
	system("pause");
	}*/
	return minishogi.GetTurn() ? -exchangeScore : exchangeScore;
}

/*    Transposition Table    */

inline U64 ZobristToIndex(Zobrist::Zobrist zobrist) {
	return zobrist & TPMask;
}

void InitializeTP() {
#ifndef TRANSPOSITION_DISABLE
	transpositTable = new TransPosition[TPSize];
	CleanTP();
	cout << "TransPosition Table Created. ";
	cout << "Used Size : " << ((TPSize * sizeof(TransPosition)) >> 20) << "MiB\n";
#else
	cout << "TransPosition Table disable.\n";
#endif
}

void CleanTP() {
#ifndef TRANSPOSITION_DISABLE
	if (transpositTable == nullptr)
		return;
	for (int i = 0; i < TPSize; i++) {
		transpositTable[i].zobrist = 0;
	}
#else
	cout << "TransPosition Table disable.\n";
#endif
}

bool ReadTP(Zobrist::Zobrist zobrist, int depth, int& alpha, int& beta, int& value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);
	if (transpositTable[index].zobrist != zobrist >> 32) {
		Observer::data[Observer::DataType::indexCollisionNums]++;
		return false;
	}
	if (transpositTable[index].depth < depth) {
		return false;
	}
	Observer::data[Observer::DataType::totalTPDepth] += depth;

	//柏勳新的
	switch (transpositTable[index].state) {
	case TransPosition::Exact:
		value = transpositTable[index].value;
		return true;
	case TransPosition::Unknown:
		beta = min(transpositTable[index].value, beta);
		break;
	case TransPosition::FailHigh:
		alpha = max(transpositTable[index].value, alpha);
		break;
	}
	if (alpha >= beta) {
		value = transpositTable[index].value;
		return true;
	}
	return false;
#else
	return false;
#endif
}

void UpdateTP(Zobrist::Zobrist zobrist, int depth, int alpha, int beta, int value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);

	//if (transpositTable[index].zobrist == (zobrist >> 32) && transpositTable[index].depth > depth)
	//	return;
	transpositTable[index].zobrist = zobrist >> 32;
	transpositTable[index].value = value;
	transpositTable[index].depth = depth;
	if (value < alpha) {
		transpositTable[index].state = TransPosition::Unknown;
	}
	else if (value >= beta) {
		transpositTable[index].state = TransPosition::FailHigh;
	}
	else {
		transpositTable[index].state = TransPosition::Exact;
	}
#endif
}

void PrintPV(ostream &os, Minishogi &minishogi, int depth) {
	/*int i;
	os << "PV: (depth | turn | action | my evaluate)" << "\n";
	for (i = 0; i < depth; i++) {
		U64 index = ZobristToIndex(minishogi.GetZobristHash());
		if (transpositTable[index].zobrist != minishogi.GetZobristHash() >> 32) {
			break;
		}
		os << i << " : " << (minishogi.GetTurn() ? "▼" : "△");
		os << Index2Input(ACTION_TO_SRCINDEX(transpositTable[index].action))
			<< Index2Input(ACTION_TO_DSTINDEX(transpositTable[index].action))
			<< (ACTION_TO_ISPRO(transpositTable[index].action) ? "+" : " ");
		os << " " << (int)transpositTable[index].depth;
		minishogi.DoMove(transpositTable[index].action);
		os << setw(7) << (i % 2 ? minishogi.GetEvaluate() : -minishogi.GetEvaluate()) << "\n";
	}
	for (; i > 0; i--) {
		minishogi.UndoMove();
	}*/
	/*if (leafEvaluate <= -CHECKMATE || CHECKMATE <= leafEvaluate) {
		os << count << " : " << (((turn + count) & 1) ? "▼" : "△") << "Lose " << setw(7) << leafEvaluate << "\n";
	}*/
}