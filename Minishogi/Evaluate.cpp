#include <iostream>
#include <fstream>
#include <filesystem>
#define NOMINMAX

#include "Evaluate.h"
#include "Thread.h"
#include "Observer.h"

using namespace std;
namespace fs = std::experimental::filesystem;

namespace Evaluate {
	Evaluater GlobalEvaluater;

	bool Evaluater::Load(string path) {
		ifstream ifKK (path + "/" + KK_FILENAME, ios::binary);
		ifstream ifKKP(path + "/" + KKP_FILENAME, ios::binary);
		ifstream ifKPP(path + "/" + KPP_FILENAME, ios::binary);
		if (!ifKK || !ifKKP || !ifKPP) {
			cout << "Error : Evaluater load from \"" << path << "\" Failed" << endl;
			Clean();
			return false;
		}

		ifKK.read(reinterpret_cast<char*>(kk), sizeof(kk));
		ifKKP.read(reinterpret_cast<char*>(kkp), sizeof(kkp));
		ifKPP.read(reinterpret_cast<char*>(kpp), sizeof(kpp));
					
		cout << "Evaluater load from \"" << path << "\" Successed" << endl;
		return true;
	}

	bool Evaluater::Save(string path) {
		fs::create_directory(path);

		ofstream ofKK(path + "/" + KK_FILENAME, ios::binary);
		ofstream ofKKP(path + "/" + KKP_FILENAME, ios::binary);
		ofstream ofKPP(path + "/" + KPP_FILENAME, ios::binary);

		if (!ofKK.write(reinterpret_cast<char*>(kk), sizeof(kk)) ||
			!ofKKP.write(reinterpret_cast<char*>(kkp), sizeof(kkp)) ||
			!ofKPP.write(reinterpret_cast<char*>(kpp), sizeof(kpp))) {
			cout << "Error : Evaluater save to \"" << path << "\" Failed" << endl;
			return false;
		}
		else {
			cout << "Evaluater save to \"" << path << "\" Successed" << endl;
			return true;
		}
	}

	void Evaluater::Clean() {
		memset(this, 0, sizeof(Evaluater));
		cout << "Evaluater Clean" << endl;
	}

	void Evaluater::Blend(Evaluater &e, float ratio) {
		float ratio2 = 1 - ratio;
		for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1)
			for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2)
				for (int i = 0; i < 2; i++)
					kk[k1][k2][i] = (float)kk[k1][k2][i] * ratio + (float)e.kk[k1][k2][i] * ratio2;

		for (Square k1 = SQUARE_ZERO; k1 < BOARD_NB; ++k1)
			for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2)
				for (BonaPiece p = BONA_PIECE_ZERO; p < BONA_PIECE_NB; ++p)
					for (int i = 0; i < 2; i++)
						kkp[k1][k2][p][i] = (float)kkp[k1][k2][p][i] * ratio + (float)e.kkp[k1][k2][p][i] * ratio2;

		for (Square k = SQUARE_ZERO; k < BOARD_NB; ++k)
			for (BonaPiece p1 = BONA_PIECE_ZERO; p1 < BONA_PIECE_NB; ++p1)
				for (BonaPiece p2 = BONA_PIECE_ZERO; p2 < BONA_PIECE_NB; ++p2)
					for (int i = 0; i < 2; i++)
						kpp[k][p1][p2][i] = (float)kpp[k][p1][p2][i] * ratio + (float)e.kpp[k][p1][p2][i] * ratio2;
	}

	Value EvalSum::Sum(const Turn c) const {
		if (IsNotCalc())
			return VALUE_NONE;
		// [0](先手KPP) + [1](後手KPP) + [2](KK+KKP) 
#ifdef PIN_DISABLE
		const Value scoreBoard = (Value)(pos[0][0] - pos[1][0] + pos[2][0]) / FV_SCALE + material;
#else
		const Value scoreBoard = (Value)(pos[0][0] - pos[1][0] + pos[2][0]) / FV_SCALE + material + pin;
#endif
		const Value scoreTurn = (Value)(pos[0][1] + pos[1][1] + pos[2][1]) / FV_SCALE;

		return (c == WHITE ? scoreBoard : -scoreBoard) + scoreTurn;
	}

	void EvalSum::Clean() {
		memset(this, 0, sizeof(EvalSum));
		material = VALUE_NONE;
		pin = VALUE_NONE;
	}
}