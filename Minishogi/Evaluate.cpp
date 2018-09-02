#include <iostream>
#include <fstream>
#define NOMINMAX
#include <windows.h>

#include "Evaluate.h"
#include "Thread.h"
#include "Observer.h"
using namespace std;

namespace Evaluate {
	Evaluater evaluater;

	bool Evaluater::Load(std::string kkptName) {
		ifstream ifKK(KPPT_DIRPATH + kkptName + "//" + KK_FILENAME, ios::binary);
		ifstream ifKKP(KPPT_DIRPATH + kkptName + "//" + KKP_FILENAME, ios::binary);
		ifstream ifKPP(KPPT_DIRPATH + kkptName + "//" + KPP_FILENAME, ios::binary);
		if (!ifKK || !ifKKP || !ifKPP) {
			cout << "Error : Evaluater load failed." << endl;
			Observer::LearnLog << Observer::GetTimeStamp() << " Load KKPT from " << + KPPT_DIRPATH << kkptName << " failed.\n";
			return false;
		}

		ifKK.read(reinterpret_cast<char*>(kk), sizeof(kk));
		ifKKP.read(reinterpret_cast<char*>(kkp), sizeof(kkp));
		ifKPP.read(reinterpret_cast<char*>(kpp), sizeof(kpp));
		cout << "Evaluater load successed." << endl;
		Observer::LearnLog << Observer::GetTimeStamp() << " Load KKPT from " << KPPT_DIRPATH << kkptName << " success.\n";
		CheckNonZero();
		return true;
	}

	bool Evaluater::Save(std::string kkptName) {
		CreateDirectory((KPPT_DIRPATH + kkptName).c_str(), NULL);

		ofstream ofKK(KPPT_DIRPATH + kkptName + "//" + KK_FILENAME, ios::binary);
		ofstream ofKKP(KPPT_DIRPATH + kkptName + "//" + KKP_FILENAME, ios::binary);
		ofstream ofKPP(KPPT_DIRPATH + kkptName + "//" + KPP_FILENAME, ios::binary);

		if (!ofKK.write(reinterpret_cast<char*>(kk), sizeof(kk)) ||
			!ofKKP.write(reinterpret_cast<char*>(kkp), sizeof(kkp)) ||
			!ofKPP.write(reinterpret_cast<char*>(kpp), sizeof(kpp))) {
			cout << "Error : Evaluater save failed." << endl; 
			Observer::LearnLog << Observer::GetTimeStamp() << " Save KKPT to " << KPPT_DIRPATH << kkptName << " failed.\n";
			return false;
		}
		else {
			cout << "Evaluater save successed." << endl;
			Observer::LearnLog << Observer::GetTimeStamp() << " Save KKPT to " << KPPT_DIRPATH << kkptName << " success.\n";
			return true;
		}
	}

	void Evaluater::Clean() {
		memset(this, 0, sizeof(Evaluater));
		cout << "Evaluater Clean." << endl;
		Observer::LearnLog << Observer::GetTimeStamp() << " Clean KKPT.\n";
	}

	void Evaluater::Blend(Evaluater &e, float ratio) {
		float ratio2 = 1 - ratio;
		for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1)
			for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2)
				for (int i = 0; i < 2; i++)
					kk[k1][k2][i] = kk[k1][k2][i] * ratio + e.kk[k1][k2][i] * ratio2;

		for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1)
			for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2)
				for (BonaPiece p = BONA_PIECE_ZERO; p < BONA_PIECE_NB; ++p)
					for (int i = 0; i < 2; i++)
						kkp[k1][k2][p][i] = kkp[k1][k2][p][i] * ratio + e.kkp[k1][k2][p][i] * ratio2;

		for (Square k = SQUARE_ZERO; k < BOARD_NB; ++k)
			for (BonaPiece p1 = BONA_PIECE_ZERO; p1 < BONA_PIECE_NB; ++p1)
				for (BonaPiece p2 = BONA_PIECE_ZERO; p2 < BONA_PIECE_NB; ++p2)
					for (int i = 0; i < 2; i++)
						kpp[k][p1][p2][i] = kpp[k][p1][p2][i] * ratio + e.kpp[k][p1][p2][i] * ratio2;
	}

	void Evaluater::CheckNonZero() const {
		for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1) {
			for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2) {
				if (kk[k1][k2][0] != 0 || kk[k1][k2][1] != 0) {
					k1 = BOARD_NB; k2 = BOARD_NB;
					cout << "KK has non zero value.\n";
				}
			}
		}

		for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1) {
			for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2) {
				for (BonaPiece p = BONA_PIECE_ZERO; p < BONA_PIECE_NB; ++p) {
					if (kkp[k1][k2][p][0] != 0 || kkp[k1][k2][p][1] != 0) {
						k1 = BOARD_NB; k2 = BOARD_NB; p = BONA_PIECE_NB;
						cout << "KKP has non zero value.\n";
					}
				}
			}
		}

		for (Square k = SQUARE_ZERO; k < BOARD_NB; ++k) {
			for (BonaPiece p1 = BONA_PIECE_ZERO; p1 < BONA_PIECE_NB; ++p1) {
				for (BonaPiece p2 = BONA_PIECE_ZERO; p2 < BONA_PIECE_NB; ++p2) {
					if (kpp[k][p1][p2][0] != 0 || kpp[k][p1][p2][1] != 0) {
						k = BOARD_NB; p1 = BONA_PIECE_NB; p2 = BONA_PIECE_NB;
						cout << "KPP has non zero value.\n";
					}
				}
			}
		}
	}

	Value EvalSum::Sum(const Color c) const {
		if (meterial == VALUE_NULL || pin == VALUE_NULL)
			return VALUE_NULL;
		// [0](先手KPP) + [1](後手KPP) + [2](KK+KKP) 
		const Value scoreBoard = (Value)(pos[0][0] - pos[1][0] + pos[2][0]) / FV_SCALE + meterial + pin;
		const Value scoreTurn = (Value)(pos[0][1] + pos[1][1] + pos[2][1]) / FV_SCALE;

		return (c == WHITE ? scoreBoard : -scoreBoard) + scoreTurn;
	}

	void EvalSum::Clean() {
		memset(this, 0, sizeof(EvalSum));
		meterial = VALUE_NULL;
		pin = VALUE_NULL;
	}
}