#include "AI.h"
#define IDAS_START_DEPTH 3
#define IDAS_END_DEPTH   3

typedef void(*genmove)(const Board &, const int, Action *, int &);
static const genmove move_func[] = { MoveGenerator, HandGenerator };

static map<U32, TranspositNode> transpositTable;

Action IDAS(Board& board, bool turn) {
	for (int d = IDAS_START_DEPTH; d <= IDAS_END_DEPTH; d++) {
		cout << "IDAS Searching " << d << " Depth" << endl; //LOG:
		cout << "Leave Evaluate : " << NegaScout(board, -INT_MAX, INT_MAX, d, turn, false) << endl;
	}
	Action action = transpositTable[board.GetHashcode(turn)].bestAction;
	if (ACTION_TO_TURN(action) != turn) {
		return IsomorphismAction(action);
	}
	return action & ACTION_MASK;
}

int NegaScout(Board& board, int alpha, int beta, int depth, bool turn, bool isFailHigh) {
	if (!board.bitboard[KING] || !board.bitboard[KING | BLACKCHESS])
		return -CHECKMATE;
	Action bestAction;
	int bestScore = -INT_MAX;
	int n = beta; 
	TranspositNode tNode;
	if (ReadTransposit(board.GetHashcode(turn), tNode)) {
		if (ACTION_TO_DEPTH(tNode.bestAction) >= depth && ACTION_TO_ISEXACT(tNode.bestAction))
			return tNode.bestScore;
		/*else {
			bestScore = tNode.bestScore;
			alpha = tNode.bestScore;
			n = alpha + 1;
			bestAction = tNode.bestAction & ACTION_MASK;
		}*/
	}
	/* 終止盤面 */
	if (depth == 0) {
		return -QuiescenceSearch(board, alpha, beta, turn ^ 1);
	}

	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 2; i++) {
		Action moveList[MAX_MOVE_NUM];
		int cnt = 0;
		move_func[i](board, turn, moveList, cnt);
		
		/* 對所有移動可能做搜尋 */
		for (int j = 0; j < cnt; j++) {
			board.DoMove(moveList[j]);
			int score = -NegaScout(board, -n, -alpha, depth - 1, turn ^ 1, isFailHigh);
			if (score > bestScore) {
				// depth<3(因為結果差異不大) || score>=beta(因為發生cutoff) || n==beta(因為first node不用null window)
				if (depth < 3 || score >= beta || n == beta) {
					bestScore = score;
				}
				else {
					bestScore = -NegaScout(board, -beta, -score, depth - 1, turn ^ 1, true);
				}
				bestAction = moveList[j];
				alpha = bestScore;
				n = alpha + 1;
			}
			board.UndoMove();
			/* Beta Cut off */
			if (bestScore >= beta) {
				UpdateTransposit(board.GetHashcode(turn), bestScore, false, turn, depth, bestAction);
				return bestScore;
			}
		}
	}
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

bool ReadTransposit(U32 hashcode, TranspositNode& bestNode) {
	if (transpositTable.find(hashcode) != transpositTable.end()) {
		bestNode = transpositTable[hashcode];
		//TODO: if board.IsMovable(bestNode.bestAction) collision avoid
		return true;
	}
	return false;
}

void UpdateTransposit(U32 hashcode, int score, bool isExact, bool turn, int depth, Action action) {
	transpositTable[hashcode] = TranspositNode(score, isExact, turn, depth, action);
}

Action IsomorphismAction(Action action) {
	int srcIndex = ACTION_TO_SRCINDEX(action),
		dstIndex = ACTION_TO_DSTINDEX(action),
		srcChess = ACTION_TO_SRCCHESS(action),
		dstChess = ACTION_TO_DSTCHESS(action);
	if (srcIndex > 24) {
		srcIndex = srcIndex > 30 ? srcIndex - 6 : srcIndex + 6;
		dstIndex = 24 - dstIndex;
		dstChess ^= BLACKCHESS;
	}
	else if (dstIndex > 24) {
		srcIndex = 24 - srcIndex;
		dstIndex = dstIndex > 30 ? dstIndex - 6 : dstIndex + 6;
		srcChess ^= BLACKCHESS;
	}
	else {
		srcIndex = 24 - srcIndex;
		dstIndex = 24 - dstIndex;
		srcChess ^= BLACKCHESS;
		dstChess ^= BLACKCHESS;
	}
	return (dstChess << 18) | (srcChess << 12) | (dstIndex << 6) | srcIndex;
}

bool PrintPV(Board& board, bool turn) {
	U32 hashcode;
	int moves = 0;
	for (;;) {
		hashcode = board.GetHashcode(turn);
		if (transpositTable.find(hashcode) == transpositTable.end() || moves > IDAS_END_DEPTH){
			break;
		}
		TranspositNode node = transpositTable[hashcode];
		cout << (ACTION_TO_DEPTH(node.bestAction)) << " ";
		PrintAction(node.bestAction);
		board.DoMove(node.bestAction & ACTION_MASK);
		moves++;
		turn ^= 1;
	}
	for (int i = 0; i < moves; i++) {
		board.UndoMove();
	}
	return true;
}