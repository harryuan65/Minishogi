#include "AI.h"

static Action** actionList = nullptr;

/*    Negascout Algorithm    */
void InitializeNS() {
	actionList = new Action*[Observer::DEPTH + 1];
	for (int i = 1; i <= Observer::DEPTH; i++) {
		actionList[i] = new Action[Minishogi::SINGLE_GENE_MAX_ACTIONS];
	}
}

int IDAS(Minishogi& minishogi, Action &bestAction) {
	int eval;
	bestAction.mode = Action::SURRENDER;
	cout << "IDAS Searching " << Observer::depth << " Depth..." << endl;
#ifdef BEST_ENDGAME_SEARCH
	eval = NegaScout(minishogi, bestAction,
		-CHECKMATE - ((Observer::depth - 1) << 2), CHECKMATE + ((Observer::depth - 1) << 2), Observer::depth, false, true);
#else
	eval = NegaScout(minishogi, bestAction, -CHECKMATE, CHECKMATE, Observer::depth, false, true);
#endif
	return eval;
}

int NegaScout(Minishogi &minishogi, Action &bestAction, int alpha, int beta, int depth, bool isResearch, bool isTop) {
	Observer::data[Observer::DataType::totalNode]++;
	Observer::data[Observer::DataType::researchNode] += isResearch;
	Observer::data[Observer::DataType::scoutGeneNums]++;

#ifdef BEST_ENDGAME_SEARCH
	int bestScore = -SHRT_MAX, n = beta, cnt;  //只要比最小的-CHECKMATE - ((Observer::depth - 1) << 2)小就好
#else
	int bestScore = -CHECKMATE, n = beta, cnt; //=alpha(CHECKMATE)最好
#endif
	Zobrist::Zobrist zobrist = minishogi.GetZobristHash();

	if (!isTop && ReadTP(zobrist, minishogi.GetTurn(), depth, alpha, beta, bestScore)) {
		Observer::data[Observer::DataType::ios_read] += minishogi.IsIsomorphic();
		return bestScore;
	}

	if (depth == 0) {
#ifdef QUIES_DISABLE
		return minishogi.GetEvaluate();
#else
		return QuiescenceSearch(minishogi, alpha, beta, Observer::QUIE_DEPTH, 0);
#endif
	}

	Action *moveList = actionList[depth];
	bool isLose = true;
	bool isChecked = minishogi.IsChecked(); // 若此盤面處於被將 解將不考慮千日手

	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 3; i++) {
		cnt = 0;
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

		for (int j = 0; j < cnt; j++) {
			if (minishogi.IsCheckAfter(moveList[j].srcIndex, moveList[j].dstIndex)) {
				continue;
			}

			minishogi.DoMove(moveList[j]);
			if (!isChecked && minishogi.IsSennichite()) {
				minishogi.UndoMove();
				continue;
			}

			Observer::data[Observer::DataType::ios_write] += minishogi.IsIsomorphic();

			isLose = false;
			int score = -NegaScout(minishogi, bestAction, -n, -max(alpha, bestScore), depth - 1, isResearch, false);
			if (score > bestScore) {
				if (depth < 3 || score >= beta || n == beta)
					bestScore = score;
				else
					bestScore = -NegaScout(minishogi, bestAction, -beta, -score + 1, depth - 1, true, false);
				if (isTop) bestAction = moveList[j];
			}
			minishogi.UndoMove();

			if (bestScore >= beta) {
				// Beta cutoff
				//Observer::data[Observer::DataType::scoutSearchBranch] += searchCnt + j;
				UpdateTP(zobrist, minishogi.GetTurn(), depth, alpha, beta, bestScore);
				return bestScore;
			}
			n = max(alpha, bestScore) + 1; // Set up a null window
		}
	}
	if (isLose) {
#ifdef BEST_ENDGAME_SEARCH
		bestScore = -CHECKMATE - (depth << 2);
#else
		bestScore = -CHECKMATE;
#endif
	}
	UpdateTP(zobrist, minishogi.GetTurn(), depth, alpha, beta, bestScore);
	return bestScore;
}

int QuiescenceSearch(Minishogi& minishogi, int alpha, int beta, int depth, int totalDepth) {
	Observer::data[Observer::DataType::quiesNode]++;

	int bestScore = -CHECKMATE;
	Zobrist::Zobrist zobrist = minishogi.GetZobristHash();
	if (ReadQTP(zobrist, minishogi.GetTurn(), totalDepth, alpha, beta, bestScore)) {
		Observer::data[Observer::DataType::ios_read] += minishogi.IsIsomorphic();
		return bestScore;
	}
	if (depth == 0 || totalDepth >= Observer::MAX_QUIE_DEPTH) {
		return minishogi.GetEvaluate();
	}

	bool isLose = true, isQuiePos = true;
	bool isChecked = minishogi.IsChecked();
	/* 分三個步驟搜尋 [攻擊 移動 打入] */
	for (int i = 0; i < 3; i++) {
		Action moveList[Minishogi::SINGLE_GENE_MAX_ACTIONS];
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

		for (int j = 0; j < cnt; j++) {
			if (minishogi.IsCheckAfter(moveList[j].srcIndex, moveList[j].dstIndex)) {
				continue;
			}
			isLose = false;
			//SEE(minishogi, moveList[j].dstIndex);
			int eatenChessValue = CHESS_SCORE[minishogi.board[moveList[j].dstIndex]];
			minishogi.DoMove(moveList[j]);
			if (isChecked || minishogi.IsChecked()) { // 如果是將軍或解將
				if (eatenChessValue - SEE(minishogi, moveList[j].dstIndex) < 0){
					minishogi.UndoMove();
					continue;
				}
			}
			else if (i == 0) { // 如果是攻擊
				if (eatenChessValue - SEE(minishogi, moveList[j].dstIndex) <= 0) {
					minishogi.UndoMove();
					continue;
				}
			}
			else {
				minishogi.UndoMove();
				continue;
			}
			isQuiePos = false;

			int score = -QuiescenceSearch(minishogi, -beta, -max(bestScore, alpha), i == 0 ? Observer::QUIE_DEPTH : depth - 1, totalDepth + 1);
			bestScore = max(bestScore, score);
			minishogi.UndoMove();
			if (bestScore >= beta) {
				UpdateQTP(zobrist, minishogi.GetTurn(), totalDepth, alpha, beta, bestScore);
				return bestScore;
			}
		}
	}
	if (isLose)
		bestScore = -CHECKMATE;
	else if (isQuiePos)
		bestScore = minishogi.GetEvaluate();
	UpdateQTP(zobrist, minishogi.GetTurn(), totalDepth, alpha, beta, bestScore);
	return bestScore;
}

int SEE(const Minishogi &minishogi, int dstIndex) {
	int exchangeScore = CHESS_SCORE[minishogi.board[dstIndex]];
	vector<int> myMoveChess, opMoveChess;

	// [ Start Add opMoveChess ]
	const Bitboard psbboard = (minishogi.RookMovable(dstIndex) | minishogi.BishopMovable(dstIndex));
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

#ifdef DOUBLETP
static TransPosition** transpositTable = nullptr;
void InitializeTP() {
#ifndef TRANSPOSITION_DISABLE
	transpositTable = new TransPosition*[2];
	transpositTable[0] = new TransPosition[TPSize];
	transpositTable[1] = new TransPosition[TPSize];
	CleanTP();
	cout << "TransPosition Table Created. ";
	cout << "Used Size : 2 * " << ((TPSize * sizeof(TransPosition)) >> 20) << "MiB\n";
#else
	cout << "TransPosition Table disable.\n";
#endif
}

void CleanTP() {
#ifndef TRANSPOSITION_DISABLE
	if (transpositTable == nullptr)
		return;
	for (int i = 0; i < TPSize; i++) {
		transpositTable[0][i].zobrist = 0;
		transpositTable[1][i].zobrist = 0;
	}
#else
	cout << "TransPosition Table disable.\n";
#endif
}

inline bool ReadTP(Zobrist::Zobrist zobrist, int turn, int depth, int& alpha, int& beta, int& value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);
	TransPosition *tp = &transpositTable[turn][index];
	if (tp->zobrist != zobrist >> 32) {
		Observer::data[Observer::DataType::indexCollisionNums]++;
		return false;
	}
	if (tp->depth < depth) {
		Observer::data[Observer::DataType::indexCollisionNums]++;
		return false;
	}
	Observer::data[Observer::DataType::totalTPDepth] += depth;

	switch (tp->state) {
	case TransPosition::Exact:
		value = tp->value;
		return true;
	case TransPosition::Unknown:
		beta = min(tp->value, beta);
		break;
	case TransPosition::FailHigh:
		alpha = max(tp->value, alpha);
		break;
	}
	if (alpha >= beta) {
		value = tp->value;
		return true;
	}
	return false;
#else
	return false;
#endif
}

inline void UpdateTP(Zobrist::Zobrist zobrist, int turn, int depth, int alpha, int beta, int value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);
	TransPosition *tp = &transpositTable[turn][index];
	//if (transpositTable[index].zobrist == (zobrist >> 32) && transpositTable[index].depth > depth)
	//	return;
	tp->zobrist = zobrist >> 32;
	tp->value = value;
	tp->depth = depth;
	//tp->depth = (value >= CHECKMATE ? SCHAR_MAX : depth);
	if (value < alpha) {
		tp->state = TransPosition::Unknown;
	}
	else if (value >= beta) {
		tp->state = TransPosition::FailHigh;
	}
	else {
		tp->state = TransPosition::Exact;
	}
#endif
}
#else
static TransPosition* transpositTable = nullptr;
static TransPosition* QtranspositTable = nullptr;
void InitializeTP() {
#ifndef TRANSPOSITION_DISABLE
	transpositTable = new TransPosition[TPSize];
	QtranspositTable = new TransPosition[TPSize];
	CleanTP();
	//TODO : DEBUG
	/*delete[] transpositTable;
	transpositTable = new TransPosition[TPSize];
	CleanTP();*/
	//TODO : DEBUG
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
		QtranspositTable[i].zobrist = 0;
	}
#else
	cout << "TransPosition Table disable.\n";
#endif
}

bool ReadTP(Zobrist::Zobrist zobrist, int turn, int depth, int& alpha, int& beta, int& value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);
	if (transpositTable[index].zobrist != zobrist >> 32) {
		Observer::data[Observer::DataType::indexCollisionNums]++;
		return false;
	}
	if (transpositTable[index].depth < depth) {
		Observer::data[Observer::DataType::indexCollisionNums]++;
		return false;
	}
	Observer::data[Observer::DataType::totalTPDepth] += depth;

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

void UpdateTP(Zobrist::Zobrist zobrist, int turn, int depth, int alpha, int beta, int value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);

	if (transpositTable[index].zobrist == (zobrist >> 32) && transpositTable[index].depth > depth)
		return;
	transpositTable[index].zobrist = zobrist >> 32;
	transpositTable[index].value = value;
	transpositTable[index].depth = depth;
	//transpositTable[index].depth = (value >= CHECKMATE ? SCHAR_MAX : depth);
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

bool ReadQTP(Zobrist::Zobrist zobrist, int turn, int depth, int& alpha, int& beta, int& value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);
	TransPosition *tp = &transpositTable[index];
	if (tp->zobrist != zobrist >> 32) {
		tp = &QtranspositTable[index];
		if (tp->zobrist != zobrist >> 32 || tp->depth < depth) {
			return false;
		}
	}

	switch (tp->state) {
	case TransPosition::Exact:
		value = tp->value;
		return true;
	case TransPosition::Unknown:
		beta = min(tp->value, beta);
		break;
	case TransPosition::FailHigh:
		alpha = max(tp->value, alpha);
		break;
	}
	if (alpha >= beta) {
		value = tp->value;
		return true;
	}
	return false;
#else
	return false;
#endif
}

void UpdateQTP(Zobrist::Zobrist zobrist, int turn, int depth, int alpha, int beta, int value) {
#ifndef TRANSPOSITION_DISABLE
	U64 index = ZobristToIndex(zobrist);

	if (transpositTable[index].zobrist == (zobrist >> 32) && transpositTable[index].depth > depth)
		return;
	QtranspositTable[index].zobrist = zobrist >> 32;
	QtranspositTable[index].value = value;
	QtranspositTable[index].depth = depth;
	if (value < alpha) {
		QtranspositTable[index].state = TransPosition::Unknown;
	}
	else if (value >= beta) {
		QtranspositTable[index].state = TransPosition::FailHigh;
	}
	else {
		QtranspositTable[index].state = TransPosition::Exact;
	}
#endif
}
#endif
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