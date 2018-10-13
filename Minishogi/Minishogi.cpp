#include <assert.h>
#include <atlstr.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "Minishogi.h"
#include "Zobrist.h"
#include "Thread.h"
#include "Observer.h"
using namespace std;
using namespace Evaluate;

const string Minishogi::START_SFEN = "rbsgk/4p/5/P4/KGSBR b - 1";

void Minishogi::Initialize() {
	Initialize(START_SFEN);
}

bool Minishogi::Initialize(const Minishogi &m) {
	turn = m.turn;
	ply = m.ply;
	for (int i = 0; i < COLOR_NB; i++) occupied[i] = m.occupied[i];
	for (int i = 0; i < PIECE_NB; i++) bitboard[i] = m.bitboard[i];
	for (int i = 0; i < SQUARE_NB; i++) board[i] = m.board[i];
	for (BonaPieceIndex i = BPI_PAWN; i < BONA_PIECE_INDEX_NB; ++i)
		SetBonaPiece(i, m.pieceList[WHITE][i], m.pieceList[BLACK][i]);

	for (int i = 0; i <= ply; i++) {
		moveHist[i] = m.moveHist[i];
		captureHist[i] = m.captureHist[i];
		bonaPieceDiffHist[i][0] = m.bonaPieceDiffHist[i][0];
		bonaPieceDiffHist[i][1] = m.bonaPieceDiffHist[i][1];
	}
	for (int i = 0; i <= ply; i++) {
		evalHist[i] = m.evalHist[i];
		keyHist[i] = m.keyHist[i];
		key2Hist[i] = m.key2Hist[i];
		checker_bb[i] = m.checker_bb[i];
	}
	return CheckLegal();
}

bool Minishogi::InitializeByBoard(string str) {
	istringstream ss(str);
	string token;
	Square sq = SQUARE_ZERO;

	memset(occupied, 0, COLOR_NB * sizeof(Bitboard));
	memset(bitboard, 0, PIECE_NB * sizeof(Bitboard));
	memset(board, NO_PIECE, SQUARE_NB * sizeof(int));
	for (BonaPieceIndex i = BPI_PAWN; i < BONA_PIECE_INDEX_NB; ++i)
		pieceList[WHITE][i] = BONA_PIECE_NB;

	getline(ss, token);
	if (token[0] == '+')
		turn = WHITE;
	else if (token[0] == '-')
		turn = BLACK;
	else {
		cerr << "Error : Load Board Failed. There is a unrecognized symbol in TURN section.\n";
		return false;
	}
	ply = 0;
	evalHist[0].meterial = VALUE_ZERO;
	keyHist[0] = 0;
	key2Hist[0] = 0;

	for (int i = 0; i < 5; i++) {
		getline(ss, token);
		for (int j = 0, find; j < 5; j++, ++sq) {
			if ((find = NONCOLOR_PIECE_WORD.find(token.substr(j * 4, 4))) != string::npos) {
				if (find == 0)
					continue;
				Piece pc = Piece(find / 4);
				board[sq] = pc;
				SetBonaPiece(sq, pc);
				bitboard[pc] |= 1 << sq;
				occupied[color_of(pc)] |= 1 << sq;
				evalHist[0].meterial += PIECE_SCORE[pc];
				keyHist[0] ^= Zobrist::table[sq][pc];
				key2Hist[0] ^= Zobrist::table2[sq][pc];
			}
			else {
				cerr << "Error : Load Board Failed. There is a unrecognized symbol in BOARD section.\n";
				return false;
			}
		}
	}
	sq = SQ_B_HAND;
	for (int i = 0; i < 2; i++) {
		getline(ss, token);
		for (int j = 0; j < 5; j++, ++sq) {
			int handNum = token[2 + 3 * j] - '0';
			if (handNum == 1 || handNum == 2) {
				board[sq] = handNum;
				SetBonaPiece(sq, Piece(1));
				evalHist[0].meterial += HAND_SCORE[sq] * handNum;
				keyHist[0] ^= Zobrist::table[sq][1];
				key2Hist[0] ^= Zobrist::table2[sq][1];
				if (handNum == 2) {
					SetBonaPiece(sq, Piece(2));
					keyHist[0] ^= Zobrist::table[sq][2];
					key2Hist[0] ^= Zobrist::table2[sq][2];
				}
			}
			else if (handNum != 0) {
				cerr << "Error : Load Board Failed. There is a unrecognized symbol in HAND section.\n";
				return false;
			}
		}
		sq = SQ_W_HAND;
	}
#ifndef ENEMY_ISO_TT
	if (turn == BLACK)	keyHist[0] ^= 1;
#endif

	CalcAllChecker();
	CalcAllPin();
	CalcAllPos();
	return CheckLegal();
}

bool Minishogi::Initialize(std::string sfen) {
	istringstream ss(sfen);
	string buf;
	Square sq = SQUARE_ZERO;
	Piece pc;
	int handNum = 1;
	bool isPromote = false;

	memset(occupied, 0, COLOR_NB * sizeof(Bitboard));
	memset(bitboard, 0, PIECE_NB * sizeof(Bitboard));
	memset(board, NO_PIECE, SQUARE_NB * sizeof(int));
	for (BonaPieceIndex i = BPI_PAWN; i < BONA_PIECE_INDEX_NB; ++i)
		pieceList[WHITE][i] = BONA_PIECE_NB;
	ply = 0;
	evalHist[0].meterial = VALUE_ZERO;
	keyHist[0] = 0;
	key2Hist[0] = 0;

	ss >> buf;
	for (int i = 0; i < buf.size(); i++) {
		if (buf[i] == '/')
			continue;
		else if (isdigit(buf[i]))
			for (int j = 0; j < buf[i] - '0'; ++j, ++sq)
				board[sq] = 0;
		else if (buf[i] == '+')
			isPromote = true;
		else if ((pc = (Piece)PIECE_2_CHAR.find(buf[i])) != (Piece)string::npos) {
			if (isPromote)
				pc = promote(pc);
			board[sq] = pc;
			SetBonaPiece(sq, pc);
			bitboard[pc] |= 1 << sq;
			occupied[color_of(pc)] |= 1 << sq;
			evalHist[0].meterial += PIECE_SCORE[pc];
			keyHist[0] ^= Zobrist::table[sq][pc];
			key2Hist[0] ^= Zobrist::table2[sq][pc];
			isPromote = false;
			++sq;
		}
		else
			return false;
	}

	ss >> buf;
	turn = buf[0] == 'b' ? WHITE : BLACK;

	ss >> buf;
	for (int i = 0; i < buf.size(); i++) {
		if (buf[i] == '-')
			break;
		else if (isdigit(buf[i]))
			handNum = buf[i] - '0';
		else if ((sq = (Square)HAND_2_CHAR.find(buf[i])) != (Square)string::npos){
			sq += BOARD_NB;
			board[sq] = handNum;
			SetBonaPiece(sq, (Piece)1);
			evalHist[0].meterial += HAND_SCORE[sq] * handNum;
			keyHist[0] ^= Zobrist::table[sq][1];
			key2Hist[0] ^= Zobrist::table2[sq][1];
			if (handNum == 2) {
				SetBonaPiece(sq, (Piece)2);
				keyHist[0] ^= Zobrist::table[sq][2];
				key2Hist[0] ^= Zobrist::table2[sq][2];
			}
			handNum = 1;
		}
	}
#ifndef ENEMY_ISO_TT
	if (turn == BLACK)	keyHist[0] ^= 1;
#endif

	CalcAllChecker();
	CalcAllPin();
	CalcAllPos();
	return CheckLegal();
}

bool Minishogi::CheckLegal() const {
	int pieceCount = 0;
	// TODO :　occupied bitboard
	/*for (Piece p = W_PAWN; p < PIECE_NB; ++p) {
		Bitboard bb = bitboard[p];
		BitScan(bb);
	}*/

	for (Square sq = SQUARE_ZERO; sq < BOARD_NB; ++sq) {
		// 檢查盤面上的棋子在範圍內
		if (board[sq] < 0 || PIECE_NB <= board[sq] || (board[sq] != 0 && PIECE_SCORE[board[sq]] == 0)) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error :　Load Board Failed. board[" << sq << "] is " << board[sq] << sync_endl;
			return false;
		}
		else if (board[sq] > 0) {
			pieceCount++;
		}
	}

	for (Square sq = BOARD_NB; sq < SQUARE_NB; ++sq) {
		// 檢查手牌上的數量在範圍內
		if (board[sq] < 0 || 3 <= board[sq]) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error :　Load Board Failed. board[" << sq << "] is " << board[sq] << sync_endl;
			return false;
		}
		else {
			pieceCount += board[sq];
		}
	}

	// 檢查盤面+手牌的數量
	if (pieceCount != BONA_PIECE_INDEX_NB) {
		cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
		sync_cout << "Error :　Load Board Failed. Piece Count is " << pieceCount << sync_endl;
		return false;
	}

#ifndef KPPT_DISABLE
	for (BonaPieceIndex bpi = BPI_PAWN; bpi < BONA_PIECE_INDEX_NB; ++bpi) {
		// 檢查pieceList[WHITE]範圍 檢查與bonaIndexList是否相對應
		if (pieceList[WHITE][bpi] < BONA_PIECE_ZERO || BONA_PIECE_NB <= pieceList[WHITE][bpi] || bonaIndexList[pieceList[WHITE][bpi]] != bpi) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error : Load Board Failed. pieceList[WHITE][" << bpi << "] is " << pieceList[WHITE][bpi]
				<< " bonaIndexList[" << pieceList[WHITE][bpi] << "] is " << bonaIndexList[pieceList[WHITE][bpi]] << sync_endl;
			return false;
		}
		// 檢查pieceList[BLACK]範圍
		if (pieceList[BLACK][bpi] < BONA_PIECE_ZERO || BONA_PIECE_NB <= pieceList[BLACK][bpi]) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error : Load Board Failed. pieceList[BLACK][" << bpi << "] is " << pieceList[BLACK][bpi] << sync_endl;
			return false;
		}
		// 檢查pieceList[WHITE]是否真的在盤面上的位置
		if (pieceList[WHITE][bpi] < F_HAND && board[pieceList[WHITE][bpi] % BOARD_NB] != pieceList[WHITE][bpi] / BOARD_NB) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error : Load Board Failed. pieceList[WHITE][" << bpi << "] is " << pieceList[WHITE][bpi]
				<< " board[" << pieceList[WHITE][bpi] % BOARD_NB << "] is " << board[pieceList[WHITE][bpi] % BOARD_NB] << sync_endl;
			return false;
		}
		// 檢查pieceList[WHITE]是否真的在手牌上的位置
		if (pieceList[WHITE][bpi] >= F_HAND && board[(pieceList[WHITE][bpi] - F_HAND) / 2 + BOARD_NB] < pieceList[WHITE][bpi] % 2) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error : Load Board Failed. pieceList[WHITE][" << bpi << "] is " << pieceList[WHITE][bpi]
				<< " board[" << (pieceList[WHITE][bpi] - F_HAND) / 2 + BOARD_NB << "] is " << board[(pieceList[WHITE][bpi] - F_HAND) / 2 + BOARD_NB] << sync_endl;
			return false;
		}
	}
#endif
	return true;
}

string Minishogi::Sfen() const {
	stringstream ss;
	int emptyCnt;
	bool hasHand = false;
	for (int rank = 0; rank < 5; rank++) {
		for (int file = 0; file < 5; file++) {
			for (emptyCnt = 0; file < 5 && board[rank * 5 + file] == NO_PIECE; file++)
				emptyCnt++;
			if (emptyCnt)
				ss << emptyCnt;
			if (file < 5) {
				if (is_promote((Piece)board[rank * 5 + file]))
					ss << "+";
				ss << PIECE_2_CHAR[board[rank * 5 + file]];
			}
		}
		if (rank < 4)
			ss << "/";
	}
	ss << (turn == WHITE ? " b " : " w ");
	for (Square sq = BOARD_NB; sq < SQUARE_NB; ++sq) {
		if (board[sq] > 1) {
			ss << board[sq];
		}
		if (board[sq] > 0) {
			ss << HAND_2_CHAR[sq - BOARD_NB];
			hasHand = true;
		}
	}
	ss << (hasHand ? " " : "- ") << ply + 1;
	return ss.str();
}

void Minishogi::DoMove(Move m) {
	ply++;
	assert(ply < MAX_PLY);

	Square from = from_sq(m);
	Square to = to_sq(m);
	bool isPro = is_promote(m);
	Piece pc = GetChessOn(from);
	Piece captured = captureHist[ply - 1] = GetChessOn(to);
	BonaPieceDiff* bonaPieceDiff = bonaPieceDiffHist[ply - 1];
	Bitboard dstboard = 1 << to;

	evalHist[ply] = evalHist[ply - 1];
	keyHist[ply] = keyHist[ply - 1];
#ifdef ENEMY_ISO_TT
	key2Hist[ply] = key2Hist[ply - 1];
#endif

	assert(from < SQUARE_NB);
	assert(to < BOARD_NB);
	assert(pc < PIECE_NB);
	assert(captured < PIECE_NB);
	assert(!pro || Promotable[pc]);

	if (!is_drop(from)) { // 移動或攻擊
		if (captured) {
			Square toHand = EatToHand[captured];
			occupied[~turn] ^= dstboard;      // 更新對方場上狀況
			bitboard[captured] ^= dstboard;   // 更新對方手牌
			board[toHand]++;                  // 轉為該方手牌
#ifndef KPPT_DISABLE
			DoBonaPiece(bonaPieceDiff[1], to, captured, toHand, board[toHand]);
#endif

			evalHist[ply].meterial += HAND_SCORE[toHand] - PIECE_SCORE[captured];
			keyHist[ply] ^= Zobrist::table[to][captured]
						 ^ Zobrist::table[toHand][board[toHand]];
#ifdef ENEMY_ISO_TT
			key2Hist[ply] ^= Zobrist::table2[to][captured]
					      ^ Zobrist::table2[toHand][board[toHand]];
#endif
		}

		occupied[turn] ^= (1 << from) | dstboard; // 更新該方場上狀況
		bitboard[pc] ^= 1 << from;                // 移除該方手牌原有位置

		keyHist[ply] ^= Zobrist::table[from][pc];
#ifdef ENEMY_ISO_TT
		key2Hist[ply] ^= Zobrist::table2[from][pc];
#endif
		if (isPro) { // 升變
			evalHist[ply].meterial += PIECE_SCORE[promote(pc)] - PIECE_SCORE[pc];
			pc = promote(pc);
		}
		bitboard[pc] ^= dstboard; // 更新該方手牌至目的位置
#ifndef KPPT_DISABLE
		DoBonaPiece(bonaPieceDiff[0], from, board[from], to, pc);
#endif
		board[from] = NO_PIECE;   // 原本清空
	}
	else { // 打入
		evalHist[ply].meterial += PIECE_SCORE[pc] - HAND_SCORE[from];
		keyHist[ply] ^= Zobrist::table[from][board[from]];
#ifdef ENEMY_ISO_TT
		key2Hist[ply] ^= Zobrist::table2[from][board[from]];
#endif

		occupied[turn] ^= dstboard;    // 打入場上的位置
		bitboard[pc] ^= dstboard;      // 打入該手牌的位置
#ifndef KPPT_DISABLE
		DoBonaPiece(bonaPieceDiff[0], from, board[from], to, pc);
#endif
		board[from]--;                 // 減少該手牌
	}
	board[to] = pc;  // 放置到目的
	assert(board[0] < PIECE_NB && board[0] >= 0);
	assert(pc < PIECE_NB);
	assert(captured < PIECE_NB);

	turn = ~turn;
	keyHist[ply] ^= Zobrist::table[to][pc];
#ifdef ENEMY_ISO_TT
	key2Hist[ply] ^= Zobrist::table2[to][pc];
#else
	keyHist[ply] ^= 1;
#endif
	moveHist[ply - 1] = m;

	evalHist[ply].pin = VALUE_NONE;
	CalcAllChecker();
}

void Minishogi::UndoMove() {
	assert(ply > 0);

	Move m = moveHist[ply - 1];
	Square from = from_sq(m);
	Square to = to_sq(m);
	Piece pc = GetChessOn(to);
	Piece captured = captureHist[ply - 1];
	BonaPieceDiff* bonaPieceDiff = bonaPieceDiffHist[ply - 1];
	Bitboard dstboard = 1 << to;

	turn = ~turn;
	ply--;

	assert(from < SQUARE_NB);
	assert(to < BOARD_NB);
	assert(pc < PIECE_NB);
	assert(captured < PIECE_NB);

	if (!is_drop(from)) { // 之前是移動或攻擊
		if (captured) {
			occupied[~turn] ^= dstboard;      // 還原對方場上狀況
			bitboard[captured] ^= dstboard;   // 還原對方手牌
			board[EatToHand[captured]]--;     // 從該方手牌移除
#ifndef KPPT_DISABLE
			UndoBonaPiece(bonaPieceDiff[1]);
#endif
		}
		occupied[turn] ^= (1 << from) | dstboard; // 還原該方場上狀況
		bitboard[pc] ^= dstboard;                 // 移除該方手牌的目的位置

		if (is_promote(m))
			pc = toggle_promote(pc);
		bitboard[pc] ^= 1 << from; // 還原該方手牌原有位置
		board[from] = pc;          // 還原棋子
	}
	else { // 之前是打入
		occupied[turn] ^= dstboard; // 取消打入場上的位置
		bitboard[pc] ^= dstboard;   // 取消打入該手牌的位置
		board[from]++;              // 手牌數量加一
	}
	board[to] = captured;           // 還原目的棋子
#ifndef KPPT_DISABLE
	UndoBonaPiece(bonaPieceDiff[0]);
#endif
}

void Minishogi::DoNullMove() {
	ply++;
	turn = ~turn;
	moveHist[ply - 1] = MOVE_NULL;
	captureHist[ply - 1] = NO_PIECE;
	evalHist[ply] = evalHist[ply - 1];
	keyHist[ply] = keyHist[ply - 1];
#ifdef ENEMY_ISO_TT
	key2Hist[ply] = key2Hist[ply - 1];
#else
	keyHist[ply] ^= 1;
#endif
	checker_bb[ply] = checker_bb[ply - 1];
}

void Minishogi::UndoNullMove() {
	ply--;
	turn = ~turn;
}

/// 移動後 會不會造成對方處於被將狀態
void Minishogi::CalcAllChecker() {
	checker_bb[ply] = 0;

	// get the position of my king
	const Bitboard kingboard = bitboard[KING | (turn << 4)];
	const int kingpos = BitScan(kingboard);

	// get the possible position which might attack king
	const Bitboard Occupied = occupied[0] | occupied[1];
	Bitboard upper, lower, attackboard, attack_path;

	// row "-"
	upper = (Occupied & RowUpper[kingpos]) | HighestPosMask;
	lower = (Occupied & RowLower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard rank = (upper - lower) & RowMask(kingpos);
	attackboard = rank & occupied[~turn];

	// search the rook direction possible position
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & rank) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}

	// column "|"
	upper = (Occupied & ColUpper[kingpos]) | HighestPosMask;
	lower = (Occupied & ColLower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard file = (upper - lower) & ColMask(kingpos);
	attackboard = file & occupied[~turn];

	// search the rook direction possible position
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & file) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}

	// slope1 "/"
	upper = (Occupied & Slope1Upper[kingpos]) | HighestPosMask;
	lower = (Occupied & Slope1Lower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard slope1 = (upper - lower) & (Slope1Upper[kingpos] | Slope1Lower[kingpos]);
	attackboard = slope1 & occupied[~turn];

	// search the rook direction possible position 
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & slope1) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}

	// slope2 "\"
	upper = (Occupied & Slope2Upper[kingpos]) | HighestPosMask;
	lower = (Occupied & Slope2Lower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard slope2 = (upper - lower) & (Slope2Upper[kingpos] | Slope2Lower[kingpos]);
	attackboard = slope2 & occupied[~turn];

	// search the rook direction possible position
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		attack_path = Movable(attsrc);
		if (attack_path & kingboard)
			checker_bb[ply] |= (attack_path & slope2) | (1 << attsrc);
		attackboard ^= 1 << attsrc;
	}
}

void Minishogi::CalcAllPin() {
	Bitboard pinner, snipper, totalOccupied = occupied[WHITE] | occupied[BLACK];
	Square sq_wk = BitScan(bitboard[W_KING]);
	Square sq_bk = BitScan(bitboard[B_KING]);
	Value pin = VALUE_ZERO;

	snipper = (bitboard[B_ROOK] | bitboard[B_PRO_ROOK] & RookMask[sq_wk]) |
		(bitboard[B_BISHOP] | bitboard[B_PRO_BISHOP] & BishopMask[sq_wk]);

	while (snipper) {
		int attsrc = BitScan(snipper);
		snipper ^= 1 << attsrc;
		pinner = BetweenBB[sq_wk][attsrc] & totalOccupied;
		if (pinner && !more_than_one(pinner) && pinner & occupied[WHITE]) {
			pin += PIN_SCORE[board[BitScan(pinner)]];
		}
	}

	snipper = (bitboard[W_ROOK] | bitboard[W_PRO_ROOK] & RookMask[sq_bk]) |
		(bitboard[W_BISHOP] | bitboard[W_PRO_BISHOP] & BishopMask[sq_bk]);

	while (snipper) {
		int attsrc = BitScan(snipper);
		snipper ^= 1 << attsrc;
		pinner = BetweenBB[sq_bk][attsrc] & totalOccupied;
		if (pinner && !more_than_one(pinner) && pinner & occupied[BLACK]) {
			pin += PIN_SCORE[board[BitScan(pinner)]];
		}
	}
	evalHist[ply].pin = pin;
}

void Minishogi::CalcAllPos() {
#ifdef KPPT_DISABLE
	return;
#endif
	const auto kk = GlobalEvaluater.kk;
	const auto kkp = GlobalEvaluater.kkp;
	const auto kpp = GlobalEvaluater.kpp;

	Square sq_wk = BitScan(bitboard[W_KING]);
	Square sq_bk = BitScan(bitboard[B_KING]), sq_bki = rotate_board_sq(sq_bk);
	EvalSum sum = evalHist[ply];

	sum.pos[0][0] = 0;
	sum.pos[0][1] = 0;
	sum.pos[1][0] = 0;
	sum.pos[1][1] = 0;
	sum.pos[2] = kk[sq_wk][sq_bk];
	for (int i = 0; i < BPI_KING; i++) {
		int w0 = pieceList[WHITE][i];
		int b0 = pieceList[BLACK][i];
		sum.pos[0] += kkp[sq_wk][sq_bk][w0];
		for (int j = 0; j < i; j++) {
			sum.pos[0] += kpp[sq_wk ][w0][pieceList[WHITE][j]];
			sum.pos[1] += kpp[sq_bki][b0][pieceList[BLACK][j]];
		}
	}
	evalHist[ply] = sum;
}

void Minishogi::CalcDiffPos() {
#ifdef KPPT_DISABLE
	return;
#endif
	if (ply == 0 || evalHist[ply - 1].pin == VALUE_NONE) {
		CalcAllPos();
		return;
	}

	const auto kk = GlobalEvaluater.kk;
	const auto kkp = GlobalEvaluater.kkp;
	const auto kpp = GlobalEvaluater.kkp;

	Piece pc = (Piece)board[to_sq(moveHist[ply - 1])];
	Piece capture = captureHist[ply - 1];
	BonaPieceDiff* bpd = bonaPieceDiffHist[ply - 1];
	Square sq_wk = BitScan(bitboard[W_KING]);
	Square sq_bk = BitScan(bitboard[B_KING]), sq_bki = SQ_E1 - sq_bk;
	EvalSum sum = evalHist[ply - 1];
	int i, j;

	if (pc == W_KING) {
		sum.pos[0][0] = 0;
		sum.pos[0][1] = 0;
		sum.pos[2] = kk[sq_wk][sq_bk];
		for (i = 0; i < BPI_KING; i++) {
			int w0 = pieceList[WHITE][i];
			sum.pos[2] += kkp[sq_wk][sq_bk][w0];
			for (j = 0; j < i; j++) {
				sum.pos[0] += kpp[sq_wk][w0][pieceList[WHITE][j]];
			}
		}
		if (capture) {
			BonaPieceIndex capturerIndex = bonaIndexList[bpd[1].nowBonaW];
			for (i = 0; i < capturerIndex; ++i) {
				sum.pos[1] -= kpp[sq_bki][bpd[1].preBonaB][pieceList[BLACK][i]];
				sum.pos[1] += kpp[sq_bki][bpd[1].nowBonaB][pieceList[BLACK][i]];
			}
			for (++i; i < BPI_KING; ++i) {
				sum.pos[1] -= kpp[sq_bki][bpd[1].preBonaB][pieceList[BLACK][i]];
				sum.pos[1] += kpp[sq_bki][bpd[1].nowBonaB][pieceList[BLACK][i]];
			}
		}
	}
	else if (pc == B_KING) {
		sum.pos[1][0] = 0;
		sum.pos[1][1] = 0;
		sum.pos[2] = kk[sq_wk][sq_bk];
		for (int i = 0; i < BPI_KING; i++) {
			int w0 = pieceList[WHITE][i];
			int w1 = pieceList[BLACK][i];
			sum.pos[2] += kkp[sq_wk][sq_bk][w0];
			for (int j = 0; j < i; j++) {
				sum.pos[1] += kpp[sq_bki][w1][pieceList[WHITE][j]];
			}
		}
		if (capture) {
			BonaPieceIndex capturerIndex = bonaIndexList[bpd[1].nowBonaW];
			for (i = 0; i < capturerIndex; ++i) {
				sum.pos[0] -= kpp[sq_wk][bpd[1].preBonaW][pieceList[WHITE][i]];
				sum.pos[0] += kpp[sq_wk][bpd[1].nowBonaW][pieceList[WHITE][i]];
			}
			for (++i; i < BPI_KING; ++i) {
				sum.pos[0] -= kpp[sq_wk][bpd[1].preBonaW][pieceList[WHITE][i]];
				sum.pos[0] += kpp[sq_wk][bpd[1].nowBonaW][pieceList[WHITE][i]];
			}
		}
	}
	else {
#define ADD_BWKPP(BPD) { \
		assert(pieceList[WHITE][i] < BONA_PIECE_NB); \
		assert(pieceList[BLACK][i] < BONA_PIECE_NB); \
        sum.pos[0] -= kpp[sq_wk ][BPD.preBonaW][pieceList[WHITE][i]]; \
        sum.pos[1] -= kpp[sq_bki][BPD.nowBonaW][pieceList[BLACK][i]]; \
        sum.pos[0] += kpp[sq_wk ][BPD.preBonaB][pieceList[WHITE][i]]; \
        sum.pos[1] += kpp[sq_bki][BPD.nowBonaB][pieceList[BLACK][i]]; \
}
		if (!capture) {
			BonaPieceIndex moverIndex = bonaIndexList[bpd[0].nowBonaW];
			sum.pos[2] -= kkp[sq_wk][sq_bk][bpd[0].preBonaW];
			sum.pos[2] += kkp[sq_wk][sq_bk][bpd[0].nowBonaW];

			for (i = 0; i < moverIndex; ++i)
				ADD_BWKPP(bpd[0]);
			for (++i; i < BPI_KING; ++i)
				ADD_BWKPP(bpd[0]);
		}
		else {
			BonaPieceIndex moverIndex = bonaIndexList[bpd[0].nowBonaW];
			BonaPieceIndex capturerIndex = bonaIndexList[bpd[1].nowBonaW];
			sum.pos[2] -= kkp[sq_wk][sq_bk][bpd[0].preBonaW];
			sum.pos[2] += kkp[sq_wk][sq_bk][bpd[0].nowBonaW];
			sum.pos[2] -= kkp[sq_wk][sq_bk][bpd[1].preBonaB];
			sum.pos[2] += kkp[sq_wk][sq_bk][bpd[1].nowBonaB];

			for (i = 0; i < moverIndex; ++i) {
				ADD_BWKPP(bpd[0]);
				ADD_BWKPP(bpd[1]);
			}
			for (++i; i < capturerIndex; ++i) {
				ADD_BWKPP(bpd[0]);
				ADD_BWKPP(bpd[1]);
			}
			for (++i; i < BPI_KING; ++i) {
				ADD_BWKPP(bpd[0]);
				ADD_BWKPP(bpd[1]);
			}

			sum.pos[0] -= kpp[sq_wk ][bpd[0].preBonaW][bpd[1].preBonaW];
			sum.pos[1] -= kpp[sq_bki][bpd[0].preBonaB][bpd[1].preBonaB];
			sum.pos[0] += kpp[sq_wk ][bpd[0].nowBonaW][bpd[1].nowBonaW];
			sum.pos[1] += kpp[sq_bki][bpd[0].nowBonaB][bpd[1].nowBonaB];
		}
	}

	// Debug : CalcDiffPos() == CalcAllPos()
	/*Value v = GetEvaluate();
	CalcAllPos();
	assert(v == GetEvaluate());*/
}

bool Minishogi::PseudoLegal(Move m) const {
	Square from = from_sq(m), to = to_sq(m);
	// 不超出移動範圍
	if (from >= SQUARE_NB || to >= BOARD_NB)
		return false;

	Piece pc = GetChessOn(from), capture = GetChessOn(to);
	// 只能出己方的手排
	if (pc == NO_PIECE || color_of(pc) != turn)
		return false;

	// 如果是吃子，不能是打入，且只能吃對方
	if (capture != NO_PIECE && (from >= BOARD_NB || color_of(capture) == turn))
		return false;

	// 理論上吃不到王，也不能讓自己被將
	if (type_of(capture) == KING || IsInCheckedAfter(m))
		return false;

	// 如果是移動(吃子)，驗證這顆棋子真的走的到
	if (!is_drop(from) && !(Movable(from) & (1 << to)))
		return false;

	// 只允許某些棋子升變
	if (is_promote(m) && !(Promotable[pc] && ((1 << from | 1 << to) & EnemyCampMask[turn])))
		return false;

	return true;
}

ExtMove* Minishogi::AttackGenerator(ExtMove *moveList) const {
	// AttackGene moveList order by attacker
	static const int AttackerOrder[] = { PAWN, SILVER, GOLD, PRO_PAWN, PRO_SILVER, BISHOP, ROOK, PRO_BISHOP, PRO_ROOK, KING };
	Bitboard srcboard, dstboard, opboard = occupied[~turn], attackboard = checker_bb[ply];
	Square src, dst;
	int turnBit = turn << 4;

	if (!attackboard)
		attackboard = BitboardMask;

	for (int i = 0; i < 10; i++) {
		srcboard = bitboard[AttackerOrder[i] | turnBit];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(src) & opboard;
			if (attackboard && AttackerOrder[i] != KING)
				dstboard &= attackboard;
			while (dstboard) {
				dst = turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;
				*moveList++ = make_move(src, dst,
					Promotable[AttackerOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[turn]));

				if (AttackerOrder[i] == SILVER && is_promote(*(moveList - 1))) {
					*moveList++ = make_move(src, dst, 0);
				}
			}
		}
	}

	return moveList;
}

ExtMove* Minishogi::MoveGenerator(ExtMove *moveList) const {
	// MoveGene moveList order by mover 
	static const int MoverOrder[] = { PRO_ROOK, PRO_BISHOP, ROOK, BISHOP, PRO_SILVER, PRO_PAWN, GOLD, SILVER, PAWN, KING };
	Bitboard srcboard, dstboard, blankboard = BlankOccupied(occupied), attackboard = checker_bb[ply];
	Square src, dst;
	int turnBit = turn << 4;

	for (int i = 0; i < 10; i++) {
		srcboard = bitboard[MoverOrder[i] | turnBit];
		while (srcboard) {
			src = BitScan(srcboard);
			srcboard ^= 1 << src;
			dstboard = Movable(src) & blankboard;
			if (attackboard && MoverOrder[i] != KING) 
				dstboard &= attackboard;
			while (dstboard) {
				dst = turn ? BitScanRev(dstboard) : BitScan(dstboard);
				dstboard ^= 1 << dst;
				*moveList++ = make_move(src, dst,
					Promotable[MoverOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[turn]));
				
				if (MoverOrder[i] == SILVER && is_promote(*(moveList - 1))) {
					*moveList++ = make_move(src, dst, 0);
				}
			}
		}
	}
	return moveList;
}

ExtMove* Minishogi::HandGenerator(ExtMove *moveList) {
	Square src = (turn ? SQ_G1 : SQ_F1), dst;

	// 如果沒有任何手排 直接回傳
	if (!board[src] && !board[src - 1] && !board[src - 2] && !board[src - 3] && !board[src - 4])
		return moveList;

	Bitboard srcboard = BlankOccupied(occupied), dstboard, nifu = 0;
	Bitboard checker = checker_bb[ply] & occupied[~turn];
	if (checker) {                    
		if (checker & (checker - 1)) // if there are more than one checkers
			return moveList;         // no need to generate

		// if there is only one checker, only the path from checker to my king can be blocked
		srcboard &= checker_bb[ply]; 
	}

	++src;
	for (int i = 0; i < 4; i++) { // 步以外的手排
		if (board[--src]) {
			dstboard = srcboard;
			while (dstboard) {
				dst = BitScan(dstboard);
				dstboard ^= 1 << dst;
				*moveList++ = make_move(src, dst, false);
			}
		}
	}

	if (board[--src]) { // 步
		dstboard = bitboard[(turn << 4) | PAWN]; // 我方的步
		if (dstboard)
			nifu |= ColMask(BitScan(dstboard)); // 二步

		/************ 打步詰 ************/
		Bitboard kingboard = bitboard[KING | (turn ? 0 : BLACKCHESS)];
		Bitboard pawnboard = turn ? kingboard >> 5 : kingboard << 5;

		if (checker == 0 && (pawnboard & srcboard)) {
			Bitboard uchifuzume = pawnboard; // 假設有打步詰
			Square kingpos = BitScan(kingboard), pawnpos = BitScan(pawnboard);

			/************ DoMove ************/
			occupied[turn] ^= pawnboard;
			bitboard[PAWN | (turn << 4)] ^= pawnboard;
			turn = ~turn;
			/************ DoMove ************/

			// 對方王可 吃/移動 的位置
			dstboard = Movement[KING][kingpos];
			dstboard &= dstboard ^ occupied[turn];
			while (dstboard) {
				dst = BitScan(dstboard);
				// 如果王移動後脫離被王手
				if (!IsInCheckedAfter(kingpos, dst)) {
					uchifuzume = 0; // 代表沒有打步詰
					break;
				}
				dstboard ^= 1 << dst;
			}

			// 對方可能攻擊到步的棋子 (不包括王)
			if (uchifuzume) {
				Bitboard attackboard = ((RookMovable(pawnpos) | BishopMovable(pawnpos)) &occupied[turn]) ^ kingboard;
				while (attackboard) {
					Square attsrc = BitScan(attackboard);
					// 如果真的吃得到步 且 吃了之後不會被王手
					if ((Movable(attsrc) & pawnboard) && !IsInCheckedAfter(attsrc, pawnpos)) {
						uchifuzume = 0; // 代表沒有打步詰
						break;
					}
					attackboard ^= 1 << attsrc;
				}
			}

			/************ UnDoMove ************/
			turn = ~turn;
			occupied[turn] ^= pawnboard;
			bitboard[PAWN | (turn << 4)] ^= pawnboard;
			/************ UnDoMove ************/

			nifu |= uchifuzume;
		}
		/************ 打步詰 ************/

		dstboard = srcboard & ~(EnemyCampMask[turn] | nifu);
		while (dstboard) {
			dst = BitScan(dstboard);
			dstboard ^= 1 << dst;
			*moveList++ = make_move(src, dst, false);
		}
	}
	return moveList;
}

Move* Minishogi::GetLegalMoves(Move* legalMoves) {
	ExtMove moveList[TOTAL_GENE_MAX_ACTIONS], *end;
	end = AttackGenerator(moveList);
	end = MoveGenerator(end);
	end = HandGenerator(end);
	for (ExtMove* i = moveList; i < end; i++) {
		if (!IsInCheckedAfter(*i)) {
			*legalMoves++ = i->move;
		}
	}
	return legalMoves;
}

inline Bitboard Minishogi::Movable(int srcIndex, Bitboard Occupied) const {
	assert(srcIndex < BOARD_NB);
	const int srcChess = board[srcIndex];

	if ((srcChess & 7) == BISHOP) {
		if (srcChess & PROMOTE)
			return BishopMovable(srcIndex, Occupied) | Movement[KING][srcIndex];
		return BishopMovable(srcIndex, Occupied);
	}
	else if ((srcChess & 7) == ROOK) {
		if (srcChess & PROMOTE)
			return RookMovable(srcIndex, Occupied) | Movement[KING][srcIndex];
		return RookMovable(srcIndex, Occupied);
	}
	else if (srcChess & PROMOTE) {
		return Movement[GOLD | (srcChess & BLACKCHESS)][srcIndex];
	}
	return Movement[srcChess][srcIndex];
}

inline Bitboard Minishogi::RookMovable(int srcIndex, Bitboard Occupied) const {
	// upper (find LSB) ; lower (find MSB)
	if (!Occupied) Occupied = occupied[0] | occupied[1];
	Bitboard rank, file, upper, lower;

	// row "-"
	upper = (Occupied & RowUpper[srcIndex]) | HighestPosMask;
	lower = (Occupied & RowLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	rank = (upper - lower) & RowMask(srcIndex);

	// column "|"
	upper = (Occupied & ColUpper[srcIndex]) | HighestPosMask;
	lower = (Occupied & ColLower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	file = (upper - lower) & ColMask(srcIndex);

	return rank | file;
}

inline Bitboard Minishogi::BishopMovable(int srcIndex, Bitboard Occupied) const {
	// upper (find LSB) ; lower (find MSB)
	if (!Occupied)
		Occupied = occupied[0] | occupied[1];
	Bitboard slope1, slope2, upper, lower;

	// slope1 "/"
	upper = (Occupied & Slope1Upper[srcIndex]) | HighestPosMask;
	lower = (Occupied & Slope1Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope1 = (upper - lower) & (Slope1Upper[srcIndex] | Slope1Lower[srcIndex]);

	// slope2 "\"
	upper = (Occupied & Slope2Upper[srcIndex]) | HighestPosMask;
	lower = (Occupied & Slope2Lower[srcIndex]) | LowestPosMask;

	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);

	slope2 = (upper - lower) & (Slope2Upper[srcIndex] | Slope2Lower[srcIndex]);

	return slope1 | slope2;
}

inline Bitboard Minishogi::attackers_to(int dstIndex, Bitboard occupied) const {
	return RookMovable(dstIndex, occupied) | BishopMovable(dstIndex, occupied);
}

void Minishogi::PrintChessBoard() const {
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	cout << "---------- ply  " << ply << " ----------\n";
	SetConsoleTextAttribute(hConsole, 7);
	cout << "  ｜ 5｜ 4｜ 3｜ 2｜ 1｜\n";
	for (int rank = 0; rank < 5; rank++) {
		cout << "—┼—┼—┼—┼—┼—┼—" << (rank == 0 ? "      後手持駒\n" : "\n");

		cout << " " << char('a' + rank) << "｜";
		for (int file = 0; file < 5; file++) {
			Piece p = Piece(board[rank * 5 + file]);
			if (p != NO_PIECE)
				SetConsoleTextAttribute(hConsole, 10 + color_of(p) * 2 + is_promote(p));
			cout << PIECE_WORD.substr(p * 2, 2);
			SetConsoleTextAttribute(hConsole, 7);
			cout << "｜";
		}
		cout << char('a' + rank) << "   ";

		if (rank == 0)
			for (int i = 0; i < 5; i++)
				if (board[SQ_F5 + i])
					cout << board[SQ_F5 + i] << PIECE_WORD.substr((i + 1) * 2, 2);
				else
					cout << "   ";
		if (rank == 4)
			for (int i = 0; i < 5; i++)
				if (board[SQ_G5 + i])
					cout << board[SQ_G5 + i] << PIECE_WORD.substr((i + 1) * 2, 2);
				else
					cout << "   ";
		cout << "\n";
	}
	cout << "—┼—┼—┼—┼—┼—┼—      先手持駒\n";
	cout << "  ｜ 5｜ 4｜ 3｜ 2｜ 1｜\n";
	cout << "SFEN : " << Sfen() << "\n";
	cout << "----------" << COLOR_WORD[turn] << " Turn----------\n";
}

/// 第一行 +代表白方先 -代表黑方先
/// 第二~六行 5*5的棋盤 旗子符號請見Piece.h的SAVE_CHESS_WORD
/// 第七~八行 ▼0步0銀0金0角0飛 △0步0銀0金0角0飛
bool Minishogi::SaveBoard(string filename) const {
	fstream file("board//" + filename, ios::app);
	if (file) {
		file << (GetTurn() ? "-" : "+") << Sfen() << "\n" << (*this) << "\n";
		file.close();
		sync_cout << "Succeed to Save Board." << sync_endl;
		return true;
	} 
	sync_cout << "Error : Fail to Save Board." << sync_endl;
	return false;
}

bool Minishogi::LoadBoard(string filename, streamoff &offset) {
	fstream file(filename, ios::in);
	if (file) {
		string buf;
		file.seekg(offset, ios::beg);
		do {
			if (!getline(file, buf)) {
				file.close();
				return false;
			}
			offset = file.tellg();
		} while (!Initialize(buf));
		file.close();
		return true;
	}
	sync_cout << "Error : Fail to Load Board. Cannot find file." << sync_endl;
	return false;
}

void Minishogi::PrintKifu(ostream &os) const {
	os << "Kifu hash : " << setw(10) << hex << GetKifuHash() << "\n";
	os << "Initboard : " << setw(18) << hex << GetKey(0) << dec << "\n";
	for (int i = 0; i < ply; i++) {
		os << setw(2) << i << " : " << COLOR_WORD[i % 2];
		os << moveHist[i] << setw(18) << hex << GetKey(i + 1) << dec << "\n";
	}
}

bool Minishogi::IsGameOver() {
	Move moveList[TOTAL_GENE_MAX_ACTIONS];
	return moveList == GetLegalMoves(moveList);
}

bool Minishogi::IsLegelAction(Move m) {
	Move moveList[TOTAL_GENE_MAX_ACTIONS], *end = GetLegalMoves(moveList);
	return end != find(moveList, end, m);
}

/// 移動完有沒有被將軍
bool Minishogi::IsInCheckedAfter(Square srcIndex, Square dstIndex) const {
	const Bitboard dstboard = 1 << dstIndex;
    Bitboard op_occupied = occupied[~turn];
    if (board[dstIndex]) // eat
        op_occupied ^= dstboard;

    const Bitboard tmp_occupied = (occupied[turn] | op_occupied) ^ ((!is_drop(srcIndex) ? (1 << srcIndex) : 0) | dstboard);

    /* get the position of the checked king */
    Bitboard kingboard = dstboard;
    int kingpos = dstIndex;
    if (type_of(board[srcIndex]) != KING) {
        kingboard = bitboard[KING | (turn << 4)];
        kingpos = BitScan(kingboard);
    }

	/* get the possible position which might attack king */
    Bitboard attackboard = attackers_to(kingpos, tmp_occupied) & op_occupied;

	/* search the possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		if (Movable(attsrc, tmp_occupied) & kingboard) return true;
		attackboard ^= 1 << attsrc;
	}

	return false;
}

/// 移動完有沒有將軍對方
bool Minishogi::IsCheckAfter(const Move m) {
	assert(m != MOVE_NULL);
	const Square srcIndex = from_sq(m), dstIndex = to_sq(m);
	const int dstChess = board[dstIndex];
	board[dstIndex] = GetChessOn(srcIndex);
	const bool isCheckable = Movable(dstIndex) & bitboard[KING | ((~turn) << 4)];
	board[dstIndex] = dstChess;

	if (is_drop(srcIndex)) return isCheckable;
	else if (isCheckable) return true;

	const Bitboard my_occupied = occupied[turn] ^ (1 << srcIndex);
	const Bitboard tmp_occupied = occupied[~turn] | my_occupied;

	/* get the position of the checking king */
	const int kingpos = BitScan(bitboard[KING | ((~turn) << 4)]);

	/* get my possible position which might attack the opponent king */
	Bitboard attackboard = RookMovable(kingpos, tmp_occupied) & my_occupied;

	/* search the possible position */
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		if ((board[attsrc] & 7) == ROOK) return true;
		attackboard ^= 1 << attsrc;
	}

	attackboard = BishopMovable(kingpos, tmp_occupied) & my_occupied;
	while (attackboard) {
		int attsrc = BitScan(attackboard);
		if ((board[attsrc] & 7) == BISHOP) return true;
		attackboard ^= 1 << attsrc;
	}

	return false;
}

/// 如果現在盤面曾經出現過 且距離為偶數(同個人) 判定為千日手 需要先DoMove後才能判斷 在此不考慮被連將
bool Minishogi::IsSennichite() const {
	for (int i = ply - 4; i >= 0; i -= 2) {
		if (keyHist[i] == keyHist[ply]) {
			return true;
		}
	}
	return false;
}

bool Minishogi::SEE(const Move move, Value threshold) const {
	Square srcIndex = from_sq(move), dstIndex = to_sq(move);

	int balance = PIECE_SCORE[type_of(board[dstIndex])] - threshold;
	int myChessValue[8], opChessValue[8];
	int my_count = 0, op_count = 0;
	const Bitboard moveboard = is_drop(srcIndex) ? 0 : (1 << srcIndex);
	const Bitboard tem_board = (occupied[0] | occupied[1]) ^ moveboard;
	const Bitboard psbboard = attackers_to(dstIndex, tem_board);

	/************ Add opChessValue ************/
	Bitboard srcboard = psbboard & occupied[turn ^ 1];
	Bitboard dstboard = 1 << dstIndex;
	while (srcboard) {
		int attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (Movable(attsrc, tem_board) & dstboard)
			opChessValue[op_count++] = PIECE_SCORE[type_of(board[attsrc])];
	}
	/************ End opChessValue ************/
	if (op_count == 0) return true; // 對方都吃不到我下的位置

	/************ Add myChessValue ************/
	srcboard = psbboard & (occupied[turn] ^ moveboard);
	dstboard = 1 << dstIndex;
	while (srcboard) {
		int attsrc = BitScan(srcboard);
		srcboard ^= 1 << attsrc;
		if (Movable(attsrc) & dstboard)
			myChessValue[my_count++] = PIECE_SCORE[type_of(board[attsrc])];
	}
	/************ End myChessValue ************/
	if (my_count == 0) return false; // 在對方能吃到我的前提，我方都無法還擊

	/************ Sorting ************/
	sort(opChessValue, opChessValue + op_count, [](const int &a, const int &b) { return a < b; });
	sort(myChessValue, myChessValue + my_count, [](const int &a, const int &b) { return a < b; });

	balance -= PIECE_SCORE[type_of(GetChessOn(srcIndex))];
	for (int op = 0, my = 0; my < my_count;) {
		balance += opChessValue[op++];
		if (op < op_count)
			balance -= myChessValue[my++];
		else break;
	}

	return balance >= threshold;
}

std::ostream& operator<<(std::ostream& os, const Minishogi& pos) {
	stringstream ss;
	ss << "[" << COLOR_WORD[pos.turn] << "]" << pos.Sfen() << "\n";
	for (Square sq = SQUARE_ZERO; sq < BOARD_NB; ++sq) {
		if (pos.board[sq] == NO_PIECE)
			ss << " ． ";
		else
			ss << NONCOLOR_PIECE_WORD.substr(pos.board[sq] * 4, 4);
		if (sq % 5 == 4)
			ss << "\n";
	}
	ss << COLOR_WORD[BLACK] << " : ";
	for (int i = 0; i < 5; i++)
		ss << pos.board[i + SQ_B_HAND] << PIECE_WORD.substr((i + 1) * 2, 2);
	ss << "\n" << COLOR_WORD[WHITE] << " : ";
	for (int i = 0; i < 5; i++)
		ss << pos.board[i + SQ_W_HAND] << PIECE_WORD.substr((i + 1) * 2, 2);
	os << ss.str();
	return os;
}