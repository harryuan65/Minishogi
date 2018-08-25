#include <fstream>
#define NOMINMAX
#include <atlstr.h>

#include "Evaluate.h"
#include "Thread.h"

namespace Evaluate {
	Evaluater evaluater;

	void Evaluater::Init() {
		for (int i = 0; i < BOARD_NB; i++) {
			for (int j = 0; j < BOARD_NB; j++) {
				kk[i][j][0] = rand() % 100;
				kk[i][j][1] = rand() % 100;
			}
		}
		for (int i = 0; i < BOARD_NB; i++) {
			for (int j = 0; j < BOARD_NB; j++) {
				for (int k = 0; k < BONA_PIECE_NB; k++) {
					kkp[i][j][k][0] = rand() % 100;
					kkp[i][j][k][1] = rand() % 100;
				}
			}
		}
		for (int i = 0; i < BOARD_NB; i++) {
			for (int j = 0; j < BONA_PIECE_NB; j++) {
				for (int k = 0; k < j; k++) {
					kpp[i][j][k][0] = rand() % 100;
					kpp[i][j][k][1] = rand() % 100;
					kpp[i][k][j][0] = kpp[i][j][k][0];
					kpp[i][k][j][1] = kpp[i][j][k][1];
				}
			}
		}
	}

	void Evaluater::Load(string path) {
		ifstream ifsKK(EVAL_PATH KK_FILE, ios::binary);
		ifstream ifsKKP(EVAL_PATH KKP_FILE, ios::binary);
		ifstream ifsKPP(EVAL_PATH KPP_FILE, ios::binary);
		if (!ifsKK || !ifsKKP || !ifsKPP) {
			cout << "Error : Evaluater load failed." << endl;
			return;
		}

		ifsKK.read(reinterpret_cast<char*>(kk), sizeof(kk));
		ifsKKP.read(reinterpret_cast<char*>(kkp), sizeof(kkp));
		ifsKPP.read(reinterpret_cast<char*>(kpp), sizeof(kpp));
	}

	void Evaluater::Save(string path) {
		CreateDirectory(CA2W(EVAL_PATH), NULL);

		ofstream ofsKK(EVAL_PATH KK_FILE, ios::binary);
		ofstream ofsKKP(EVAL_PATH KKP_FILE, ios::binary);
		ofstream ofsKPP(EVAL_PATH KPP_FILE, ios::binary);

		if (!ofsKK.write(reinterpret_cast<char*>(kk), sizeof(kk)) ||
			!ofsKKP.write(reinterpret_cast<char*>(kkp), sizeof(kkp)) ||
			!ofsKPP.write(reinterpret_cast<char*>(kpp), sizeof(kpp)))
			cout << "Error : Evaluater save failed." << endl;
		else
			cout << "Evaluater save successed." << endl;
	}

	void Evaluater::Clean() {
		memset(this, 0, sizeof(Evaluater));
	}

	Value EvalSum::Sum(const Color c) const {
		if (meterial == VALUE_NONE || pin == VALUE_NONE)
			return VALUE_NONE;
		// [0](先手玉) + [1](後手玉) + [2](KK+KKP) 
		const Value scoreBoard = (Value)(pos[0][0] - pos[1][0] + pos[2][0]) + meterial + pin;
		const Value scoreTurn = (Value)(pos[0][1] + pos[1][1] + pos[2][1]);

		return (c == WHITE ? scoreBoard : -scoreBoard) + scoreTurn;
	}

	Value EvalSum::PosSum(const Color c) const {
		if (meterial == VALUE_NONE || pin == VALUE_NONE)
			return VALUE_NONE;
		// [0](先手玉) + [1](後手玉) + [2](KK+KKP) 
		const Value scoreBoard = (Value)(pos[0][0] - pos[1][0] + pos[2][0]) + meterial + pin;
		const Value scoreTurn = (Value)(pos[0][1] + pos[1][1] + pos[2][1]);

		return (c == WHITE ? scoreBoard : -scoreBoard) + scoreTurn;
	}

	void EvalSum::Clean() {
		memset(this, 0, sizeof(EvalSum));
		meterial = VALUE_NONE;
		pin = VALUE_NONE;
	}

	void Initialize() {
		evaluater.Init();
	}
}

/*void Evaluate::CalcAllPin(Minishogi &b) {
	Bitboard pinner, snipper, totalOccupied = b.GetOccupied(WHITE) | b.GetOccupied(BLACK);
	Square sq_wk = BitScan(b.GetBitboard(W_KING));
	Square sq_bk = BitScan(b.GetBitboard(B_KING));
	EvalSum* sum = b.GetEvalSum();
	sum->pin = VALUE_ZERO;

	snipper = (b.GetBitboard(B_ROOK) | b.GetBitboard(B_PRO_ROOK) & RookMask[sq_wk]) |
		(b.GetBitboard(B_BISHOP) | b.GetBitboard(B_PRO_BISHOP) & BishopMask[sq_wk]);

	while (snipper) {
		int attsrc = BitScan(snipper);
		snipper ^= 1 << attsrc;
		pinner = BetweenBB[sq_wk][attsrc] & totalOccupied;
		if (pinner && !more_than_one(pinner) && pinner & b.GetOccupied(WHITE)) {
			sum->pin += PIN_SCORE[b.GetBoard(BitScan(pinner))];
		}
	}

	snipper = (b.GetBitboard(W_ROOK) | b.GetBitboard(W_PRO_ROOK) & RookMask[sq_bk]) |
		(b.GetBitboard(W_BISHOP) | b.GetBitboard(W_PRO_BISHOP) & BishopMask[sq_bk]);

	while (snipper) {
		int attsrc = BitScan(snipper);
		snipper ^= 1 << attsrc;
		pinner = BetweenBB[sq_bk][attsrc] & totalOccupied;
		if (pinner && !more_than_one(pinner) && pinner & b.GetOccupied(BLACK)) {
			sum->pin += PIN_SCORE[b.GetBoard(BitScan(pinner))];
		}
	}
}

void Evaluate::CalcAllPos(Minishogi &b) {
	const auto kk = b.GetThread()->evaluater.Kk;
	const auto kkp = b.GetThread()->evaluater.Kkp;
	const auto kpp = b.GetThread()->evaluater.Kpp;
	const auto pieceListW = b.GetPieceList(WHITE);
	const auto pieceListB = b.GetPieceList(BLACK);

	Square sq_wk = BitScan(b.GetBitboard(W_KING));
	Square sq_bk = BitScan(b.GetBitboard(B_KING)), sq_bki = Square(BOARD_NB - sq_bk);
	EvalSum* sum = b.GetEvalSum();

	sum->pos[0][0] = 0;
	sum->pos[0][1] = 0;
	sum->pos[1][0] = 0;
	sum->pos[1][1] = 0;
	sum->pos[2] = kk[sq_wk][sq_bk];
	for (int i = 0; i < KING_INDEX; i++) {
		int w0 = pieceListW[i];
		int b0 = pieceListB[i];
		sum->pos[0] += kkp[sq_wk][sq_bk][w0];
		for (int j = 0; j < i; j++) {
			sum->pos[0] += kpp[sq_wk][w0][pieceListW[j]];
			sum->pos[1] += kpp[sq_bki][b0][pieceListB[j]];
		}
	}
}

void CalcDiffPos(Minishogi &b) {
	const auto kk = b.GetThread()->evaluater.Kk;
	const auto kkp = b.GetThread()->evaluater.Kkp;
	const auto kpp = b.GetThread()->evaluater.Kpp;
	const auto pieceListW = b.GetPieceList(WHITE);
	const auto pieceListB = b.GetPieceList(BLACK);

	Square sq_wk = BitScan(b.GetBitboard(W_KING));
	Square sq_bk = BitScan(b.GetBitboard(B_KING)), sq_bki = Square(BOARD_NB - sq_bk);
	EvalSum* sum = b.GetEvalSum();

	if (pc == W_KING) {
		sum->pos[0][0] = 0;
		sum->pos[0][1] = 0;
		sum->pos[2] = kk[sq_wk][sq_bk];
		for (int i = 0; i < KING_INDEX; i++) {
			int w0 = pieceListW[i];
			sum->pos[2] += kkp[sq_wk][sq_bk][w0];
			for (int j = 0; j < i; j++) {
				sum->pos[0] += kpp[sq_wk][w0][pieceListW[j]];
			}
		}
		if (capture) {
		}
	}
	else if (pc == B_KING) {
		sum->pos[1][0] = 0;
		sum->pos[1][1] = 0;
		sum->pos[2] = kk[sq_wk][sq_bk];
		for (int i = 0; i < KING_INDEX; i++) {
			int w0 = pieceListW[i];
			int w1 = pieceListB[i];
			sum->pos[2] += kkp[sq_wk][sq_bk][w0];
			for (int j = 0; j < i; j++) {
				sum->pos[1] += kpp[sq_bki][w1][pieceListW[j]];
			}
		}
		if (capture) {
			int i;
			for (i = 0; i < dirty; ++i) {
				sum->pos[0] -= kpp[sq_wk][k0][list_fb[i]];
				sum->pos[0] += kpp[sq_wk][k2][list_fb[i]];
			}
			for (++i; i < PIECE_NO_KING; ++i) {
				sum->pos[0] -= kpp[sq_wk][k0][list_fb[i]];
				sum->pos[0] += kpp[sq_wk][k2][list_fb[i]];
			}
		}
	}
	else {

	}
}*/