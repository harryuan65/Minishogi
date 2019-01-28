#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#define NOMINMAX

#include "Thread.h"
#include "Zobrist.h"
#include "Search.h"
#include "Observer.h"
#include "Minishogi.h"
#include "Evaluate.h"
#include "EvaluateLearn.h"
#include "usi.h"

using namespace std;
using namespace Evaluate;
namespace fs = std::experimental::filesystem;

Move algebraic2move(string str, Minishogi &pos) {
	Piece srcPiece = NO_PIECE;
	int i = 0, srcFile = -1, srcRank = -1, dstFile = -1, dstRank = -1;
	bool isDrop = false, isPromote = false, isCapture = false;

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
		isCapture = true;
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
	ExtMove moveList[SINGLE_GENE_MAX_MOVES], *end;
	if (isCapture)
		end = pos.AttackGenerator(moveList);
	else if (isDrop)
		end = pos.HandGenerator(moveList);
	else
		end = pos.MoveGenerator(moveList);
	for (auto *start = moveList; start < end; start++) {
		if (srcPiece != NO_PIECE && pos.GetPiece(*start) != srcPiece)
			continue;
		if (srcFile != -1 && from_sq(*start) % 5 != srcFile)
			continue;
		if (srcRank != -1 && from_sq(*start) / 5 != srcRank)
			continue;
		if (dstFile != -1 && to_sq(*start) % 5 != dstFile)
			continue;
		if (dstRank != -1 && to_sq(*start) / 5 != dstRank)
			continue;
		/*if (isDrop != is_drop(from_sq(*start)))
			continue;
		if (isPromote != is_promote(*start))
			continue;*/
		if (pos.IsInCheckedAfter(*start))
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
	return MOVE_NULL;
}

namespace EvaluateLearn {
	double lambda = LAMBDA;
	double gamma = GAMMA;
	LearnFloatType Weight::eta = WEIGHT_ETA;
	int Weight::skip_count = 10;

	Weight(*kk_w)[BOARD_NB][BOARD_NB];
	Weight(*kkp_w)[BOARD_NB][BOARD_NB][BONA_PIECE_NB];
	Weight(*kpp_w)[BOARD_NB][BONA_PIECE_NB][BONA_PIECE_NB];

#define KK  (GlobalEvaluater.kk)
#define KKP (GlobalEvaluater.kkp)
#define KPP (GlobalEvaluater.kpp)
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
		double o = lambda * (w - p) * progress;
		return q - (p + o);
	}

	void AddGrad(const Minishogi &m, Turn rootTurn, double delta_grad) {
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
				Turn winner;
				moves[0] = MOVE_NULL;
				values[0] = VALUE_NONE;

				// Playing
				pthread = new Thread(rootPos, USI::Options["Hash"]);
				cout << "Cycle " << cycle << " Game " << b << " ";
				while (ply < MAX_PLY - 50) {
					RootMove rm;
					pthread->IDAS(rm, USI::Options["Hash"]);
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
				double progress = pow(gamma, ply); RootMove quietRM;
				for (int i = 0; i < ply; pthread->DoMove(moves[i++]), progress /= gamma) {
					Turn rootTurn = pos.GetTurn();
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
							Evaluate::GlobalEvaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp());
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
		cout << "AI Version : " << AI_NAME << "\n" << Observer::GetSettingStr() << endl;
		SetConsoleTitle("Nyanpass " AI_NAME " - EvaluateLearn");

		cout << "輸入訓練Cycle次數" << endl;
		cin >> cycleNum;

		// Resize
		CreateDirectory(KPPT_DIRPATH.c_str(), NULL);
		EvaluateLearn::InitGrad();
		Zobrist::Resize();
		cout << Observer::GetTimeStamp() << " Set Depth " << USI::Options["Depth"] << ",Cycle " << cycleNum
			<< ",LEARN_PATCH_SIZE " << LEARN_PATCH_SIZE << ",EVAL_LIMIT " << EVAL_LIMIT << ",lambda " << lambda
			<< ",gamma " << gamma << ",eta " << Weight::eta << ",skip_count " << Weight::skip_count << "\n";
		cout << Observer::GetTimeStamp() << " Learning StartGameLoop.\n";

		Learning();

		if (Evaluate::GlobalEvaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp() + "_Finish"))
			cout << Observer::GetTimeStamp() << " Save KKPT to " << KPPT_DIRPATH << Observer::GetTimeStamp() + "_Finish" << " failed.\n";
		else
			cout << Observer::GetTimeStamp() << " Save KKPT to " << KPPT_DIRPATH << Observer::GetTimeStamp() + "_Finish" << " success.\n";
		cout << Observer::GetTimeStamp() << " Learning End.\n";

		SetConsoleTitle("Nyanpass " AI_NAME " - EvaluateLearn : Stop");
	}
	*/

	struct KifuLearn : public Thread {
	public:
		const Value SHOKIDOKO_SKIP_MIN = Value(399 * PIECE_SCORE[PAWN]);
		//const Value SHOKIDOKO_SKIP_MAX = Value(400 * PIECE_SCORE[PAWN]);

		vector<string> teacher;
		string kifu_path = KIFULEARN_KIFU_PATH;
		int eval_limit   = KIFULEARN_EVAL_LIMIT;
		int update_patch = 1000000;
		int save_patch   = 100000000;

		KifuLearn() : Thread() {}

		virtual void Run() {
			vector<pair<Move, Value>> kifu;
			fs::create_directory(KPPT_DIRPATH);

			// Load Kifu Data
			for (auto &p : fs::directory_iterator(kifu_path)) {
				if (CheckStop())
					break;
				if (fs::is_directory(p))
					continue;

				ifstream ifile(fs::absolute(p));
				string line, token;
				Turn winner;
				bool isTeacher[2] = { false };
				char c;
				float f;

				cout << "Load Kifu from " << fs::absolute(p) << (ifile ? " Success" : " Failed") << endl;
				if (!ifile)
					continue;

				while (getline(ifile, line)) {
					istringstream iss(line);
					iss >> token;
					if (token == "[White") {
						getline(iss, token, '\"');
						getline(iss, token, '\"');
						isTeacher[0] = find(teacher.begin(), teacher.end(), token) != teacher.end();
					}
					else if (token == "[Black") {
						getline(iss, token, '\"');
						getline(iss, token, '\"');
						isTeacher[1] = find(teacher.begin(), teacher.end(), token) != teacher.end();
					}
					else if (token == "[Result") {
						iss >> token;
						winner = Turn(token[1] == '0');
					}
					/*else if (token == "[Round") {
						iss >> token;
						cout << token << endl;
					}*/
					/*else if (token == "[FEN") {
						getline(iss, token);
						token.erase(0, 2);
						kifus.back().sfen = fen2sfen(token);
					}*/
					else if (token[0] == '{' || token == "[Annotator") {
						if (!isTeacher[0] && !isTeacher[1])
							break;

						pos.Initialize();
						if (token[0] == '{')
							for (int j = 0; j < 7; j++)
								getline(ifile, line);
						while (true) {
							Value v = VALUE_NONE;
							if (pos.GetTurn() == WHITE && token != "1.") {
								ifile >> token;             // 1.
								if (token[0] == '{')
									break;
							}
							ifile >> token;                 // Gc2
							if (token[0] == '{')
								break;

							kifu.push_back(make_pair(algebraic2move(token, pos), VALUE_NONE));
							ifile >> noskipws >> c >> skipws;
							if (ifile.peek() == '{') {
								ifile >> c;                 // {
								getline(ifile, token, '/'); // +0.68/
								istringstream ss(token);
								ss >> f;
								v = (Value)int(f * (int)VALUE_PAWN);
								getline(ifile, token, '}'); // 18 5:51}

								if (!isTeacher[pos.GetTurn()] ||
									(v >= eval_limit && winner == pos.GetTurn()) ||
									(v <= -eval_limit && winner != pos.GetTurn()) ||
									// Shokidoko條款 排除0 399+ -399-
									v == VALUE_ZERO ||
									SHOKIDOKO_SKIP_MIN <= abs(v))
									v = VALUE_NONE;
								else
									kifu.back().second = v;
							}
							else {
								// 排除mini-opening上的evaluate
								for (int i = kifu.size() - 1; i >= 0 && kifu[i].first != MOVE_NONE; i--)
									kifu[i].second = VALUE_NONE;
							}
							pos.DoMove(kifu.back().first);
						}
						for (int i = kifu.size() - 1; i >= 0; i--) {
							if (kifu[i].second != VALUE_NONE)
								break;
							if (kifu[i].first != MOVE_NONE)
								for (int j = kifu.size() - 1; j >= i; j--)
									kifu.pop_back();	
						}
						kifu.push_back(make_pair(MOVE_NONE, VALUE_NONE));
					}
				}
			}

			int boardCount = 0;
			for (auto &k : kifu)
				boardCount += k.second != VALUE_NONE;
			cout << "Train Board Count : " << boardCount << endl;

			// Learning
			uint64_t updateGradCount = 0, epoch = 0;
			double sumError = 0.0, minSumError = 1.0;
			int convFailCount = 0;
			maxCheckPly = 1024;

			cout << "Learn Start." << endl;
			pos.Initialize();
			TimePoint t = now();
			while (boardCount && !IsStop()) {
				for (auto &k : kifu) {
					if (CheckStop())
						break;

					if (k.first == MOVE_NONE && k.second == VALUE_NONE) {
						pos.Initialize();
						continue;
					}
					assert(k.first != MOVE_NONE);
					if (k.second == VALUE_NONE) {
						pos.DoMove(k.first);
						continue;
					}

					RootMove quietRM;
					Turn rootTurn = pos.GetTurn();

					InitSearch();
					Search(quietRM, 0);

					double dj_dw = CalcGrad(k.second, quietRM.value);
					int j;

					if (dj_dw == 0.0) {
						pos.DoMove(k.first);
						continue;
					}
					sumError += dj_dw * dj_dw;

					for (j = 0; quietRM.pv[j] != MOVE_NULL; j++)
						pos.DoMove(quietRM.pv[j]);
					AddGrad(pos, rootTurn, dj_dw);
					for (j--; j >= 0; j--)
						pos.UndoMove();

					pos.DoMove(k.first);

					if (++updateGradCount % update_patch == 0) {
						UpdateKPPT(++epoch);
						cout << "epoch : " << epoch << " mse : " << sqrt(sumError / update_patch);
						if (sqrt(sumError / update_patch) > minSumError && epoch > Weight::skip_count) {
							if (++convFailCount > 8)
								Stop();
						}
						else {
							minSumError = sqrt(sumError / update_patch);
							convFailCount = 0;
						}
						sumError = 0.0;
						if (updateGradCount % save_patch == 0)
							Evaluate::GlobalEvaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp());
						cout << " elpased : " << now() - t << endl;
						t = now();
					}
				}
			}
			if (epoch)
				Evaluate::GlobalEvaluater.Save(KPPT_DIRPATH + Observer::GetTimeStamp());
			cout << "Update Grad Count : " << updateGradCount << endl;
			cout << "Learn end" << endl;
			maxCheckPly = USI::Options["MaxCheckPly"];
		}
	};

	void StartKifuLearn(istringstream& ss_cmd) {
		KifuLearn *th = new KifuLearn();
		string token;

		while (ss_cmd >> token) {
			if (token == "kifu_path") {
				getline(ss_cmd, token, '\"');
				getline(ss_cmd, th->kifu_path, '\"');
				cout << th->kifu_path << endl;
			}
			else if (token == "lambda")            ss_cmd >> lambda;
			else if (token == "gamma")             ss_cmd >> gamma;
			else if (token == "eta")               ss_cmd >> Weight::eta;
			else if (token == "eval_limit")        ss_cmd >> th->eval_limit;
			else if (token == "teacher") { // 必須在指令的最後面
				getline(ss_cmd, token, '\"');
				do {
					getline(ss_cmd, token, '\"');
					th->teacher.push_back(token);
					getline(ss_cmd, token, '\"');
				} while (token.size() && !ss_cmd.eof());
			}
		}
		
		delete GlobalThread;
		EvaluateLearn::InitGrad();
		GlobalThread = th;
		GlobalTT.Resize(1);
		cout << "Kifu Learn Setting : eta " << Weight::eta 
			<< " lambda " << lambda
			<< " eval_limit " << th->eval_limit << endl;
		GlobalThread->StartWorking();
	}
}