#include "AI.h"
#define IDAS_START_DEPTH 5
#define IDAS_END_DEPTH   9

typedef void(*genmove)(const Board &, const int, Action *, int &);
static const genmove move_func[] = { MoveGenerator, HandGenerator };

static map<U64, TranspositNode> transpositTable;
//line pv;

Action IDAS(Board& board, bool turn) {
	for (int d = IDAS_START_DEPTH; d <= IDAS_END_DEPTH; d++) {
		cout << "IDAS Searching " << d << " Depth" << endl; //LOG:
		cout << "Leave Evaluate : " << NegaScout(/*&pv,*/ board, -INT_MAX, INT_MAX, d, turn, false) << endl;
	}
	// Check PV
	/*for (int i = 0; i < pv.pv_count; i++) {
		PrintAction(pv.pv[i]);
	}*/
	PrintPV(board, turn, IDAS_END_DEPTH);

	Action action = transpositTable[board.GetHashcode(turn)].bestAction;
	if (ACTION_TO_TURN(action) != turn) {
		return IsomorphismAction(action);
	}
	return action & ACTION_MASK;
}

int NegaScout(/*line *mPVAction,*/ Board& board, int alpha, int beta, int depth, bool turn, bool isFailHigh) {
	if (!board.bitboard[KING] || !board.bitboard[KING | BLACKCHESS]) {
		//mPVAction->pv_count = 0;
		return -CHECKMATE;
	}
	Action bestAction;
	Action moveList[MAX_MOVE_NUM];
	int cnt = 0;
	int bestScore = -INT_MAX;
	int n = beta; 
	TranspositNode tNode;
	//line tmpPVAction;
	bool isLoad = false;
	if (!isFailHigh && ReadTransposit(board.GetHashcode(turn), tNode)) {
		if (ACTION_TO_DEPTH(tNode.bestAction) >= depth && ACTION_TO_ISEXACT(tNode.bestAction) && !board.IsSennichite(tNode.bestAction)) {
			//mPVAction->pv[0] = tNode.bestAction;
			//mPVAction->pv_count = 0;
			return tNode.bestScore;
		}
		else if (tNode.bestScore > alpha){
			moveList[0] = tNode.bestAction & ACTION_MASK;
			if (ACTION_TO_TURN(tNode.bestAction) != turn) {
				moveList[0] = IsomorphismAction(moveList[0]);
			}
			//if (board.board[ACTION_TO_SRCINDEX(moveList[0])] != BLANK &&
			//	((board.board[ACTION_TO_SRCINDEX(moveList[0])] & BLACKCHESS) >> 4) == turn) {
				cnt++;
				isLoad = true;
			//}
		}
	}
	/* 終止盤面 */
	if (depth == 0) {
		//mPVAction->pv_count = 0;
		return -QuiescenceSearch(board, alpha, beta, turn ^ 1);
	}

	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 2; i++) {
		move_func[i](board, turn, moveList, cnt);
	}
	int j = 0;
	if (isLoad) {
		for (int i = 1; i < cnt; i++) {
			if (moveList[i] == moveList[0]) {
				j = 0;
				break;
			}
			if (i == cnt - 1) {
				j = 1;
			}
		}
	}
	/* 對所有移動可能做搜尋 */
	for (; j < cnt; j++) {
		board.DoMove(moveList[j]);
		if (board.record[board.record.size() - 1] != board.record[board.record.size() - 5]) {
			if (!board.CheckChessCount()) {
				board.PrintChessBoard(turn);
				cout << "有棋子消失了" << endl;
			}
			int score = -NegaScout(/*&tmpPVAction,*/ board, -n, -alpha, depth - 1, turn ^ 1, isFailHigh);
			if (score > bestScore) {
				// depth<3(因為結果差異不大) || score>=beta(因為發生cutoff) || n==beta(因為first node不用null window)
				if (/*depth < 3 ||*/ score >= beta || n == beta) {
					bestScore = score;
				}
				else {
					bestScore = -NegaScout(/*&tmpPVAction, */board, -beta, -score, depth - 1, turn ^ 1, true);
				}
				bestAction = moveList[j];
				alpha = bestScore;
				n = alpha + 1;

				//mPVAction->pv[0] = bestAction;
				//memcpy(mPVAction->pv + 1, tmpPVAction.pv, tmpPVAction.pv_count * sizeof(Action));
				//mPVAction->pv_count = tmpPVAction.pv_count + 1;
			}

		}
		board.UndoMove();
		if (!board.CheckChessCount()) {
			board.PrintChessBoard(turn);
			cout << "有棋子消失了" << endl;
		}
		/* Beta Cut off */
		if (bestScore >= beta) {
			UpdateTransposit(board.GetHashcode(turn), bestScore, false, turn, depth, bestAction);
			return bestScore;
		}
	}
	cnt = 0;
	UpdateTransposit(board.GetHashcode(turn), bestScore, true, turn, depth, bestAction);
	return bestScore;
}

//TODO : 改成negascout 加同形表?
int QuiescenceSearch(Board& board, int alpha, int beta, bool turn) {
	if (!board.bitboard[KING] || !board.bitboard[KING | BLACKCHESS])
		return -CHECKMATE;

	Action moveList[MAX_MOVE_NUM];
	int cnt = 0, bestScore = -INT_MAX;
	//TODO : moveGene 吃掉 將軍 解將軍

	for (int i = 0; i < cnt; i++) {
		if (ACTION_TO_DSTCHESS(moveList[i]) || SEE(board, ACTION_TO_DSTINDEX(moveList[i]), turn ^ 1) > 0) {
			board.DoMove(moveList[i]);
			bestScore = max(bestScore, -QuiescenceSearch(board, max(-beta, bestScore), -alpha, turn ^ 1));
			board.UndoMove();
			if (bestScore >= beta)
				return bestScore;
		}
	}
	if (bestScore == -INT_MAX) {
		return board.GetEvaluate(turn);
	}
	return bestScore;
}

int SEE(Board& board, int dstIndex, bool turn) {
	if (!board.bitboard[KING] || !board.bitboard[KING | BLACKCHESS])
		return -CHECKMATE;

	vector<int> myMoveChess, opMoveChess;
	int exchangeScore = 0;
	//TODO : 
	if (opMoveChess.size() == 0)
		return 0;
	//TODO : 
	if (myMoveChess.size() == 0)
		return 0;

	for (int i = 0; i < opMoveChess.size(); i++) {
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

bool ReadTransposit(U64 hashcode, TranspositNode& bestNode) {
	if (transpositTable.find(hashcode) != transpositTable.end()) {
		bestNode = transpositTable[hashcode];
		//TODO: if board.IsMovable(bestNode.bestAction) collision avoid
		return true;
	}
	return false;
}

void UpdateTransposit(U64 hashcode, int score, bool isExact, bool turn, int depth, Action action) {
	if (transpositTable.find(hashcode) == transpositTable.end() ||
		(transpositTable.find(hashcode) != transpositTable.end() && depth >= ACTION_TO_DEPTH(transpositTable[hashcode].bestAction)))
		transpositTable[hashcode] = TranspositNode(score, isExact, turn, depth, action);
}

//註: 不處理Chess ID
Action IsomorphismAction(Action action) {
	int srcIndex = ACTION_TO_SRCINDEX(action),
		dstIndex = ACTION_TO_DSTINDEX(action);
	if (srcIndex > 24) {
		srcIndex = srcIndex > 30 ? srcIndex - 6 : srcIndex + 6;
		dstIndex = 24 - dstIndex;
	}
	else if (dstIndex > 24) {
		srcIndex = 24 - srcIndex;
		dstIndex = dstIndex > 30 ? dstIndex - 6 : dstIndex + 6;
	}
	else {
		srcIndex = 24 - srcIndex;
		dstIndex = 24 - dstIndex;
	}
	return (dstIndex << 6) | srcIndex;
}

void PrintPV(Board& board, bool turn, int depth) {
	U32 hashcode;
	int i = 0;
	bool moverTurn = turn;
	for (i = 0; i < depth; i++) {
		hashcode = board.GetHashcode(turn);
		printf("Hashcode : %11llu Evaluate : %6d\n", hashcode, board.GetEvaluate(moverTurn));
		if (transpositTable.find(hashcode) == transpositTable.end()){
			cout << "ERROR : Can't Find Action" << endl;
			break; 
		}
		TranspositNode node = transpositTable[hashcode];
		PrintAction(node.bestAction);
		board.DoMove(node.bestAction & ACTION_MASK);
		turn ^= 1;
	}
	printf("Hashcode : %11llu Evaluate : %6d\n", board.GetHashcode(turn), board.GetEvaluate(moverTurn));
	for (; i > 0; i--) {
		board.UndoMove();
	}
}