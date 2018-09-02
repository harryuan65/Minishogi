#include <algorithm>
#include <math.h>
#define NOMINMAX
#include <windows.h>
#include <fstream>

#include "Thread.h"
#include "Minishogi.h"
#include "Search.h"
#include "Observer.h"
#include "Transposition.h"
#include "EvaluateLearn.h"
using namespace std;
using namespace Evaluate;
using namespace EvaluateLearn;

int main() {
	const int SAVE_PATCH_SIZE = 10000;
	Thread *pthread = nullptr;
	Minishogi rootPos;
	Move moves[MAX_HISTORY_PLY];
	Value values[MAX_HISTORY_PLY];

	uint64_t addGradCount = 0, epoch = 0;
	double sumError = 0.0;
	int cycleNum;
	streamoff readBoardOffset = 0;
	string logPathStr = KPPT_DIRPATH + Observer::GetTimeStamp() + "_log.txt";

	// Learn Setting
	SetConsoleTitle("Minishogi (EvaluateLearn) - " AI_VERSION);
	cout << "AI Version : " << AI_VERSION << "\n" << Search::GetSettingStr() << endl;

	cout << "輸入搜尋的深度" << endl;
	cin >> Observer::depth;

	cout << "輸入KPP名稱" << endl;
	cin >> Observer::kpptName;
	if (!evaluater.Load(Observer::kpptName)) {
		Observer::kpptName = "";
		evaluater.Clean();
	}
	cout << "輸入訓練Cycle次數" << endl;
	cin >> cycleNum;

	// Initialize
	CreateDirectory(KPPT_DIRPATH, NULL);
	EvaluateLearn::InitGrad();
	Zobrist::Initialize();
	Transposition::Initialize();
	Observer::LearnLog << Observer::GetTimeStamp() << " Set Depth " << Observer::depth << ",Cycle " << cycleNum
		<< ",LEARN_PATCH_SIZE " << LEARN_PATCH_SIZE << ",EVAL_LIMIT " << EVAL_LIMIT << ",LAMBDA " << LAMBDA
		<< ",GAMMA " << GAMMA << ",eta " << Weight::eta << ",skip_count " << Weight::skip_count << "\n";
	Observer::LearnLog << Observer::GetTimeStamp() << " Learning Start.\n";
	Observer::LearnLogDump(logPathStr);

	for (int cycle = 0; cycle < cycleNum; cycle++, readBoardOffset = 0) {
		for (int b = 0; rootPos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset); b++) {
			// Clean
			int ply = 0;
			Color winner;
			Transposition::Clean();
			moves[0] = MOVE_NULL;
			values[0] = VALUE_NULL;

			// Playing
			pthread = new Thread(rootPos, WHITE);
			cout << "Cycle " << cycle << " Game " << b << " ";
			while (ply < MAX_HISTORY_PLY - 50) {
				RootMove rm;
				pthread->IDAS(rm, Observer::depth, true);
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

			if (ply >= MAX_HISTORY_PLY - 50)
				continue;
			
			// Learning
			/*for (int aaa = 0; aaa < 10000; aaa++) {
				pthread = new Thread(rootPos, WHITE);
				const Minishogi &pos = pthread->GetMinishogi();
				double progress = pow(GAMMA, ply);
				for (int i = 0; i < ply; pthread->DoMove(moves[i++]), progress /= GAMMA) {
					RootMove quietRM;
					Color rootTurn = pos.GetTurn();
					bool isWin = winner == rootTurn;
					Value searchValue = values[i];
					int j;

					if ((searchValue >= EVAL_LIMIT && isWin) || (searchValue <= -EVAL_LIMIT && !isWin))
						continue;

					pthread->Search(quietRM, 0);
					double dj_dw = CalcGrad(searchValue, quietRM.value, isWin, progress);

					if (dj_dw == 0.0)
						continue;
					sumError += dj_dw * dj_dw;

					for (j = 0; quietRM.pv[j] != MOVE_NULL; j++)
						pthread->DoMove(quietRM.pv[j]);

					AddGrad(pos, rootTurn, dj_dw);

					for (j--; j >= 0; j--)
						pthread->UndoMove();
				}
				UpdateKPPT(++epoch);
				cout << "epoch : " << epoch << " mse : " << sqrt(sumError / ply) << endl;
				Observer::LearnLog << Observer::GetTimeStamp() << " epoch : " << epoch << " mse : " << sqrt(sumError / ply) << "\n";
				Observer::LearnLogDump(logPathStr);
				sumError = 0;
			}
			system("pause");*/
			pthread = new Thread(rootPos, WHITE);
			const Minishogi &pos = pthread->GetMinishogi();
			double progress = pow(GAMMA, ply);
			for (int i = 0; i < ply; pthread->DoMove(moves[i++]), progress /= GAMMA) {
				RootMove quietRM;
				Color rootTurn = pos.GetTurn();
				bool isWin = winner == rootTurn;
				Value searchValue = values[i];
				int j;

				if ((searchValue >= EVAL_LIMIT && isWin) || (searchValue <= -EVAL_LIMIT && !isWin))
					continue;

				pthread->Search(quietRM, 0);

				double dj_dw = CalcGrad(searchValue, quietRM.value, isWin, progress);
				//cout << isWin << " " << searchValue << " " << quietRM.value/* << " " << pthread->GetEvaluate()*/ << " " << progress << " " << dj_dw << "\n";


				if (dj_dw == 0.0)
					continue;
				sumError += dj_dw * dj_dw;

				for (j = 0; quietRM.pv[j] != MOVE_NULL; j++)
					pthread->DoMove(quietRM.pv[j]);

				AddGrad(pos, rootTurn, dj_dw);

				for (j--; j >= 0; j--)
					pthread->UndoMove();

				if (++addGradCount % LEARN_PATCH_SIZE == 0) {
					UpdateKPPT(++epoch);
					cout << "epoch : " << epoch << " mse : " << sqrt(sumError / LEARN_PATCH_SIZE) << endl;
					Observer::LearnLog << Observer::GetTimeStamp() << " epoch : " << epoch << " mse : " << sqrt(sumError / LEARN_PATCH_SIZE) << "\n";
					sumError = 0.0;
					if (addGradCount % SAVE_PATCH_SIZE == 0)
						Evaluate::evaluater.Save(Observer::GetTimeStamp());
					Observer::LearnLogDump(logPathStr);
				}
			}
		}
		Observer::LearnLog << Observer::GetTimeStamp() << " Cycle " << cycle << " finished. Add Grad Count " << addGradCount << "\n";
		Observer::LearnLogDump(logPathStr);
	}
	if (epoch)
		Evaluate::evaluater.Save(Observer::GetTimeStamp());
	Observer::LearnLog << Observer::GetTimeStamp() << " Learning End.\n";
	Observer::LearnLogDump(logPathStr);

	system("pause");
	return 0;
}