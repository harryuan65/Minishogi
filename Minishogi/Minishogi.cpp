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

	copy(m.occupied, m.occupied + COLOR_NB, occupied);
	copy(m.bitboard, m.bitboard + PIECE_NB, bitboard);
	copy(m.board, m.board + SQUARE_NB, board);
	for (BonaPieceIndex i = BPI_PAWN; i < BONA_PIECE_INDEX_NB; ++i)
		SetBonaPiece(i, m.pieceList[WHITE][i], m.pieceList[BLACK][i]);
	copy(m.stateHist, m.stateHist + m.ply + 1, stateHist);

	assert(m.GetKey() == GetKey());
	return CheckLegal();
}

bool Minishogi::InitializeByBoard(string str) {
	istringstream ss(str);
	string token;
	Square sq = SQUARE_ZERO;
	StateInfo *st = &stateHist[0];

	memset(occupied, 0, COLOR_NB * sizeof(Bitboard));
	memset(bitboard, 0, PIECE_NB * sizeof(Bitboard));
	memset(board, NO_PIECE, SQUARE_NB * sizeof(int));
	memset(st, 0, sizeof(StateInfo));
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
				st->eval.material += PIECE_SCORE[pc];
				st->key ^= Zobrist::table[sq][pc];
				st->key2 ^= Zobrist::table2[sq][pc];
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
				st->eval.material += HAND_SCORE[sq] * handNum;
				st->key ^= Zobrist::table[sq][1];
				st->key2 ^= Zobrist::table2[sq][1];
				if (handNum == 2) {
					SetBonaPiece(sq, Piece(2));
					st->key ^= Zobrist::table[sq][2];
					st->key2 ^= Zobrist::table2[sq][2];
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
	st->checker_bb = GetChecker();

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
	StateInfo *st = &stateHist[0];

	memset(occupied, 0, COLOR_NB * sizeof(Bitboard));
	memset(bitboard, 0, PIECE_NB * sizeof(Bitboard));
	memset(board, NO_PIECE, SQUARE_NB * sizeof(int));
	memset(st, 0, sizeof(StateInfo));
	for (BonaPieceIndex i = BPI_PAWN; i < BONA_PIECE_INDEX_NB; ++i)
		pieceList[WHITE][i] = BONA_PIECE_NB;
	ply = 0;

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
			st->eval.material += PIECE_SCORE[pc];
			st->key ^= Zobrist::table[sq][pc];
			st->key2 ^= Zobrist::table2[sq][pc];
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
			st->eval.material += HAND_SCORE[sq] * handNum;
			st->key ^= Zobrist::table[sq][1];
			st->key2 ^= Zobrist::table2[sq][1];
			if (handNum == 2) {
				SetBonaPiece(sq, (Piece)2);
				st->key ^= Zobrist::table[sq][2];
				st->key2 ^= Zobrist::table2[sq][2];
			}
			handNum = 1;
		}
	}
#ifndef ENEMY_ISO_TT
	if (turn == BLACK)	keyHist[0] ^= 1;
#endif
	st->checker_bb = GetChecker();

	CalcAllPin();
	CalcAllPos();
	return CheckLegal();
}

bool Minishogi::CheckLegal() const {
	// TODO : 對stateHist檢查
	int pieceCount = 0;

	for (Square sq = SQUARE_ZERO; sq < BOARD_NB; ++sq) {
		// 檢查盤面上的棋子在範圍內
		if (board[sq] < 0 || PIECE_NB <= board[sq] || (board[sq] != 0 && PIECE_SCORE[board[sq]] == 0)) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error :　board[" << sq << "] is " << board[sq] << sync_endl;
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
			sync_cout << "Error :　board[" << sq << "] is " << board[sq] << sync_endl;
			return false;
		}
		else {
			pieceCount += board[sq];
		}
	}

	// 檢查盤面+手牌的數量
	if (pieceCount != BONA_PIECE_INDEX_NB) {
		cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
		sync_cout << "Error :　Piece Count is " << pieceCount << sync_endl;
		return false;
	}

#ifndef KPPT_DISABLE
	for (BonaPieceIndex bpi = BPI_PAWN; bpi < BONA_PIECE_INDEX_NB; ++bpi) {
		// 檢查pieceList[WHITE]範圍 檢查與bonaIndexList是否相對應
		if (pieceList[WHITE][bpi] < BONA_PIECE_ZERO || BONA_PIECE_NB <= pieceList[WHITE][bpi] || bonaIndexList[pieceList[WHITE][bpi]] != bpi) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error : pieceList[WHITE][" << bpi << "] is " << pieceList[WHITE][bpi]
				<< " bonaIndexList[" << pieceList[WHITE][bpi] << "] is " << bonaIndexList[pieceList[WHITE][bpi]] << sync_endl;
			return false;
		}
		// 檢查pieceList[BLACK]範圍
		if (pieceList[BLACK][bpi] < BONA_PIECE_ZERO || BONA_PIECE_NB <= pieceList[BLACK][bpi]) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error :pieceList[BLACK][" << bpi << "] is " << pieceList[BLACK][bpi] << sync_endl;
			return false;
		}
		// 檢查pieceList[WHITE]是否真的在盤面上的位置
		if (pieceList[WHITE][bpi] < F_HAND && board[pieceList[WHITE][bpi] % BOARD_NB] != pieceList[WHITE][bpi] / BOARD_NB) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error : pieceList[WHITE][" << bpi << "] is " << pieceList[WHITE][bpi]
				<< " board[" << pieceList[WHITE][bpi] % BOARD_NB << "] is " << board[pieceList[WHITE][bpi] % BOARD_NB] << sync_endl;
			return false;
		}
		// 檢查pieceList[WHITE]是否真的在手牌上的位置
		if (pieceList[WHITE][bpi] >= F_HAND && board[(pieceList[WHITE][bpi] - F_HAND) / 2 + BOARD_NB] < pieceList[WHITE][bpi] % 2) {
			cout << IO_LOCK << (*this) << IO_UNLOCK << endl;
			sync_cout << "Error : pieceList[WHITE][" << bpi << "] is " << pieceList[WHITE][bpi]
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
	turn = ~turn;

	StateInfo *st = &stateHist[ply];
	memcpy(st, &stateHist[ply - 1], offsetof(StateInfo, move));

	Square from = from_sq(m);
	Square to = to_sq(m);
	bool isPro = is_promote(m);
	Piece pc = GetChessOn(from);
	Piece captured = st->capture = GetChessOn(to);
	BonaPieceDiff* bonaPieceDiff = st->bonaPieceDiff;
	Bitboard dstBoard = 1 << to;

	/*st->eval = stateHist[ply - 1].eval;
	st->key = stateHist[ply - 1].key;
#ifdef ENEMY_ISO_TT
	st->key2 = stateHist[ply - 1].key2;
#endif
	st->continueCheck[0] = stateHist[ply - 1].continueCheck[0];
	st->continueCheck[1] = stateHist[ply - 1].continueCheck[1];*/

	if (!is_drop(from)) {							/// Move or Catpure
		if (captured) {
			Square toHand = EatToHand[captured];
			occupied[turn] ^= dstBoard;				// remove capture in occupied
			bitboard[captured] ^= dstBoard;			// remove capture in bitboard
			board[toHand]++;						// place  capture in hand
#ifndef KPPT_DISABLE
			DoBonaPiece(bonaPieceDiff[1], to, captured, toHand, board[toHand]);
#endif
			// update capture in evaluate and hashkey
			st->eval.material += HAND_SCORE[toHand] - PIECE_SCORE[captured];
			st->key ^= Zobrist::table[to][captured] ^ Zobrist::table[toHand][board[toHand]];
#ifdef ENEMY_ISO_TT
			st->key2 ^= Zobrist::table2[to][captured] ^ Zobrist::table2[toHand][board[toHand]];
#endif
		}
		// remove srcPiece in hashkey
		st->key ^= Zobrist::table[from][pc];
#ifdef ENEMY_ISO_TT
		st->key2 ^= Zobrist::table2[from][pc];
#endif

		occupied[~turn] ^= (1 << from) | dstBoard;	// remove srcPiece & place dstPiece in occupied
		bitboard[pc] ^= 1 << from;					// remove srcPiece in bitboard
		if (isPro) {
			// update srcPiece in evaluate
			st->eval.material += PIECE_SCORE[promote(pc)] - PIECE_SCORE[pc];
			pc = promote(pc);
		}
		bitboard[pc] ^= dstBoard;					// place  dstPiece in bitboard
#ifndef KPPT_DISABLE
		DoBonaPiece(bonaPieceDiff[0], from, board[from], to, pc);
#endif
		board[from] = NO_PIECE;						// remove srcPiece in board
	}
	else {											/// Drop
		// update srcPiece in evaluate, remove srcPiece in hashkey
		st->eval.material += PIECE_SCORE[pc] - HAND_SCORE[from];
		st->key ^= Zobrist::table[from][board[from]];
#ifdef ENEMY_ISO_TT
		st->key2 ^= Zobrist::table2[from][board[from]];
#endif

		occupied[~turn] ^= dstBoard;				// place  dstPiece in occupied
		bitboard[pc] ^= dstBoard;					// place  dstPiece in bitboard
#ifndef KPPT_DISABLE
		DoBonaPiece(bonaPieceDiff[0], from, board[from], to, pc);
#endif
		board[from]--;								// reduce srcPiece in hand
	}
	board[to] = pc;									// update dstPiece in board

	// put dstPiece in hashkey
	st->key ^= Zobrist::table[to][pc];
#ifdef ENEMY_ISO_TT
	st->key2 ^= Zobrist::table2[to][pc];
#else
	st->key ^= 1;
#endif
	
	st->move = m;
	st->eval.SetNonCalc();
	st->checker_bb = GetChecker();
	if (st->checker_bb)
		st->continueCheck[~turn] += 2;
	else
		st->continueCheck[~turn] = 0;

}

void Minishogi::UndoMove() {
	Move m = stateHist[ply].move;
	Square from = from_sq(m);
	Square to = to_sq(m);
	Piece pc = GetChessOn(to);
	Piece captured = stateHist[ply].capture;
	BonaPieceDiff* bonaPieceDiff = stateHist[ply].bonaPieceDiff;
	Bitboard dstBoard = 1 << to;

	if (!is_drop(from)) {							/// Move or Capture
		if (captured) {
			occupied[turn] ^= dstBoard;				// place  capture in occupied
			bitboard[captured] ^= dstBoard;			// place  capture in bitboard
			board[EatToHand[captured]]--;			// remove capture in hand
#ifndef KPPT_DISABLE
			UndoBonaPiece(bonaPieceDiff[1]);
#endif
		}
		occupied[~turn] ^= (1 << from) | dstBoard;	// place  srcPiece & remove dstPiece in occupied
		bitboard[pc] ^= dstBoard;					// remove srcPiece in bitboard
		if (is_promote(m))
			pc = toggle_promote(pc);
		bitboard[pc] ^= 1 << from;					// place  srcPiece in bitboard
		board[from] = pc;							// place  srcPiece in board
	}
	else {											/// Drop
		occupied[~turn] ^= dstBoard;				// remove dstPiece in occupied
		bitboard[pc] ^= dstBoard;					// remove dstPiece in bitboard
		board[from]++;								// add    srcPiece in hand
	}
	board[to] = captured;							// update dstPiece in board
#ifndef KPPT_DISABLE
	UndoBonaPiece(bonaPieceDiff[0]);
#endif

	turn = ~turn;
	ply--;
}

void Minishogi::DoNullMove() {
	ply++;
	turn = ~turn;

	StateInfo *st = &stateHist[ply];
	memcpy(st, &stateHist[ply - 1], offsetof(StateInfo, move));
	st->move = MOVE_NULL;
	st->capture = NO_PIECE;
	st->checker_bb = 0;
	st->continueCheck[0] = 0;
	st->continueCheck[1] = 0;
	st->nullMovePly = ply;
}

void Minishogi::UndoNullMove() {
	ply--;
	turn = ~turn;
}

/// 移動後 會不會造成對方處於被將狀態
Bitboard Minishogi::GetChecker() {
	Bitboard checker = 0;

	// get the position of my king
	const Bitboard kingboard = bitboard[KING | (turn << 4)];
	const Square kingpos = BitScan(kingboard);

	// get the possible position which might attack king
	const Bitboard totalOccupied = occupied[WHITE] | occupied[BLACK];
	Bitboard upper, lower, attackboard, attack_path;

	// row "-"
	upper = (totalOccupied & RowUpper[kingpos]) | HighestPosMask;
	lower = (totalOccupied & RowLower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard rank = (upper - lower) & RowMask(kingpos);
	attackboard = rank & occupied[~turn];

	// search the rook direction possible position
	while (attackboard) {
		Square attSrc = BitScan(attackboard);
		attack_path = Movable(attSrc, (Piece)board[attSrc], totalOccupied);
		if (attack_path & kingboard)
			checker |= (attack_path & rank) | (1 << attSrc);
		attackboard ^= 1 << attSrc;
	}

	// column "|"
	upper = (totalOccupied & ColUpper[kingpos]) | HighestPosMask;
	lower = (totalOccupied & ColLower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard file = (upper - lower) & ColMask(kingpos);
	attackboard = file & occupied[~turn];

	// search the rook direction possible position
	while (attackboard) {
		Square attSrc = BitScan(attackboard);
		attack_path = Movable(attSrc, (Piece)board[attSrc], totalOccupied);
		if (attack_path & kingboard)
			checker |= (attack_path & file) | (1 << attSrc);
		attackboard ^= 1 << attSrc;
	}

	// slope1 "/"
	upper = (totalOccupied & Slope1Upper[kingpos]) | HighestPosMask;
	lower = (totalOccupied & Slope1Lower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard slope1 = (upper - lower) & (Slope1Upper[kingpos] | Slope1Lower[kingpos]);
	attackboard = slope1 & occupied[~turn];

	// search the rook direction possible position 
	while (attackboard) {
		Square attSrc = BitScan(attackboard);
		attack_path = Movable(attSrc, (Piece)board[attSrc], totalOccupied);
		if (attack_path & kingboard)
			checker |= (attack_path & slope1) | (1 << attSrc);
		attackboard ^= 1 << attSrc;
	}

	// slope2 "\"
	upper = (totalOccupied & Slope2Upper[kingpos]) | HighestPosMask;
	lower = (totalOccupied & Slope2Lower[kingpos]) | LowestPosMask;
	upper = (upper & (~upper + 1)) << 1;
	lower = 1 << BitScanRev(lower);
	const Bitboard slope2 = (upper - lower) & (Slope2Upper[kingpos] | Slope2Lower[kingpos]);
	attackboard = slope2 & occupied[~turn];

	// search the rook direction possible position
	while (attackboard) {
		Square attSrc = BitScan(attackboard);
		attack_path = Movable(attSrc, (Piece)board[attSrc], totalOccupied);
		if (attack_path & kingboard)
			checker |= (attack_path & slope2) | (1 << attSrc);
		attackboard ^= 1 << attSrc;
	}

	return checker;
}

void Minishogi::CalcAllPin() {
#ifdef PIN_DISABLE
	stateHist[ply].eval.pin = VALUE_ZERO;
	return;
#endif
	Bitboard pinner, snipper, totalOccupied = occupied[WHITE] | occupied[BLACK];
	Square sq_wk = BitScan(bitboard[W_KING]), sq_bk = BitScan(bitboard[B_KING]);
	Value pin = VALUE_ZERO;

	snipper = (bitboard[B_ROOK] | bitboard[B_PRO_ROOK] & RookMask[sq_wk]) |
		(bitboard[B_BISHOP] | bitboard[B_PRO_BISHOP] & BishopMask[sq_wk]);

	while (snipper) {
		int attSrc = BitScan(snipper);
		snipper ^= 1 << attSrc;
		pinner = BetweenBB[sq_wk][attSrc] & totalOccupied;
		if (pinner && !more_than_one(pinner) && pinner & occupied[WHITE]) {
			pin += PIN_SCORE[board[BitScan(pinner)]];
		}
	}

	snipper = (bitboard[W_ROOK] | bitboard[W_PRO_ROOK] & RookMask[sq_bk]) |
		(bitboard[W_BISHOP] | bitboard[W_PRO_BISHOP] & BishopMask[sq_bk]);

	while (snipper) {
		int attSrc = BitScan(snipper);
		snipper ^= 1 << attSrc;
		pinner = BetweenBB[sq_bk][attSrc] & totalOccupied;
		if (pinner && !more_than_one(pinner) && pinner & occupied[BLACK]) {
			pin += PIN_SCORE[board[BitScan(pinner)]];
		}
	}
	stateHist[ply].eval.pin = pin;
}

#define KK  (GlobalEvaluater.kk)
#define KKP (GlobalEvaluater.kkp)
#define KPP (GlobalEvaluater.kpp)

void Minishogi::CalcAllPos() {
#ifdef KPPT_DISABLE
	return;
#endif
	Square sq_wk = BitScan(bitboard[W_KING]);
	Square sq_bk = BitScan(bitboard[B_KING]), sq_bki = rotate_board_sq(sq_bk);
	EvalSum sum = stateHist[ply].eval;

	sum.pos[0][0] = 0;
	sum.pos[0][1] = 0;
	sum.pos[1][0] = 0;
	sum.pos[1][1] = 0;
	sum.pos[2] = KK[sq_wk][sq_bk];
	for (int i = 0; i < BPI_KING; i++) {
		int w0 = pieceList[WHITE][i];
		int b0 = pieceList[BLACK][i];
		sum.pos[2] += KKP[sq_wk][sq_bk][w0];
		for (int j = 0; j < i; j++) {
			sum.pos[0] += KPP[sq_wk ][w0][pieceList[WHITE][j]];
			sum.pos[1] += KPP[sq_bki][b0][pieceList[BLACK][j]];
		}
	}
	stateHist[ply].eval = sum;
}

void Minishogi::CalcDiffPos() {
#ifdef KPPT_DISABLE
	return;
#endif
	if (ply == 0 || stateHist[ply - 1].eval.IsNotCalc()) {
		CalcAllPos();
		return;
	}
	Piece pc = (Piece)board[to_sq(stateHist[ply].move)];
	Piece capture = stateHist[ply].capture;
	BonaPieceDiff* bpd = stateHist[ply].bonaPieceDiff;
	Square sq_wk = BitScan(bitboard[W_KING]);
	Square sq_bk = BitScan(bitboard[B_KING]), sq_bki = SQ_E1 - sq_bk;
	EvalSum sum = stateHist[ply].eval;
	int i, j;

	sum.pos = stateHist[ply - 1].eval.pos;
	if (pc == W_KING) {
		sum.pos[0][0] = 0;
		sum.pos[0][1] = 0;
		sum.pos[2] = KK[sq_wk][sq_bk];
		for (i = 0; i < BPI_KING; i++) {
			int w0 = pieceList[WHITE][i];
			sum.pos[2] += KKP[sq_wk][sq_bk][w0];
			for (j = 0; j < i; j++) {
				sum.pos[0] += KPP[sq_wk][w0][pieceList[WHITE][j]];
			}
		}
		if (capture) {
			BonaPieceIndex capturerIndex = bonaIndexList[bpd[1].nowBonaW];
			for (i = 0; i < capturerIndex; ++i) {
				sum.pos[1] -= KPP[sq_bki][bpd[1].preBonaB][pieceList[BLACK][i]];
				sum.pos[1] += KPP[sq_bki][bpd[1].nowBonaB][pieceList[BLACK][i]];
			}
			for (++i; i < BPI_KING; ++i) {
				sum.pos[1] -= KPP[sq_bki][bpd[1].preBonaB][pieceList[BLACK][i]];
				sum.pos[1] += KPP[sq_bki][bpd[1].nowBonaB][pieceList[BLACK][i]];
			}
		}
	}
	else if (pc == B_KING) {
		sum.pos[1][0] = 0;
		sum.pos[1][1] = 0;
		sum.pos[2] = KK[sq_wk][sq_bk];
		for (int i = 0; i < BPI_KING; i++) {
			int w0 = pieceList[WHITE][i];
			int w1 = pieceList[BLACK][i];
			sum.pos[2] += KKP[sq_wk][sq_bk][w0];
			for (int j = 0; j < i; j++) {
				sum.pos[1] += KPP[sq_bki][w1][pieceList[WHITE][j]];
			}
		}
		if (capture) {
			BonaPieceIndex capturerIndex = bonaIndexList[bpd[1].nowBonaW];
			for (i = 0; i < capturerIndex; ++i) {
				sum.pos[0] -= KPP[sq_wk][bpd[1].preBonaW][pieceList[WHITE][i]];
				sum.pos[0] += KPP[sq_wk][bpd[1].nowBonaW][pieceList[WHITE][i]];
			}
			for (++i; i < BPI_KING; ++i) {
				sum.pos[0] -= KPP[sq_wk][bpd[1].preBonaW][pieceList[WHITE][i]];
				sum.pos[0] += KPP[sq_wk][bpd[1].nowBonaW][pieceList[WHITE][i]];
			}
		}
	}
	else {
#define ADD_BWKPP(BPD) { \
		assert(pieceList[WHITE][i] < BONA_PIECE_NB); \
		assert(pieceList[BLACK][i] < BONA_PIECE_NB); \
        sum.pos[0] -= KPP[sq_wk ][BPD.preBonaW][pieceList[WHITE][i]]; \
        sum.pos[1] -= KPP[sq_bki][BPD.preBonaB][pieceList[BLACK][i]]; \
        sum.pos[0] += KPP[sq_wk ][BPD.nowBonaW][pieceList[WHITE][i]]; \
        sum.pos[1] += KPP[sq_bki][BPD.nowBonaB][pieceList[BLACK][i]]; \
}
		if (!capture) {
			BonaPieceIndex moverIndex = bonaIndexList[bpd[0].nowBonaW];
			sum.pos[2] -= KKP[sq_wk][sq_bk][bpd[0].preBonaW];
			sum.pos[2] += KKP[sq_wk][sq_bk][bpd[0].nowBonaW];

			for (i = 0; i < moverIndex; ++i)
				ADD_BWKPP(bpd[0]);
			for (++i; i < BPI_KING; ++i)
				ADD_BWKPP(bpd[0]);
		}
		else {
			BonaPieceIndex moverIndex = bonaIndexList[bpd[0].nowBonaW];
			BonaPieceIndex capturerIndex = bonaIndexList[bpd[1].nowBonaW];
			if (moverIndex > capturerIndex)
				swap(moverIndex, capturerIndex);
			sum.pos[2] -= KKP[sq_wk][sq_bk][bpd[0].preBonaW];
			sum.pos[2] += KKP[sq_wk][sq_bk][bpd[0].nowBonaW];
			sum.pos[2] -= KKP[sq_wk][sq_bk][bpd[1].preBonaW];
			sum.pos[2] += KKP[sq_wk][sq_bk][bpd[1].nowBonaW];

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

			sum.pos[0] -= KPP[sq_wk ][bpd[0].preBonaW][bpd[1].preBonaW];
			sum.pos[1] -= KPP[sq_bki][bpd[0].preBonaB][bpd[1].preBonaB];
			sum.pos[0] += KPP[sq_wk ][bpd[0].nowBonaW][bpd[1].nowBonaW];
			sum.pos[1] += KPP[sq_bki][bpd[0].nowBonaB][bpd[1].nowBonaB];
		}
	}
	stateHist[ply].eval = sum;
}

bool Minishogi::PseudoLegal(Move m) const {
	Square from = from_sq(m), to = to_sq(m);
	// 不超出移動範圍
	if (from >= SQUARE_NB || to >= BOARD_NB)
		return false;

	Piece pc = GetChessOn(from), capture = GetChessOn(to);
	// 只能打入我方的手排
	if (pc == NO_PIECE || color_of(pc) != turn)
		return false;

	// 如果是吃子，不能是打入，且只能吃對方
	if (capture != NO_PIECE && (from >= BOARD_NB || color_of(capture) == turn))
		return false;

	// 理論上吃不到王，也不能讓自己被將
	if (type_of(capture) == KING || IsInCheckedAfter(m))
		return false;

	// 如果是移動(吃子)，驗證這顆棋子真的走的到
	if (!is_drop(from) && !(Movable(from, pc, occupied[WHITE] | occupied[BLACK]) & (1 << to)))
		return false;

	// 只允許某些棋子升變
	if (is_promote(m) && !(Promotable[pc] && ((1 << from | 1 << to) & EnemyCampMask[turn])))
		return false;

	return true;
}

ExtMove* Minishogi::AttackGenerator(ExtMove *moveList, Bitboard dstBB) const {
	// AttackGene moveList order by attacker
	static const Piece AttackerOrder[] = { PAWN, SILVER, GOLD, PRO_PAWN, PRO_SILVER, BISHOP, ROOK, PRO_BISHOP, PRO_ROOK, KING };
	Bitboard srcBoard, dstBoard, attackboard = stateHist[ply].checker_bb;
	Bitboard opboard = occupied[~turn] & dstBB, totalOccupied = occupied[WHITE] | occupied[BLACK];
	Square src, dst;
	int turnBit = turn << 4;

	if (!attackboard)
		attackboard = BitboardMask;

	for (int i = 0; i < 10; i++) {
		srcBoard = bitboard[AttackerOrder[i] | turnBit];
		while (srcBoard) {
			src = BitScan(srcBoard);
			srcBoard ^= 1 << src;
			dstBoard = Movable(src, (Piece)board[src], totalOccupied) & opboard;
			if (attackboard && AttackerOrder[i] != KING)
				dstBoard &= attackboard;
			while (dstBoard) {
				dst = turn ? BitScanRev(dstBoard) : BitScan(dstBoard);
				dstBoard ^= 1 << dst;
				*moveList++ = make_move(src, dst,
					Promotable[AttackerOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[turn]));

				//if (AttackerOrder[i] != PAWN && is_promote(*(moveList - 1))) {
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
	static const Piece MoverOrder[] = { PRO_ROOK, PRO_BISHOP, ROOK, BISHOP, PRO_SILVER, PRO_PAWN, GOLD, SILVER, PAWN, KING };
	Bitboard srcBoard, dstBoard, attackboard = stateHist[ply].checker_bb;
	Bitboard totalOccupied = occupied[WHITE] | occupied[BLACK], blankboard = BlankOccupied(totalOccupied);
	Square src, dst;
	int turnBit = turn << 4;

	for (int i = 0; i < 10; i++) {
		srcBoard = bitboard[MoverOrder[i] | turnBit];
		while (srcBoard) {
			src = BitScan(srcBoard);
			srcBoard ^= 1 << src;
			dstBoard = Movable(src, (Piece)board[src], totalOccupied) & blankboard;
			if (attackboard && MoverOrder[i] != KING) 
				dstBoard &= attackboard;
			while (dstBoard) {
				dst = turn ? BitScanRev(dstBoard) : BitScan(dstBoard);
				dstBoard ^= 1 << dst;
				*moveList++ = make_move(src, dst,
					Promotable[MoverOrder[i]] && ((1 << src | 1 << dst) & EnemyCampMask[turn]));
				
				//if (MoverOrder[i] != PAWN && is_promote(*(moveList - 1))) {
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

	Bitboard srcBoard = BlankOccupied(occupied[WHITE] | occupied[BLACK]), dstBoard, nifu = 0;
	Bitboard checker = stateHist[ply].checker_bb & occupied[~turn];
	if (checker) {                    
		if (checker & (checker - 1)) // if there are more than one checkers
			return moveList;         // no need to generate

		// if there is only one checker, only the path from checker to my king can be blocked
		srcBoard &= stateHist[ply].checker_bb;
	}

	++src;
	for (int i = 0; i < 4; i++) { // 步以外的手排
		if (board[--src]) {
			dstBoard = srcBoard;
			while (dstBoard) {
				dst = BitScan(dstBoard);
				dstBoard ^= 1 << dst;
				*moveList++ = make_move(src, dst, false);
			}
		}
	}

	if (board[--src]) { // 步
		dstBoard = bitboard[(turn << 4) | PAWN]; // 我方的步
		if (dstBoard)
			nifu |= ColMask(BitScan(dstBoard)); // 二步

		// 打步詰
		Bitboard kingboard = bitboard[turn ? W_KING : B_KING];
		Bitboard pawnboard = turn ? kingboard >> 5 : kingboard << 5;

		if (checker == 0 && (pawnboard & srcBoard)) {
			Bitboard uchifuzume = pawnboard; // 假設有打步詰
			Square kingpos = BitScan(kingboard), pawnpos = BitScan(pawnboard);

			// DoMove
			occupied[turn] ^= pawnboard;
			bitboard[PAWN | (turn << 4)] ^= pawnboard;
			turn = ~turn;
			Bitboard totalOccupied = occupied[WHITE] | occupied[BLACK];

			// 對方王可 吃/移動 的位置
			dstBoard = Movement[KING][kingpos];
			dstBoard &= dstBoard ^ occupied[turn];
			while (dstBoard) {
				dst = BitScan(dstBoard);
				// 如果王移動後脫離被王手
				if (!IsInCheckedAfter(kingpos, dst)) {
					uchifuzume = 0; // 代表沒有打步詰
					break;
				}
				dstBoard ^= 1 << dst;
			}

			// 對方可能攻擊到步的棋子 (不包括王)
			if (uchifuzume) {
				Bitboard attackboard = ((RookMovable(pawnpos, totalOccupied) | BishopMovable(pawnpos, totalOccupied)) & occupied[turn]) ^ kingboard;
				while (attackboard) {
					Square attSrc = BitScan(attackboard);
					// 如果真的吃得到步 且 吃了之後不會被王手
					if ((Movable(attSrc, (Piece)board[attSrc], totalOccupied) & pawnboard) && !IsInCheckedAfter(attSrc, pawnpos)) {
						uchifuzume = 0; // 代表沒有打步詰
						break;
					}
					attackboard ^= 1 << attSrc;
				}
				nifu |= uchifuzume;
			}

			// UndoMove
			turn = ~turn;
			occupied[turn] ^= pawnboard;
			bitboard[PAWN | (turn << 4)] ^= pawnboard;
		}

		dstBoard = srcBoard & ~(EnemyCampMask[turn] | nifu);
		while (dstBoard) {
			dst = BitScan(dstBoard);
			dstBoard ^= 1 << dst;
			*moveList++ = make_move(src, dst, false);
		}
	}
	return moveList;
}

Move* Minishogi::GetTotalMoves(Move* moveList) {
	ExtMove start[TOTAL_GENE_MAX_MOVES], *end;
	end = AttackGenerator(start);
	end = MoveGenerator(end);
	end = HandGenerator(end);
	for (ExtMove* i = start; i < end; i++)
		*moveList++ = *i;
	return moveList;
}

Move* Minishogi::GetLegalMoves(Move* moveList) {
	ExtMove start[TOTAL_GENE_MAX_MOVES], *end;
	end = AttackGenerator(start);
	end = MoveGenerator(end);
	end = HandGenerator(end);
	for (ExtMove* i = start; i < end; i++) {
		if (!IsInCheckedAfter(*i)) {
			DoMove(*i);
			auto type = SennichiteType(256);
			if (type == NO_SENNICHITE || type == SENNICHITE_LOSE)
				*moveList++ = *i;
			UndoMove();
		}
	}
	return moveList;
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
	string ext = get_extension(filename);
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
			if (ext == "fen")
				buf = fen2sfen(buf);
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
		//os << moveHist[i] << setw(18) << hex << GetKey(i + 1) << dec << "\n";
	}
}

bool Minishogi::IsGameOver() {
	Move moveList[TOTAL_GENE_MAX_MOVES];
	return moveList == GetLegalMoves(moveList);
}

bool Minishogi::IsLegelAction(Move m) {
	Move moveList[TOTAL_GENE_MAX_MOVES], *end = GetLegalMoves(moveList);
	return end != find(moveList, end, m);
}

bool Minishogi::IsCheckingAfter(Move m) {
	assert(m != MOVE_NULL);
	const Square srcIndex = from_sq(m), dstIndex = to_sq(m);
	const int dstChess = board[dstIndex];
	board[dstIndex] = GetChessOn(srcIndex);
	const bool isCheckable = Movable(dstIndex, (Piece)board[dstIndex], occupied[WHITE] | occupied[BLACK]) & bitboard[KING | ((~turn) << 4)];
	board[dstIndex] = dstChess;

	if (is_drop(srcIndex)) return isCheckable;
	else if (isCheckable) return true;

	const Bitboard my_occupied = occupied[turn] ^ (1 << srcIndex);
	const Bitboard tmp_occupied = occupied[~turn] | my_occupied;

	// get the position of the checking king
	const Square kingpos = BitScan(bitboard[KING | ((~turn) << 4)]);

	// get my possible position which might attack the opponent king
	Bitboard attackboard = RookMovable(kingpos, tmp_occupied) & my_occupied;

	// search the possible position
	while (attackboard) {
		int attSrc = BitScan(attackboard);
		if ((board[attSrc] & 7) == ROOK) return true;
		attackboard ^= 1 << attSrc;
	}

	attackboard = BishopMovable(kingpos, tmp_occupied) & my_occupied;
	while (attackboard) {
		int attSrc = BitScan(attackboard);
		if ((board[attSrc] & 7) == BISHOP) return true;
		attackboard ^= 1 << attSrc;
	}

	return false;
}

bool Minishogi::IsInCheckedAfter(Square srcIndex, Square dstIndex) const {
	const Bitboard dstBoard = 1 << dstIndex;
    Bitboard op_occupied = occupied[~turn];
    if (board[dstIndex]) // eat
        op_occupied ^= dstBoard;

    const Bitboard tmp_occupied = (occupied[turn] | op_occupied) ^ ((!is_drop(srcIndex) ? (1 << srcIndex) : 0) | dstBoard);

    // get the position of the checked king
    Bitboard kingboard = dstBoard;
	Square kingpos = dstIndex;
    if (type_of(board[srcIndex]) != KING) {
        kingboard = bitboard[KING | (turn << 4)];
        kingpos = BitScan(kingboard);
    }

	// get the possible position which might attack king
    Bitboard attackboard = AttackersTo(kingpos, tmp_occupied) & op_occupied;

	// search the possible position
	while (attackboard) {
		Square attSrc = BitScan(attackboard);
		if (Movable(attSrc, (Piece)board[attSrc], tmp_occupied) & kingboard) return true;
		attackboard ^= 1 << attSrc;
	}

	return false;
}

SennichiteType Minishogi::SennichiteType(int checkMaxPly) const {
	const int stop = max(ply - checkMaxPly, stateHist[ply].nullMovePly);
	for (int i = ply - 4; i >= stop; i -= 2) {
		if (stateHist[i].key == stateHist[ply].key) {
			if (stateHist[ply].continueCheck[~turn] > ply - i)
				return SENNICHITE_CHECK;
			if (turn == WHITE)
				return SENNICHITE_LOSE;
			if (stateHist[ply].continueCheck[BLACK] < ply - i)
				return SENNICHITE_WIN;
			return NO_SENNICHITE;
		}
	}
	return NO_SENNICHITE;
}

bool Minishogi::SEE(Move move, Value threshold) const {
	const Square srcIndex = from_sq(move), dstIndex = to_sq(move);
	const Bitboard moveBoard = is_drop(srcIndex) ? 0 : (1 << srcIndex);
	const Bitboard movedOccupied = (occupied[WHITE] | occupied[BLACK]) ^ moveBoard;
	const Bitboard psbBoard = AttackersTo(dstIndex, movedOccupied);
	const Bitboard dstBoard = 1 << dstIndex;

	Value balance = PIECE_SCORE[type_of(board[dstIndex])] - threshold;
	Value myValueList[8], opValueList[8], *myValueEnd = myValueList, *opValueEnd = opValueList;

	// Add opValueList
	Bitboard srcBoard = psbBoard & occupied[~turn];
	while (srcBoard) {
		Square attSrc = BitScan(srcBoard);
		srcBoard ^= 1 << attSrc;
		if (Movable(attSrc, (Piece)board[attSrc], movedOccupied) & dstBoard)
			*opValueEnd++ = PIECE_SCORE[type_of(board[attSrc])];
	}
	if (opValueEnd == opValueList) return balance >= 0; // 對方都吃不到我下的位置

	balance -= PIECE_SCORE[type_of(GetChessOn(srcIndex))]; // 我方移動子被吃

	// Add myValueList
	srcBoard = psbBoard & (occupied[turn] ^ moveBoard);
	while (srcBoard) {
		Square attSrc = BitScan(srcBoard);
		srcBoard ^= 1 << attSrc;
		if (Movable(attSrc, (Piece)board[attSrc], movedOccupied) & dstBoard)
			*myValueEnd++ = PIECE_SCORE[type_of(board[attSrc])];
	}

	for (Value *opValue = opValueList, *myValue = myValueList; myValue < myValueEnd;) { // 我方能吃對方
		swap(*opValue, *min_element(opValue, opValueEnd));
		balance += *opValue++; // 對方被吃掉
		if (opValue < opValueEnd) {	// 對方能吃我方
			swap(*myValue, *min_element(myValue, myValueEnd));
			balance -= *myValue++; // 我方被吃掉
		}
		else break;
	}

	return balance >= 0;
}

std::ostream& operator<<(std::ostream& os, const Minishogi& pos) {
	stringstream ss;
	ss << "[" << COLOR_WORD[pos.turn] << "]" << pos.Sfen() << "\n";
	ss << "  5   4   3   2   1\n";
	for (Square sq = SQUARE_ZERO; sq < BOARD_NB; ++sq) {
		if (pos.board[sq] == NO_PIECE)
			ss << " ． ";
		else
			ss << NONCOLOR_PIECE_WORD.substr(pos.board[sq] * 4, 4);
		if (sq % 5 == 4)
			ss << " " << (char)('a' + sq / 5) << "\n";
	}
	ss << COLOR_WORD[BLACK] << " : ";
	for (int i = 0; i < 5; i++)
		ss << pos.board[i + SQ_B_HAND] << PIECE_WORD.substr((i + 1) * 2, 2);
	ss << "\n" << COLOR_WORD[WHITE] << " : ";
	for (int i = 0; i < 5; i++)
		ss << pos.board[i + SQ_W_HAND] << PIECE_WORD.substr((i + 1) * 2, 2);
	ss << "\nConti. Check : " << pos.stateHist[pos.GetPly()].continueCheck[0] << " " << pos.stateHist[pos.GetPly()].continueCheck[1];
	ss << "\nChecker : " << pos.stateHist[pos.GetPly()].checker_bb;
	os << ss.str();

	return os;
}