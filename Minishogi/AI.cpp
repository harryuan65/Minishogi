#include "AI.h"

U32 max_move[3] = { 0 };

typedef void(*genmove)(Board &, Action *, U32 &);
static const genmove move_func[] = { AttackGenerator, MoveGenerator, HandGenerator };

map<U32, TranspositNode> transpositTable;

Action IDAS(Board& board, PV &pv) {
    cout << "IDAS Searching " << Observer::depth << " Depth..." << endl;
	pv.leafEvaluate = NegaScout(pv, board, -CHECKMATE, CHECKMATE, Observer::depth, false);
	cout << "pvleaf : " << pv.leafEvaluate << endl;
    if (pv.count == 0 || pv.leafEvaluate <= -CHECKMATE)
		return 0;
	return pv.action[0];
}

int NegaScout(PV &pv, Board &board, int alpha, int beta, int depth, bool isResearch) {
	Observer::totalNode++;
	Observer::researchNode += isResearch;

    if (depth == 0) {
        pv.count = 0;
		//Observer::totalNode--; //不然會重複計數
		return board.Evaluate();//QuiescenceSearch(board, alpha, beta, turn);
    }

    int bestScore = -CHECKMATE;
    int n = beta;
    PV tempPV;
    U32 accCnt = 0;
	Observer::scoutGeneNums++;
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

            /* Search Depth */
            board.DoMove(moveList[j]);
			if (!board.IsSennichite() /*||DoMove()前被將軍 也就是被連將(被連將攻擊者判輸)*/ ) {
				int score = -NegaScout(tempPV, board, -n, -max(alpha, bestScore), depth - 1, isResearch);
				if (score > bestScore) {
					if (depth < 3 || score >= beta || n == beta)
						bestScore = score;
					else
						bestScore = -NegaScout(tempPV, board, -beta, -score + 1, depth - 1, true);
					// Save PV
					pv.action[0] = moveList[j];
					pv.evaluate[0] = -board.Evaluate();
					memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
					memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
					pv.count = tempPV.count + 1;
				}
				else if (n == beta && score == -CHECKMATE) {
					// Save PV
					pv.action[0] = moveList[j];
					pv.evaluate[0] = -board.Evaluate();
					memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
					memcpy(pv.evaluate + 1, tempPV.evaluate, tempPV.count * sizeof(int));
					pv.count = tempPV.count + 1;
				}
			}
            board.UndoMove();
			if (bestScore >= beta) {
				Observer::scoutSearchBranch += accCnt + j;
				return bestScore; // cut off
			}
			n = max(alpha, bestScore) + 1; // set up a null window
        }
    }
	if (!accCnt) {
		pv.count = 0;
#ifdef PERFECT_ENDGAME_PV
		return -CHECKMATE - 10 * depth;
#else
		return -CHECKMATE;
#endif
	}
	Observer::scoutSearchBranch += accCnt;
    return bestScore;
}

int QuiescenceSearch(Board& board, int alpha, int beta) {
	Observer::totalNode++;
	Observer::quiesNode++;

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

bool ReadTransposit(U32 hashcode, TranspositNode& bestNode) {
	if (transpositTable.find(hashcode) != transpositTable.end()) {
		bestNode = transpositTable[hashcode];
		//TODO: if board.IsMovable(bestNode.bestAction) collision avoid
		return true;
	}
	return false;
}

void UpdateTransposit(U32 hashcode, int score, bool isExact, U32 depth, Action action) {
	map<U32, TranspositNode>::iterator it = transpositTable.find(hashcode);
	if (it != transpositTable.end() && depth >= ACTION_TO_DEPTH(it->second.bestAction)) {
		it->second = TranspositNode(score, isExact, depth, action);
	}
}
