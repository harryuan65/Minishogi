#include <algorithm>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#define NOMINMAX
#include <windows.h>

#include "Thread.h"
#include "Zobrist.h"
#include "Search.h"
#include "Observer.h"
#include "Minishogi.h"
#include "Evaluate.h"
#include "EvaluateLearn.h"
using namespace std;
using namespace Evaluate;
namespace fs = std::tr2::sys;

Move algebraic2move(string str, Minishogi &pos) {
	Piece srcPiece = NO_PIECE;
	int i = 0, srcFile = -1, srcRank = -1, dstFile = -1, dstRank = -1;
	bool isDrop = false, isPromote = false;

	// mover pro (+)
	if (str[i] == '+') {
		srcPiece = PROMOTE;
		i++;
	}
	// mover type (PSGBRK)
	if (PIECE_2_CHAR.find(str[i]) < BLACKCHESS) {
		srcPiece = Piece((pos.GetTurn() << 4) | srcPiece | PIECE_2_CHAR.find(str[i]));
		i++;
	}
	// mover file (abcde)
	if ('a' <= str[i] && str[i] <= 'e') {
		srcFile = str[i] - 'a';
		i++;
	}
	// mover rank (12345)
	if ('1' <= str[i] && str[i] <= '5') {
		srcRank = '5' - str[i];
		i++;
	}
	// atk (x)
	if (str[i] == 'x') {
		i++;
	}
	// drop (@)
	if (str[i] == '@') {
		isDrop = true;
		i++;
	}
	// dst file (abcde)
	if ('a' <= str[i] && str[i] <= 'e') {
		dstFile = str[i] - 'a';
		i++;
	}
	// dst rank (12345) 
	if ('1' <= str[i] && str[i] <= '5') {
		dstRank = '5' - str[i];
		i++;
	}
	// pro (+#=)
	if (str[i] == '+') {
		isPromote = true;
	}
	// no dst info
	if (dstFile == -1 && dstRank == -1) {
		swap(srcFile, dstFile);
		swap(srcRank, dstRank);
	}
	// no src info
	if (srcPiece == NO_PIECE && srcFile == -1 && srcRank == -1) {
		srcPiece = Piece((pos.GetTurn() << 4) | PAWN);
	}

	// Find Suitable Move
	Move moveList[TOTAL_GENE_MAX_ACTIONS], *end = pos.GetLegalMoves(moveList);
	for (Move *start = moveList; start < end; start++) {
		if (srcPiece != NO_PIECE && pos.GetChessOn(from_sq(*start)) != srcPiece)
			continue;
		if (srcFile != -1 && from_sq(*start) % 5 != srcFile)
			continue;
		if (srcRank != -1 && from_sq(*start) / 5 != srcRank)
			continue;
		if (dstFile != -1 && to_sq(*start) % 5 != dstFile)
			continue;
		if (dstRank != -1 && to_sq(*start) / 5 != dstRank)
			continue;
		if (isDrop != is_drop(from_sq(*start)))
			continue;
		if (isPromote != is_promote(*start))
			continue;
		return *start;
	}
	
	cout << "Error : Illegal Move. Move Detail : " << endl;
	cout << "Move String " << str << " srcPiece " << srcPiece << endl;
	cout << "srcFile " << srcFile << " srcRank " << srcRank << endl;
	cout << "dstFile " << dstFile << " dstRank " << dstRank << endl;
	cout << "isDrop " << isDrop << " isPromote " << isPromote << endl;
	cout << pos << endl;
	system("pause");
	assert(false);
	return MOVE_NULL;
}

string fen2sfen(string fen) {
	string board, hand, turn;
	istringstream iss(fen);
	getline(iss, board, '[');
	getline(iss, hand, ']');
	iss >> turn;
	return board + " " + (turn == "w" ? "b" : "w") + " " + hand + " 1";
}

namespace EvaluateLearn {
	double LAMBDA = 0.5;
	double GAMMA = 0.93;
	LearnFloatType Weight::eta = 64.0f;
	int Weight::skip_count = 10;
	Thread *th;

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
    X[0] = min(X[0],(MAX));   \
    X[0] = max(X[0],(MIN));   \
    X[1] = min(X[1],(MAX));   \
    X[1] = max(X[1],(MIN));

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

		return min({ q1, q2, q3, q4 });
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

	double CalcGrad(Value searchValue, Value quietValue) {
		if (searchValue == VALUE_NONE || quietValue == VALUE_NONE)
			return 0.0;
		return winest(quietValue) - winest(searchValue);
	}

	double CalcGrad(Value searchValue, Value quietValue, bool winner, double progress) {
		if (searchValue == VALUE_NONE || quietValue == VALUE_NONE)
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
						KK[k1][k2] = { (int32_t)round(w.weight[0]), (int32_t)round(w.weight[1]) };
					}
				}
			}

			for (Square k1 = Square(5 * id); k1 < (Square)(5 * (id + 1)); ++k1) {
				for (Square k2 = SQUARE_ZERO; k2 < BOARD_NB; ++k2) {
					for (BonaPiece p = BONA_PIECE_ZERO; p < BONA_PIECE_NB; ++p) {
						Weight& w = KKPW[k1][k2][p];
						if (w.update(skip_update)) {
							SET_A_LIMIT_TO(w.weight, (LearnFloatType)(INT16_MIN / 2), (LearnFloatType)(INT16_MAX / 2));
							KKP[k1][k2][p] = ValueKkp{ (int32_t)round(w.weight[0]), (int32_t)round(w.weight[1]) };
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
							WriteKPP(k, p1, p2, ValueKpp{ (int16_t)round(w.weight[0]), (int16_t)round(w.weight[1]) });
						}
					}
				}
			}
		};

		thread th[5];

		for (int i = 0; i < 5; i++)
			th[i] = thread(func, i);

		for (int i = 0; i < 5; i++)
			th[i].join();
	}

	/*
	void Learning() {
		Thread *pthread = nullptr;
		Minishogi rootPos(nullptr);
		Move moves[MAX_PLY];
		Value values[MAX_PLY];

		uint64_t updateGradCount = 0, epoch = 0;
		double sumError = 0.0;
		streamoff readBoardOffset = 0;

		for (int cycle = 0; cycle < cycleNum; cycle++, readBoardOffset = 0) {
			for (int b = 0; rootPos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset); b++) {
				int ply = 0;
				Color winner;
				moves[0] = MOVE_NULL;
				values[0] = VALUE_NONE;

				// Playing
				pthread = new Thread(rootPos, USI::Options["HashEntry"]);
				cout << "Cycle " << cycle << " Game " << b << " ";
				while (ply < MAX_PLY - 50) {
					RootMove rm;
					pthread->IDAS(rm, USI::Options["HashEntry"]);
					if (rm.pv[0] == MOVE_NULL)
						break;
					pthread->DoMove(rm.pv[0]);
					moves[ply] = rm.pv[0];
					values[ply] = rm.value;
					ply++;
					cout << ".";
				}
				cout << endl;
				assert(pthread->GetMinishogi().CheckLegal());
				winner = ~pthread->GetMinishogi().GetTurn();
				delete pthread;

				if (ply >= MAX_PLY - 50)
					continue;

				// Learning
				pthread = new Thread(rootPos, 1);
				const Minishogi &pos = pthread->GetMinishogi();
				double progress = pow(GAMMA, ply); RootMove quietRM;
				for (int i = 0; i < ply; pthread->DoMove(moves[i++]), progress /= GAMMA) {
					Color rootTurn = pos.GetTurn();
					bool isWin = winner == rootTurn;
					Value searchValue = values[i];
					int j;

					if ((searchValue >= EVAL_LIMIT && isWin) || (searchValue <= -EVAL_LIMIT && !isWin))
						continue;

					pthread->Search(quietRM, 0);

					double dj_dw = CalcGrad(searchValue, quietRM.value, isWin, progress);
					//double dj_dw = CalcGrad(searchValue, quietRM.value, isWin, double(i + 1) / ply);
					//cout << isWin << " " << searchValue << " " << quietRM.value << " " << progress << " " << dj_dw << "\n";

					if (dj_dw == 0.0)
						continue;
					sumError += dj_dw * dj_dw;

					for (j = 0; quietRM.pv[j] != MOVE_NULL; j++)
						pthread->DoMove(quietRM.pv[j]);

					AddGrad(pos, rootTurn, dj_dw);

					for (j--; j >= 0; j--)
						pthread->UndoMove();

					if (++updateGradCount % LEARN_PATCH_SIZE == 0) {
						UpdateKPPT(++epoch);
						cout << "epoch : " << epoch << " mse : " << sqrt(sumError / LEARN_PATCH_SIZE) << endl;
						log << Observer::GetTimeStamp() << " epoch : " << epoch << " mse : " << sqrt(sumError / LEARN_PATCH_SIZE) << "\n";
						sumError = 0.0;
						if (updateGradCount % SAVE_PATCH_SIZE == 0)
							Evaluate::evaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp());
						DumpLog(logPathStr);
					}
				}
			}
			log << Observer::GetTimeStamp() << " Cycle " << cycle << " finished. Add Grad Count " << updateGradCount << "\n";
			DumpLog(logPathStr);
		}
	}

	void SelfLearn() {
		int cycleNum;
		cout << "AI Version : " << AI_VERSION << "\n" << Observer::GetSettingStr() << endl;
		SetConsoleTitle("Nyanpass " AI_VERSION " - EvaluateLearn");

		cout << "輸入訓練Cycle次數" << endl;
		cin >> cycleNum;

		// Initialize
		CreateDirectory(KPPT_DIRPATH.c_str(), NULL);
		EvaluateLearn::InitGrad();
		Zobrist::Initialize();
		cout << Observer::GetTimeStamp() << " Set Depth " << USI::Options["Depth"] << ",Cycle " << cycleNum
			<< ",LEARN_PATCH_SIZE " << LEARN_PATCH_SIZE << ",EVAL_LIMIT " << EVAL_LIMIT << ",LAMBDA " << LAMBDA
			<< ",GAMMA " << GAMMA << ",eta " << Weight::eta << ",skip_count " << Weight::skip_count << "\n";
		cout << Observer::GetTimeStamp() << " Learning StartGameLoop.\n";

		Learning();

		if (Evaluate::evaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp() + "_Finish"))
			cout << Observer::GetTimeStamp() << " Save KKPT to " << KPPT_DIRPATH << Observer::GetTimeStamp() + "_Finish" << " failed.\n";
		else
			cout << Observer::GetTimeStamp() << " Save KKPT to " << KPPT_DIRPATH << Observer::GetTimeStamp() + "_Finish" << " success.\n";
		cout << Observer::GetTimeStamp() << " Learning End.\n";

		SetConsoleTitle("Nyanpass " AI_VERSION " - EvaluateLearn : Stop");
	}
	*/
	
	struct KifuLearn : public Thread {
	public:
		string kifu_path = KIFULEARN_DIRPATH;
		Value eval_limit = (Value)3000;
		double mse_target = 0.0;
		int update_patch = 100000;
		int save_patch = 10000000;

		virtual void Run() {
			uint64_t updateGradCount = 0, epoch = 0;
			double sumError = 0.0;

			do {
				for (auto &p : fs::directory_iterator(KIFULEARN_DIRPATH)) {
					if (CheckStop())
						break;

					//cout << "Load Kifu at " << fs::path(p).relative_path().string() << endl;
					ifstream ifile(fs::path(p).relative_path());
					vector<LearnData> kifus;
					string line, token;
					Move m;
					char c;
					float f;

					// Load Kifu
					kifus.emplace_back();
					while (getline(ifile, line)) {
						istringstream iss(line);
						iss >> token;
						// Load Infos
						if (token == "[White") {
							iss >> token;
							kifus.back().isTeacher[0] = token != "\"Nyanpass";
						}
						else if (token == "[Black") {
							iss >> token;
							kifus.back().isTeacher[1] = token != "\"Nyanpass";
						}
						else if (token == "[Result") {
							iss >> token;
							kifus.back().winner = Color(token[1] == '0');
						}
						else if (token == "[FEN") {
							getline(iss, token);
							token.erase(0, 2);
							kifus.back().sfen = fen2sfen(token);
						}
						// Load Moves & Evals
						else if (token[0] == '{') {
							for (int j = 0; j < 7; j++)
								getline(ifile, line);
							while (true) {
								if (kifus.back().moves.size() % 2 == 0)
									ifile >> token;
								if (token[0] == '{')
									break;
								ifile >> token >> c;
								if (token[0] == '{')
									break;
								kifus.back().moves.push_back(token);
								getline(ifile, token, '/');
								istringstream ss(token);
								ss >> f;
								kifus.back().values.push_back(Value(int(f * PIECE_SCORE[PAWN])));
								getline(ifile, token, '}');
							}
							/*cout << kifus.back().sfen << " "
								<< kifus.back().isTeacher[0] << " "
								<< kifus.back().isTeacher[1] << " "
								<< kifus.back().winner << " "
								<< kifus.back().moves.size() << endl;*/
							kifus.emplace_back();
						}
					}
					kifus.pop_back();

					// Learning
					for (auto &ld : kifus) {
						if (CheckStop())
							break;

						pos.InitializeSFEN(ld.sfen);
						for (int ply = 0; ply < ld.moves.size(); ply++, pos.DoMove(m)) {
							RootMove quietRM;
							Color rootTurn = pos.GetTurn();
							bool isWin = ld.winner == rootTurn;
							int j;
							m = algebraic2move(ld.moves[ply], pos);
							//cout << pos << m << endl;

							if (!m)
								break;

							if (!ld.isTeacher[rootTurn] || (ld.values[ply] >= eval_limit && isWin) || (ld.values[ply] <= -eval_limit && !isWin))
								continue;

							InitSearch();
							Search(quietRM, 0);

							double dj_dw = CalcGrad(ld.values[ply], quietRM.value);

							if (dj_dw == 0.0)
								continue;
							sumError += dj_dw * dj_dw;

							for (j = 0; quietRM.pv[j] != MOVE_NULL; j++)
								pos.DoMove(quietRM.pv[j]);

							AddGrad(pos, rootTurn, dj_dw);

							for (j--; j >= 0; j--)
								pos.UndoMove();

							if (++updateGradCount % update_patch == 0) {
								UpdateKPPT(++epoch);
								cout << "epoch : " << epoch << " mse : " << sqrt(sumError / update_patch) << endl;
								if (updateGradCount % save_patch == 0)
									Evaluate::evaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp());
								if (mse_target && sqrt(sumError / update_patch) < mse_target)
									Stop();
								sumError = 0.0;
							}
						}
					}
				}
			} while (mse_target && !CheckStop());
			Evaluate::evaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp());
			cout << "Update Grad Count : " << updateGradCount << endl;
			cout << "Learn end." << endl;
			
			isExit = true;
		}
	};

	void StartKifuLearn(istringstream& is) {
		KifuLearn *kl = new KifuLearn();

		while (true) {
			string option;
			is >> option;

			if (option == "")
				break;
			else if (option == "kifu_path")
				is >> kl->kifu_path;
			else if (option == "eta")
				is >> Weight::eta;
			else if (option == "lambda")
				is >> LAMBDA;
			else if (option == "eval_limit")
				is >> kl->eval_limit;
			else if (option == "mse_target")
				is >> kl->mse_target;
			else if (option == "update_patch")
				is >> kl->update_patch;
			else if (option == "save_patch")
				is >> kl->save_patch;
		}
		
		if (th)
			delete th;
		EvaluateLearn::InitGrad();
		USI::Limits.ponder = false;
		th = kl;
		th->StartWorking();
	}

	void Stop() {
		if (th)
			th->Stop();
	}
}