#include <algorithm>
#include <assert.h>
#include <iostream>
#include "Evaluate.h"
#include "EvaluateLearn.h"
using namespace Evaluate;

namespace EvaluateLearn {
	LearnFloatType Weight::eta = 64.0f;
	int Weight::skip_count = 10;

	Weight(*kk_w)[BOARD_NB][BOARD_NB];
	Weight(*kkp_w)[BOARD_NB][BOARD_NB][BONA_PIECE_NB];
	Weight(*kpp_w)[BOARD_NB][BONA_PIECE_NB][BONA_PIECE_NB];

#define KK  (evaluater.kk)
#define KKP (evaluater.kkp)
#define KPP (evaluater.kpp)
#define KKW (*kk_w)
#define KKPW (*kkp_w)
#define KPPW (*kpp_w)
#define SET_A_LIMIT_TO(X,MIN,MAX)  \
    X[0] = std::min(X[0],(MAX));   \
    X[0] = std::max(X[0],(MIN));   \
    X[1] = std::min(X[1],(MAX));   \
    X[1] = std::max(X[1],(MIN));

	void WriteKPP(Square k1, BonaPiece p1, BonaPiece p2, ValueKpp value) {
		Square mk1 = mirror_board_sq(k1);
		BonaPiece mp1 = mirror_bonapiece(p1);
		BonaPiece mp2 = mirror_bonapiece(p2);

		assert(KPP[k1][p1][p2] == KPP[k1][p2][p1]);
		assert(KPP[k1][p1][p2] == KPP[mk1][mp1][mp2]);
		assert(KPP[k1][p1][p2] == KPP[mk1][mp2][mp1]);

		KPP[k1][p1][p2] = value;
		KPP[k1][p2][p1] = value;
		KPP[mk1][mp1][mp2] = value;
		KPP[mk1][mp2][mp1] = value;
	}

	uint64_t GetKPPIndex(Square k1, BonaPiece p1, BonaPiece p2) {
		const ValueKpp* q0 = &KPP[0][0][0];
		BonaPiece mp1 = mirror_bonapiece(p1);
		BonaPiece mp2 = mirror_bonapiece(p2);
		Square mk1 = mirror_board_sq(k1);

		assert(k1 < BOARD_NB);
		assert(mk1 < BOARD_NB);
		assert(p1 < BONA_PIECE_NB);
		assert(p2 < BONA_PIECE_NB);
		assert(mp1 < BONA_PIECE_NB);
		assert(mp2 < BONA_PIECE_NB);

		int64_t q1 = &KPP[k1][p1][p2] - q0;
		int64_t q2 = &KPP[k1][p2][p1] - q0;
		int64_t q3 = &KPP[mk1][mp1][mp2] - q0;
		int64_t q4 = &KPP[mk1][mp2][mp1] - q0;

		return std::min({ q1, q2, q3, q4 });
	}

	inline void Weight::addGrad(LearnFloatType g1, LearnFloatType g2) {
		grad[0] += g1;
		grad[1] += g2;
	}

	bool Weight::update(bool skip_update) {
		if (grad[0] == 0 && grad[1] == 0)
			return false;

		grad2[0] += grad[0] * grad[0];
		grad2[1] += grad[1] * grad[1];

		// 跳過太小的值
		if (!skip_update) {
			if (grad2[0] >= 0.1f)
				weight[0] -= eta * grad[0] / sqrt(grad2[0]);

			if (grad2[1] >= 0.1f)
				weight[1] -= eta * grad[1] / sqrt(grad2[1]);
		}

		grad = { 0, 0 };
		return !skip_update;
	}

	void InitGrad() {
		if (kk_w == nullptr) {
			const int sizekk  = BOARD_NB * BOARD_NB;
			const int sizekkp = BOARD_NB * BOARD_NB * BONA_PIECE_NB;
			const int sizekpp = BOARD_NB * BONA_PIECE_NB * BONA_PIECE_NB;
			kk_w  = (Weight(*)[BOARD_NB][BOARD_NB]) new Weight[sizekk];
			kkp_w = (Weight(*)[BOARD_NB][BOARD_NB][BONA_PIECE_NB]) new Weight[sizekkp];
			kpp_w = (Weight(*)[BOARD_NB][BONA_PIECE_NB][BONA_PIECE_NB]) new Weight[sizekpp];
			memset(kk_w , 0, sizeof(Weight) * sizekk);
			memset(kkp_w, 0, sizeof(Weight) * sizekkp);
			memset(kpp_w, 0, sizeof(Weight) * sizekpp);

			for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1) {
				for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2) {
					KKW[k1][k2].weight[0] = LearnFloatType(KK[k1][k2][0]);
					KKW[k1][k2].weight[1] = LearnFloatType(KK[k1][k2][1]);
					for (BonaPiece p = BONA_PIECE_ZERO; p < BONA_PIECE_NB; ++p) {
						KKPW[k1][k2][p].weight[0] = LearnFloatType(KKP[k1][k2][p][0]);
						KKPW[k1][k2][p].weight[1] = LearnFloatType(KKP[k1][k2][p][1]);
					}
				}
			}
			for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1) {
				for (BonaPiece p1 = BONA_PIECE_ZERO; p1 < BONA_PIECE_NB; ++p1) {
					for (BonaPiece p2 = BONA_PIECE_ZERO; p2 < BONA_PIECE_NB; ++p2) {
						KPPW[k1][p1][p2].weight[0] = LearnFloatType(KPP[k1][p1][p2][0]);
						KPPW[k1][p1][p2].weight[1] = LearnFloatType(KPP[k1][p1][p2][1]);
					}
				}
			}
		}
	}

	double CalcGrad(Value searchValue, Value quietValue, bool winner, double progress) {
		if (searchValue == VALUE_NULL || quietValue == VALUE_NULL)
			return 0.0;
		double p = winest(searchValue);
		double q = winest(quietValue);
		double w = winner ? 1.0 : 0.0;
		double o = LAMBDA * (w - p) * progress;
		return q - (p + o);
	}

	void AddGrad(const Minishogi &m, Color rootTurn, double delta_grad) {
		const BonaPiece* list_fw = m.GetPieceList(WHITE);
		const BonaPiece* list_fb = m.GetPieceList(BLACK);
		Square sq_wk = BitScan(m.GetBitboard(W_KING));
		Square sq_bk = BitScan(m.GetBitboard(B_KING)), sq_bki = rotate_board_sq(sq_bk);
		LearnFloatType f = (rootTurn == WHITE) ? LearnFloatType(delta_grad) : -LearnFloatType(delta_grad);
		LearnFloatType g = (rootTurn == m.GetTurn()) ? LearnFloatType(delta_grad) : -LearnFloatType(delta_grad);

		KKW[sq_wk][sq_bk].addGrad(f, g);
		for (int i = 0; i < BPI_KING; i++) {
			BonaPiece k0 = list_fw[i];
			BonaPiece k1 = list_fb[i];
			KKPW[sq_wk][sq_bk][k0].addGrad(f, g);
			for (int j = 0; j < i; j++) {
				((Weight*)kpp_w)[GetKPPIndex(sq_wk , k0, list_fw[j])].addGrad(f, g);
				((Weight*)kpp_w)[GetKPPIndex(sq_bki, k1, list_fb[j])].addGrad(-f, g);
			}
		}
		
	}

	void UpdateKPPT(uint64_t epoch) {
		const bool skip_update = epoch <= (uint64_t)Weight::skip_count;

		auto func = [&](size_t id) {
			for (Square k1 = Square(5 * id); k1 < (Square)(5 * (id + 1)); ++k1) {
				for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2) {
					Weight& w = KKW[k1][k2];
					if (w.update(skip_update)) {
						SET_A_LIMIT_TO(w.weight, LearnFloatType((int32_t)INT16_MIN * 4), LearnFloatType((int32_t)INT16_MAX * 4));
						KK[k1][k2] = { (int32_t)std::round(w.weight[0]), (int32_t)std::round(w.weight[1]) };
					}
				}
			}

			for (Square k1 = Square(5 * id); k1 < (Square)(5 * (id + 1)); ++k1) {
				for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2) {
					for (BonaPiece p = BONA_PIECE_ZERO; p < BONA_PIECE_NB; ++p) {
						Weight& w = KKPW[k1][k2][p];
						if (w.update(skip_update)) {
							SET_A_LIMIT_TO(w.weight, (LearnFloatType)(INT16_MIN / 2), (LearnFloatType)(INT16_MAX / 2));
							KKP[k1][k2][p] = ValueKkp{ (int32_t)std::round(w.weight[0]), (int32_t)std::round(w.weight[1]) };
						}
					}
				}
			}

			for (Square k = Square(5 * id); k < (Square)(5 * (id + 1)); ++k) {
				for (BonaPiece p1 = BONA_PIECE_ZERO; p1 < BONA_PIECE_NB; ++p1) {
					for (BonaPiece p2 = BONA_PIECE_ZERO; p2 < BONA_PIECE_NB; ++p2) {
						Weight& w = KPPW[k][p1][p2];
						if (w.update(skip_update)) {
							SET_A_LIMIT_TO(w.weight, (LearnFloatType)(INT16_MIN / 2), (LearnFloatType)(INT16_MAX / 2));
							WriteKPP(k, p1, p2, ValueKpp{ (int16_t)std::round(w.weight[0]), (int16_t)std::round(w.weight[1]) });
						}
					}
				}
			}
		};

		std::thread th[5];

		for (int i = 0; i < 5; i++)
			th[i] = std::thread(func, i);

		for (int i = 0; i < 5; i++)
			th[i].join();
	}
}