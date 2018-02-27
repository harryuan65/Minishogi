#include "AI.h"
static TransPosition* transpositTable = nullptr;

/*    Negascout Algorithm    */

typedef void(*genmove)(Board &, Action *, U32 &);
static const genmove move_func[] = { AttackGenerator, MoveGenerator, HandGenerator };

Action IDAS(Board& board, PV &pv) {
	cout << "IDAS Searching " << Observer::depth << " Depth..." << endl;
	pv.leafEvaluate = NegaScout(pv, board, -INT_MAX, INT_MAX, Observer::depth, false);
	if (pv.count == 0 || pv.leafEvaluate <= -CHECKMATE)
		return ACTION_SURRENDER;
	return pv.action[0];
}

int NegaScout(PV &pv, Board &board, int alpha, int beta, int depth, bool isResearch) {
	Observer::data[Observer::DataType::totalNode]++;
	Observer::data[Observer::DataType::researchNode] += isResearch;
	Observer::data[Observer::DataType::scoutGeneNums]++;
	pv.count = 0;

	int bestScore = -INT_MAX;
	int n = beta;
	PV tempPV;
	U32 accCnt = 0;
	Zobrist::Zobrist zobrist = board.GetZobristHash();

	if (ReadTP(zobrist, depth, alpha, beta, bestScore, board)) {
		return bestScore;
	}

	if (depth == 0) {
		//Observer::data[Observer::DataType::totalNode]--; //不然會重複計數
		return board.GetEvaluate();//QuiescenceSearch(board, alpha, beta, turn);
	}
	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 3; i++) {
		Action moveList[MAX_MOVE_NUM];
		U32 cnt = 0;
		move_func[i](board, moveList, cnt);
		accCnt += cnt;

		for (U32 j = 0; j < cnt; ++j) {
			if (board.IsCheckAfter(ACTION_TO_SRCINDEX(moveList[j]), ACTION_TO_DSTINDEX(moveList[j]))) {
				accCnt--;
				continue;
			}

			board.DoMove(moveList[j]);
			if (board.IsSennichite() /*&& !(DoMove()前被將軍)*/) {
				/* 千日手 且 沒有被將軍 (連將時攻擊者判輸) TODO : 判斷是否被將軍 */
				board.UndoMove();
				accCnt--;
				continue;
			}
			int score = -NegaScout(tempPV, board, -n, -max(alpha, bestScore), depth - 1, isResearch);
			if (score > bestScore) {
				if (depth < 3 || score >= beta || n == beta)
					bestScore = score;
				else
					bestScore = -NegaScout(tempPV, board, -beta, -score + 1, depth - 1, true);
#ifndef PV_DISABLE
				pv.action[0] = moveList[j];
				pv.evaluate[0] = -board.GetEvaluate();
				memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
				memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
				pv.count = tempPV.count + 1;
#else
				if (depth == Observer::depth) {
					pv.action[0] = moveList[j];
					pv.evaluate[0] = -board.GetEvaluate();
					memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
					memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
					pv.count = tempPV.count + 1;
				}
#endif
			}
#ifndef PV_DISABLE
			else if (pv.count == 0 && score == -CHECKMATE) {
				/* moveList的第一個action是必輸的話照樣儲存pv 才能在必輸下得到pv */
				pv.action[0] = moveList[j];
				pv.evaluate[0] = -board.GetEvaluate();
				memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
				memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
				pv.count = tempPV.count + 1;
		}
#endif
			board.UndoMove();

			if (bestScore >= beta) {
				/* Beta cutoff */
				Observer::data[Observer::DataType::scoutSearchBranch] += accCnt + j;
				UpdateTP(zobrist, depth, alpha, beta, bestScore, board);
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
	UpdateTP(zobrist, depth, alpha, beta, bestScore, board);
	Observer::data[Observer::DataType::scoutSearchBranch] += accCnt;
	return bestScore;
}

int QuiescenceSearch(Board& board, int alpha, int beta) {
	Observer::data[Observer::DataType::totalNode]++;
	Observer::data[Observer::DataType::quiesNode]++;

	//int bestScore = board.Evaluate();
	//if (bestScore >= beta) return bestScore;

	int bestScore = -CHECKMATE;
	int n = beta;
	U32 accCnt = 0;

	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 3; i++) {
		Action moveList[MAX_MOVE_NUM];
		U32 cnt = 0;
		move_func[i](board, moveList, cnt);
		accCnt += cnt;

		for (U32 j = 0; j < cnt; ++j) {
			if (board.IsCheckAfter(ACTION_TO_SRCINDEX(moveList[j]), ACTION_TO_DSTINDEX(moveList[j]))
				/*|| !我是否被將軍 || !動完是否將軍對方(moveList[j]) || !(i == 0 && SEE >= 0)*/) {
				accCnt--;
				continue;
			}

			/* Search Depth */
			board.DoMove(moveList[j]);
			int score = -QuiescenceSearch(board, -n, -max(alpha, bestScore));
			if (score > bestScore) {
				if (score >= beta || n == beta)
					bestScore = score;
				else
					bestScore = -QuiescenceSearch(board, -beta, -score + 1);
			}
			board.UndoMove();
			if (bestScore >= beta) {
				return bestScore; // cut off
			}
			n = max(alpha, bestScore) + 1; // set up a null window
		}
	}
	/*U32 cnt = 0;
	AttackGenerator(board, moveList, cnt);
	for (U32 i = 0; i < cnt; i++) {
	board.DoMove(moveList[i]);
	int score = -QuiescenceSearch(board, -n, -max(alpha, bestScore), 1 - turn);
	if (score > bestScore)
	bestScore = ((n == beta) || (score >= beta)) ? score : \
	- QuiescenceSearch(board, -beta, -score + 1, 1 - turn);
	board.UndoMove();
	if (bestScore >= beta) {
	return bestScore; // cut off
	}
	n = max(alpha, bestScore) + 1; // set up a null window
	}*/
	if (!accCnt) return -CHECKMATE;
	return bestScore;
}

int SEE(const Board &board, int dstIndex) {
	int exchangeScore = CHESS_SCORE[board.board[dstIndex]];
	vector<int> myMoveChess, opMoveChess;

	// [ Start Add opMoveChess ]
	const U32 psbboard = (RookMove(board, dstIndex) | BishopMove(board, dstIndex));
	U32 srcboard = psbboard & board.occupied[board.GetTurn() ^ 1],
		dstboard = 1 << dstIndex;
	while (srcboard) {
		U32 attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (Movable(board, attsrc) & dstboard)
			opMoveChess.push_back(CHESS_SCORE[board.board[attsrc]]);
	}
	if (opMoveChess.size() == 0) return board.GetTurn() ? -exchangeScore : exchangeScore;
	// [ End Add opMoveChess ]

	// [ Start myMoveChess ]
	srcboard = psbboard & board.occupied[board.GetTurn()];
	dstboard = 1 << dstIndex;
	while (srcboard) {
		U32 attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (Movable(board, attsrc) & dstboard)
			myMoveChess.push_back(CHESS_SCORE[board.board[attsrc]]);
	}
	if (myMoveChess.size() == 0) return board.GetTurn() ? -exchangeScore : exchangeScore;
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
	board.PrintChessBoard();
	cout << " to: " << dstIndex << " see: " << (board.GetTurn() ? -exchangeScore : exchangeScore) << endl;
	system("pause");
	}*/
	return board.GetTurn() ? -exchangeScore : exchangeScore;
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

bool ReadTP(Zobrist::Zobrist zobrist, int depth, int& alpha, int& beta, int& value, Board &board) {
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
	//我原本的
	/*if (transpositTable[index].state == TransPosition::Unknown) {
		// Value在(-Infinity, value]
		beta = min(transpositTable[index].value, beta);
		return false;
	}
	if (transpositTable[index].value >= beta) {
		// 自己發生Failed High
		value = transpositTable[index].value;
		return true;
	}
	if (transpositTable[index].state == TransPosition::FailHigh) {
		//Failed-High
		alpha = max(transpositTable[index].value, alpha);
		return false;
	}
	// Exact
	value = transpositTable[index].value;
	return true;*/
#else
	return false;
#endif
}

void UpdateTP(Zobrist::Zobrist zobrist, int depth, int alpha, int beta, int value, Board &board) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);

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

void PrintPV(ostream &os, Board &board, int depth) {
	/*int i;
	os << "PV: (depth | turn | action | my evaluate)" << "\n";
	for (i = 0; i < depth; i++) {
		U64 index = ZobristToIndex(board.GetZobristHash());
		if (transpositTable[index].zobrist != board.GetZobristHash() >> 32) {
			break;
		}
		os << i << " : " << (board.GetTurn() ? "▼" : "△");
		os << Index2Input(ACTION_TO_SRCINDEX(transpositTable[index].action))
			<< Index2Input(ACTION_TO_DSTINDEX(transpositTable[index].action))
			<< (ACTION_TO_ISPRO(transpositTable[index].action) ? "+" : " ");
		os << setw(7) << (i % 2 ? -board.GetEvaluate() : board.GetEvaluate()) << "\n";
		board.DoMove(transpositTable[index].action);
	}
	for (; i > 0; i--) {
		board.UndoMove();
	}
	if (leafEvaluate <= -CHECKMATE || CHECKMATE <= leafEvaluate) {
		os << count << " : " << (((turn + count) & 1) ? "▼" : "△") << "Lose " << setw(7) << leafEvaluate << "\n";
	}*/
}