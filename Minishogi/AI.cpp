#include "AI.h"
#define IDAS_START_DEPTH 7
#define IDAS_END_DEPTH   11

map<U32, TranspositNode> transpositTable;

Action IDAS(Board& board, bool turn) {
	Action bestAction;
	for (int d = IDAS_START_DEPTH; d <= IDAS_END_DEPTH; d++) {
		cout << "IDAS Searching " << d << " Depth" << endl; //LOG:
		NegaScout(board, INT_MIN, INT_MAX, d, turn, false);
	}
	return bestAction;
}

int NegaScout(Board& board, int alpha, int beta, int depth, bool turn, bool isFailHigh) {
	//nodes++;
	//failed_nodes += isFailHigh;
	int bestScore = INT_MIN;
	Action bestAction;
	int nwAlpha = beta;  // Null Window Alpha

	TranspositNode tNode;
	if (ReadTransposit(board.GetHashcode(turn), tNode)) {
		if (ACTION_TO_DEPTH(tNode.bestAction) >= depth)
			return tNode.bestScore;
		else {
			beta = tNode.bestScore;
		}
	}
	/* 終止盤面 */
	if (depth == 0) {
		//if (!isFailHigh)
		//	leave_nodes++;
		return QuiescenceSearch(board, alpha, beta, turn);
	}

	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 3; i++) {
		vector<Action> moveList;
		/* Generate Function */
		//TODO : moveGene 全部

		/* 對所有移動可能做搜尋 */
		for (int j = 0; j < moveList.size(); j++) {
			/* Set up a null window */
			nwAlpha = max(alpha, bestScore);

			/* Search Depth */
			board.DoMove(moveList[j]);
			int score = -NegaScout(board, -nwAlpha - 1, -nwAlpha, depth - 1, !turn, isFailHigh);
			if (score > bestScore) {
				// depth<3(因為結果差異不大) || score>=beta(因為發生cutoff) || nwAlpha==alpha(因為1st node不用null window)
				if (depth < 3 || score >= beta || nwAlpha == alpha) {
					bestScore = score;
				}
				else {
					/* 發生Failed-High */
					bestScore = -NegaScout(board, -beta, -score, depth - 1, !turn, true);
				}
				bestAction = moveList[j];
			}
			board.UndoMove();

			/* Beta Cut off */
			if (bestScore >= beta) {
				UpdateTransposit(board.GetHashcode(turn), bestScore, false, depth, bestAction);
				return bestScore;
			}
		}
	}
	UpdateTransposit(board.GetHashcode(turn), bestScore, true, depth, bestAction);
	return bestScore;
}

int QuiescenceSearch(Board& board, int alpha, int beta, bool turn) {
	vector<Action> moveList;
	int bestScore = INT_MIN;
	//TODO : moveGene 吃掉 將軍 解將軍

	for (Action action : moveList) {
		if ((action & DST_CHESS_MASK) >> 18 || SEE((action & DST_INDEX_MASK) >> 6, turn ^ 1) > 0) {
			board.DoMove(action);
			bestScore = max(bestScore, -QuiescenceSearch(board, max(-beta, bestScore), -alpha, turn ^ 1));
			board.UndoMove();
			if (bestScore >= beta)
				return bestScore;
		}
	}
	if (bestScore == INT_MIN) {
		return board.Evaluate(turn);
	}
	return bestScore;
}

int SEE(int dstIndex, bool turn) {
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

bool ReadTransposit(U32 hashcode, TranspositNode& bestNode) {
	if (transpositTable.find(hashcode) != transpositTable.end()) {
		bestNode = transpositTable[hashcode];
		//TODO: if board.IsMovable(bestNode.bestAction) collision avoid
		return true;
	}
	return false;
}

void UpdateTransposit(U32 hashcode, int score, bool isExact, int depth, Action action) {
	map<U32, TranspositNode>::iterator it = transpositTable.find(hashcode);
	if (it != transpositTable.end() && depth >= ACTION_TO_DEPTH(it->second.bestAction)) {
		it->second = TranspositNode(score, isExact, depth, action);
	}
}