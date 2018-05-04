#define _CRT_SECURE_NO_WARNINGS
#include <atlstr.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include "Minishogi.h"
#include "Observer.h"
#include "AI.h"
#include "FileMapping.h"

#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"
#define AI_DISCRIPTION    "AI 老師的寧靜4層 最高8層"
using namespace std;

enum PlayerType {
	Human,
	AI,
	OtherAI
};

bool Human_DoMove(Minishogi &board, Action &action);
int AI_DoMove(Minishogi &board, Action &action);

string GetCurrentTimeString();
string GetAIVersion();
bool GetOpenFileNameString(string& out);

int main(int argc, char **argv) {
	Minishogi minishogi;
	Action action;

	int playerType[2];
	string playerName[2];
	int gameMode;
	bool isCustomBoard = false;
	bool isSwap = false;

	FileMapping fileMapping("DoMove");
	fstream file;
	streamoff readBoardOffset = 0;
	string aiVersion;
	string gameDirStr;
	string currTimeStr;
	string playDetailStr;

	/*    遊戲設定     */
	aiVersion = GetAIVersion();
	cout << "AI Version : " << aiVersion << endl;
	if (argc == 6) {
		// argv[6] = 遊戲路徑 輸出檔名 玩家名 深度 是否自訂棋盤 是否輸出
		gameMode = 5;
		currTimeStr = argv[1];
		playerType[0] = PlayerType::OtherAI;
		playerType[1] = PlayerType::AI;
		playerName[0] = argv[2];
		playerName[1] = aiVersion;
		Observer::DEPTH = atoi(argv[3]);
		isCustomBoard = argv[4][0] != '0';
		Observer::isSaveRecord = argv[5][0] != '0';
	}
	else {
		currTimeStr = GetCurrentTimeString();
		for (;;) {
			cout << "請選擇對手:\n"
				"(0)玩家vs電腦\n"
				"(1)電腦vs玩家\n"
				"(2)玩家對打\n"
				"(3)電腦對打\n"
				"(4)電腦對打 本機vs其他程式\n";
			cin >> gameMode;
			switch (gameMode)
			{
			case 0:
				playerType[0] = PlayerType::Human;
				playerType[1] = PlayerType::AI;
				playerName[0] = "Human";
				playerName[1] = aiVersion;
				break;
			case 1:
				playerType[0] = PlayerType::AI;
				playerType[1] = PlayerType::Human;
				playerName[0] = aiVersion;
				playerName[1] = "Human";
				break;
			case 2:
				playerType[0] = PlayerType::Human;
				playerType[1] = PlayerType::Human;
				playerName[0] = "Human";
				playerName[1] = "Human";
				break;
			case 3:
				playerType[0] = PlayerType::AI;
				playerType[1] = PlayerType::AI;
				playerName[0] = aiVersion;
				playerName[1] = aiVersion;
				break;
			case 4:
				if (!GetOpenFileNameString(gameDirStr)) {
					continue;
				}
				playerType[0] = PlayerType::AI; 
				playerType[1] = PlayerType::OtherAI;
				playerName[0] = aiVersion;
				playerName[1] = "";
				break;
			default:
				continue;
			}
			break;
		}

		if (playerType[0] == PlayerType::AI || playerType[1] == PlayerType::AI) {
			cout << "輸入搜尋的深度\n";
			cin >> Observer::DEPTH;
		}
		cin.ignore();
		cout << "從board//" << CUSTOM_BOARD_FILE << "讀取多個盤面 並連續對打?\n";
		isCustomBoard = getchar() != '0';
		cin.ignore();
		cout << "結束時匯出紀錄?\n";
		Observer::isSaveRecord = getchar() != '0';

		cout << "確定要開始? ";
		system("pause");
		if (gameMode == 4) {
			cout << "等待對方程式回應...\n";
			// argv[6] = 遊戲路徑 輸出檔名 玩家名 深度 是否自訂棋盤 是否輸出
			system(("start \"\" \"" + gameDirStr + "\" " +
				currTimeStr + " " +
				"\"" + aiVersion + "\" " +
				to_string(Observer::DEPTH) + " " +
				to_string(isCustomBoard) + " " +
				to_string(Observer::isSaveRecord)).c_str());
		}
	}
	CreateDirectory(CA2W(REPORT_PATH), NULL);

	/*    AI初始化    */
	cout << "---------- Game Initialize ----------\n";
	Zobrist::Initialize();
	if (playerType[0] == PlayerType::AI || playerType[1] == PlayerType::AI) {
		InitializeNS();
		InitializeTP();
	}
	do {
		/*    遊戲初始化    */
		minishogi.Initialize();
		Observer::depth = Observer::DEPTH;
		if (isCustomBoard && !minishogi.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
			if ((gameMode == 4 || gameMode == 5) && !isSwap) {
				// 先後手交換
				cout << "Swap player.\n";
				isSwap = true;
				swap(playerType[0], playerType[1]);
				swap(playerName[0], playerName[1]);
				readBoardOffset = 0;
				continue;
			}
			else { 
				// 對打結束
				break;
			}
		}
		cout << "---------- Game " << Observer::gameNum << " ----------\n";
		playDetailStr = REPORT_PATH + currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt";
		if (Observer::isSaveRecord && gameMode != 4) {
			file.open(playDetailStr, ios::app);
			if (file) {
				file << "#▼ : " << playerName[1] << "\n";
				file << "#△ : " << playerName[0] << "\n";
				file.close();
			}
			else cout << "Error : Fail to Save PlayDetail Title.\n";
		}
		CleanTP();

		/*    遊戲開始    */
		Observer::GameStart();
		while (true) {
			int eval = 0;
			cout << "---------- Game " << Observer::gameNum << " Step " << minishogi.GetStep() << " ----------\n";
			minishogi.PrintChessBoard();

			if (minishogi.IsGameOver()) {
				cout << (minishogi.GetTurn() ? "▼" : "△") << " Cannot Move.\n";
				action.mode = Action::SURRENDER;
				if (Observer::isSaveRecord && playerType[minishogi.GetTurn()] != PlayerType::OtherAI) {
					file.open(playDetailStr, ios::app);
					if (file) {
						file << "---------- Game " << Observer::gameNum << " Step " << minishogi.GetStep() << " ----------\n";
						minishogi.PrintNoncolorBoard(file);
						file << (minishogi.GetTurn() ? "▼" : "△") << " Cannot Move.\n";
						file.close();
					}
					else cout << "Error : Fail to Save PlayDetail.\n";
				}
				break;
			}
			switch (playerType[minishogi.GetTurn()]) {
			case Human:
				while (!Human_DoMove(minishogi, action));
				cout << "Action : " << action << "\n";
				break;
			case AI:
				Observer::StartSearching();
				eval = AI_DoMove(minishogi, action);
				Observer::EndSearching();

				cout << "Action : " << action << "\n";
				cout << "Leaf Eval : " << eval << "\n";
				//PrintPV(cout, minishogi, Observer::depth);
				Observer::PrintSearchReport(cout);
				if (playerType[!minishogi.GetTurn()] == OtherAI) {
					action.srcChess = 0;
					action.dstChess = 0;
					U32 actionU32 = action.ToU32();
					fileMapping.SendMsg(&actionU32, sizeof(U32));
				}
				break;
			case OtherAI:
				U32 actionU32;
				fileMapping.RecvMsg(&actionU32, sizeof(U32));
				action.SetU32(actionU32);
				cout << "Action : " << action << "\n";
				break;
			}

			if (Observer::isSaveRecord && playerType[minishogi.GetTurn()] != PlayerType::OtherAI) {
				file.open(playDetailStr, ios::app);
				if (file) {
					file << "---------- Game " << Observer::gameNum << " Step " << minishogi.GetStep() << " ----------\n";
					minishogi.PrintNoncolorBoard(file);
					file << "Action : " << action << "\n";
					file << "Leaf Eval : " << eval << "\n";
					//PrintPV(file, minishogi, Observer::depth);
					Observer::PrintSearchReport(file);
					file.close();
				}
				else cout << "Error : Fail to Save PlayDetail.\n";
			}
			if (eval < -CHECKMATE || CHECKMATE < eval) Observer::depth--;

			if (action.mode == Action::SURRENDER) {
				cout << (minishogi.GetTurn() ? "▼" : "△") << "投降! I'm lose\n";
				break;
			}
			else if (action.mode == Action::UNDO) {
				if (minishogi.GetStep() >= 2) {
					minishogi.UndoMove();
					minishogi.UndoMove();
					cout << "Succeed to Undo!\n";
				}
				else {
					cout << "Error : Cannot Undo!\n";
				}
			}
			else if (action.mode == Action::SAVEBOARD) {
				minishogi.SaveBoard(GetCurrentTimeString() + "_SaveBoard.txt");
			}
			else {
				minishogi.DoMove(action);
			}
		}
		/*    遊戲結束    */
		Observer::GameOver(!minishogi.GetTurn(), isSwap, minishogi.GetInitZobristHash(), minishogi.GetKifuHash());
		cout << "-------- Game Over! " << (!minishogi.GetTurn() ? "▼" : "△") << " Win! --------\n";
		Observer::PrintGameReport(cout);

		if (Observer::isSaveRecord) {
			if (gameMode == 5) {
				char msg[20];
				fileMapping.RecvMsg(msg, 20);
				if (strcmp("Save Report", msg)) {
					cout << "Error : 不同步啦\n";
					system("pause");
				}
			}
			// Save PlayDetail
			file.open(playDetailStr, ios::app);
			if (file) {
				file << "-------- Game Over! " << (!minishogi.GetTurn() ? "▼" : "△") << " Win! --------\n";
				file << "#" << "Player " << (gameMode == 5 ? "2 : " : "1 : ") << aiVersion << "\n";
				Observer::PrintGameReport(file);
				file.close();
			}
			else cout << "Error : Fail to Save PlayDetail.\n";
			// Save Kifu
			if (gameMode != 4) {
				file.open(REPORT_PATH + currTimeStr + "_Kifu_" + to_string(Observer::gameNum) + ".txt",
					ios::app);
				if (file) {
					file << "#▼ : " << playerName[1] << "\n";
					file << "#△ : " << playerName[0] << "\n";
					minishogi.PrintKifu(file);
					file.close();
				}
				else cout << "Error : Fail to Save Kifu Title.\n";
			}
			// Save AIReport
			if (gameMode != 5) {
				file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::out);
				if (file) {
					file << "Start at : " << currTimeStr << "\nEnd at   : " << GetCurrentTimeString() << "\n\n";
					Observer::PrintWinnerReport(file);
					file << "#" << "Player " << (gameMode == 5 ? "2 : " : "1 : ") << aiVersion << "\n";
					Observer::PrintTotalReport(file);
					file.close();
				}
				else cout << "Error : Fail to Save AI Report.\n";
			}
			else {
				file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::app);
				if (file) {
					file << "#" << "Player " << (gameMode == 5 ? "2 : " : "1 : ") << aiVersion << "\n";
					Observer::PrintTotalReport(file);
					file.close();
				}
				else cout << "Error : Fail to Save AI Report.\n";
			}
			if (gameMode == 4) {
				fileMapping.SendMsg("Save Report", 13);
			}
		}
	} while (isCustomBoard);
	cout << "---------- Total Report ----------\n";
	Observer::PrintWinnerReport(cout);
	Observer::PrintTotalReport(cout);

	cout << "\a\a\a";
    system("pause");
    return 0;
}


bool Human_DoMove(Minishogi &board, Action &action) {
	cout << "請輸入移動指令(E5D5+)或其他指令(UNDO, SURRENDER, SAVEBOARD) : " << endl;
	cin >> action;
	if (action.mode == Action::DO && !board.IsLegelAction(action)) {
		cout << "Error : Illegal action." << endl;
		return false;
	}
	else if (action.mode == Action::ILLEGAL) {
		cout << "Error : Unrecognized command." << endl;
		return false;
	}
	return true;
}

int AI_DoMove(Minishogi &board, Action &action) {
	cout << "AI 思考中..." << endl;
	return IDAS(board, action);
}

string GetCurrentTimeString() {
	char buffer[80];
	time_t rawtime;
	time(&rawtime);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", localtime(&rawtime));
	return string(buffer);
}

string GetAIVersion() {
	string str(AI_DISCRIPTION);
#ifdef BEST_ENDGAME_SEARCH
	str += " 不能投降";
#else
	str += " 可以投降";
#endif
#ifdef TRANSPOSITION_DISABLE
	str += " 無同型表";
	return str;
#endif
#ifdef DOUBLETP
	str += " 雙同型表";
#else
	str += " 國籍同構";
#endif
#ifdef QUIES_DISABLE
	str += " 沒寧靜";
#else
	str += " 有寧靜";
#endif
	return str;
}

bool GetOpenFileNameString(string& out) {
	char szFile[MAX_PATH];
	OPENFILENAME ofn;
	wchar_t currDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currDir);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetConsoleWindow();
	ofn.lpstrFile = (LPWSTR)szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"*.exe\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = L"選擇對手程式";
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = currDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	bool isOK = GetOpenFileName(&ofn);
	out.assign(CW2A(ofn.lpstrFile));
	return isOK;
}