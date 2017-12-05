#include "AI.h"

U64 nodes;
U64 failed_nodes;
U64 leave_nodes;
U32 pvEvaluate;
U32 max_move[3] = { 0 };

typedef void(*genmove)(Board &, Action *, U32 &);
static const genmove move_func[] = { AttackGenerator, MoveGenerator, HandGenerator };

map<U32, TranspositNode> transpositTable;

Action IDAS(Board& board, bool turn) {
    PV pv;
    cout << "IDAS Searching " << IDAS_END_DEPTH << " Depth" << endl; //LOG:
	pvEvaluate = NegaScout(pv, board, -INT_MAX, INT_MAX, IDAS_END_DEPTH, turn, false);
    printf("\n");
	for (U32 i = 0; i < pv.count; ++i)
		printf("%d %2d -> %2d\n", i, ACTION_TO_SRCINDEX(pv.action[i]), ACTION_TO_DSTINDEX(pv.action[i]));

    if (pv.count == 0) return 0;
	return pv.action[0];
}

int NegaScout(PV &pv, Board &board, int alpha, int beta, int depth, int turn, bool isFailHigh) {
    ++nodes;
    failed_nodes += isFailHigh;
    // using fail soft with negamax:
    // terminal
    if (depth == 0) {
        leave_nodes += !isFailHigh;
        pv.count = 0;
        return board.Evaluate();//QuiescenceSearch(board, alpha, beta, turn);
    }

    int bestScore = -INT_MAX;
    int n = beta;
    PV tempPV;
    U32 accCnt = 0;
    /* 分三個步驟搜尋 [攻擊 移動 打入] */
    for (int i = 0; i < 3; i++) {
        Action moveList[MAX_MOVE_NUM];
        U32 cnt = 0;
        move_func[i](board, moveList, cnt);
        accCnt += cnt;
        //if (cnt > max_move[i])
        //    printf("%d %d:%d ", board.GetTurn(), i, (max_move[i] = cnt));

        for (U32 j = 0; j < cnt; ++j) {
			if (board.IsSennichite(moveList[j]) &&
				!board.IsStillChecking(ACTION_TO_SRCINDEX(moveList[j]), ACTION_TO_DSTINDEX(moveList[j])))
				continue;

            /* Search Depth */
            board.DoMove(moveList[j]);
            //if (board.IsChecking()) cout << "Do\n";
            int score = -NegaScout(tempPV, board, -n, -alpha, depth - 1, turn ^ 1, isFailHigh);
            if (score > bestScore) {
                if (depth < 3 || score >= beta || n == beta)
                    bestScore = score;
                else
                    bestScore = -NegaScout(tempPV, board, -beta, -score + 1, depth - 1, turn ^ 1, true);

                pv.action[0] = moveList[j];
                memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
                pv.count = tempPV.count + 1;
            }
            //if (board.IsChecking()) cout << "Undo\n";
            board.UndoMove();
            if (bestScore >= beta) return bestScore; // cut off
            n = max(alpha, bestScore) + 1; // set up a null window
        }
    }
	if (!accCnt) {
		pv.count = 0;
		leave_nodes += !isFailHigh;
		return -CHECKMATE - depth * 10;
	}
    return bestScore;
}


//int NegaScout(PV &pv, Board& board, int alpha, int beta, int depth, bool turn, bool isFailHigh) {
//	nodes++;
//	failed_nodes += isFailHigh;
//
//	if (!board.bitboard[KING] || !board.bitboard[KING | BLACKCHESS]) {
//        leave_nodes += !isFailHigh;
//		pv.count = 0;
//        return -CHECKMATE;
//    }
//
//	/*TranspositNode tNode;
//	if (ReadTransposit(board.GetHashcode(turn), tNode)) {
//		if (ACTION_TO_DEPTH(tNode.bestAction) >= depth)
//			return tNode.bestScore;
//		else {
//			nw = tNode.bestScore;
//		}
//	}*/
//	/* 終止盤面 */
//	if (depth == 0) {
//        leave_nodes += !isFailHigh;
//		pv.count = 0;
//		return QuiescenceSearch(board, alpha, beta, turn);
//	}
//
//	int bestScore = INT_MIN;
//	int nw = beta;  // Null Window Alpha
//	PV tempPV;
//
//	/* 分三個步驟搜尋 [攻擊 移動 打入] */
//	for (int i = 0; i < 3; i++) {
//		Action moveList[MAX_MOVE_NUM];
//		int cnt = 0;
//		/* Generate Function */
//		//TODO : moveGene 全部
//        move_func[i](board, moveList, cnt);
//
//		/* 對所有移動可能做搜尋 */
//		for (int j = 0; j < cnt; j++) {
//
//			/* Search Depth */
//            board.DoMove(moveList[j]);
//
//            //int score = -NegaScout(pv, board, -nw, -alpha, depth - 1, turn ^ 1, isFailHigh);
//            //if (score > bestScore) {
//            //    // depth<3(因為結果差異不大) || score>=beta(因為發生cutoff) || n==beta(因為first node不用null window)
//            //    if (depth < 3 || score >= beta || nw == beta) {
//            //        bestScore = score;
//            //    }
//            //    else {
//            //        bestScore = -NegaScout(pv, board, -beta, -score, depth - 1, turn ^ 1, true);
//            //    }
//            //    pv.action[0] = moveList[j];
//            //    memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
//            //    pv.count = tempPV.count + 1;
//            //    alpha = bestScore;
//            //    nw = alpha + 1;
//            //}
//
//			int score = -NegaScout(tempPV, board, -nw, -max(alpha, bestScore), depth - 1, turn ^ 1, isFailHigh);
//			if (score > bestScore) {
//				// depth<3(因為結果差異不大) || score>=beta(因為發生cutoff) || nw==beta(因為1st node不用null window)
//				if (score >= beta || depth < 3 || nw == beta) {
//					bestScore = score;
//				}
//				else {
//					/* 發生Failed-High */
//					bestScore = -NegaScout(tempPV, board, -beta, -score + 1, depth - 1, turn ^ 1, true);
//				}
//
//                pv.action[0] = moveList[j];
//                memcpy(pv.action + 1, tempPV.action, tempPV.count * sizeof(Action));
//                pv.count = tempPV.count + 1;
//			}
//            board.UndoMove();
//
//			/* Beta Cut off */
//			if (bestScore >= beta) {
//				//UpdateTransposit(board.GetHashcode(turn), bestScore, false, depth, pv.action[0]);
//				return bestScore;
//			}
//			/* Set up a null window */
//			nw = max(alpha, bestScore) + 1;
//		}
//	}
//    //if (bestScore != INT_MIN)
//		//UpdateTransposit(board.GetHashcode(turn), bestScore, true, depth, pv.action[0]);
//	return bestScore;
//}

int QuiescenceSearch(Board& board, int alpha, int beta, int turn) {
    // terminal
    ++nodes;
    if (!board.bitboard[KING] || !board.bitboard[KING | BLACKCHESS])
        return -CHECKMATE;

    int bestScore = board.Evaluate();
    if (bestScore >= beta) return bestScore;

    int n = beta;
    U32 moveList[MAX_MOVE_NUM] = { BLANK };
    U32 cnt = 0;
    AttackGenerator(board, moveList, cnt);

    for (U32 i = 0; i < cnt; i++) {
        board.DoMove(moveList[i]);
        int score = -QuiescenceSearch(board, -n, -max(alpha, bestScore), 1 - turn);

        if (score > bestScore)
            bestScore = ((n == beta) || (score >= beta)) ? score : \
            - QuiescenceSearch(board, -beta, -score + 1, 1 - turn);

        board.UndoMove();
        if (bestScore >= beta) return bestScore; // cut off
        n = max(alpha, bestScore) + 1; // set up a null window
    }
    return bestScore;
}


//TODO : 改成negascout 加同形表?
//int QuiescenceSearch(Board& board, int alpha, int beta, bool turn) {
//	int bestScore = board.Evaluate();
//	if (bestScore >= beta) return bestScore;
//
//	Action moveList[MAX_MOVE_NUM];
//	int cnt = 0;
//	//TODO : moveGene 吃掉 將軍 解將軍
//
//	for (int i = 0; i < cnt; i++) {
//		if (ACTION_TO_DSTCHESS(moveList[i]) || SEE(ACTION_TO_DSTINDEX(moveList[i]), turn ^ 1) > 0) {
//			board.DoMove(moveList[i]);
//			bestScore = max(bestScore, -QuiescenceSearch(board, max(-beta, bestScore), -alpha, turn ^ 1));
//			board.UndoMove();
//			if (bestScore >= beta)
//				return bestScore;
//		}
//	}
//	
//	return bestScore;
//}

int SEE(int dstIndex, int turn) {
	vector<int> myMoveChess, opMoveChess;
	int exchangeScore = 0;
	//TODO : 
	if (opMoveChess.size() == 0)
		return 0;
	//TODO : 
	if (myMoveChess.size() == 0)
		return 0;

	for (U32 i = 0; i < opMoveChess.size(); i++) {
		exchangeScore += CHESS_SCORE[opMoveChess[i]];
		if (i < myMoveChess.size()) {
			exchangeScore += CHESS_SCORE[myMoveChess[i]];
		}
		else {
			break;
		}
	}
	return exchangeScore;
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