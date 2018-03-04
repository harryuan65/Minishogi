#define _CRT_SECURE_NO_WARNINGS
#include <atlstr.h>
#include <fstream>
#include <iostream>
#include <string>

#include "Minishogi.h"
#include "AI.h"

#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"
#define AI_VERSION		  "TPSize=1<<30 國籍同構 include第六次修正後 actionList新增" //輸出報告註解用
using namespace std;

enum PlayerType {
	Human,
	AI,
	OtherAI
};

bool Human_DoMove(Minishogi &board, Action &action);
int AI_DoMove(Minishogi &board, Action &action);

string GetCurrentTimeString();
bool GetOpenFileNameString(string& out);
void SendMessageByHWND(HWND hwnd, string message);

int main(int argc, char **argv) {
	Minishogi minishogi;
	Action action;

	int playerType[2];
	string playerName[2];
	int gameMode;
	bool isCustomBoard = false;
	bool isSwap = false;
	HWND opponentHWND = 0;

	fstream file;
	streamoff readBoardOffset = 0;
	string currTimeStr;
	string playDetailStr;
	string kifuStr;

	/*    遊戲設定     */
	cout << "AI Version : " << AI_VERSION << endl;
	if (argc == 3) {
		gameMode = 5;
		opponentHWND = (HWND)atoi(argv[1]);
		currTimeStr = argv[2];
		SendMessageByHWND(opponentHWND, to_string((int)GetConsoleWindow()));
		playerType[0] = PlayerType::OtherAI;
		playerType[1] = PlayerType::AI;
		playerName[0] = "";
		playerName[1] = AI_VERSION;
	}
	else {
		string gameDirStr;
		currTimeStr = GetCurrentTimeString();
		for (;;) {
			cout << "請選擇對手:\n"
				"(0)玩家vs電腦\n"
				"(1)電腦vs玩家\n"
				"(2)玩家對打\n"
				"(3)電腦對打\n"
				"(4)電腦對打 本機vs其他程式\n";
			gameMode = getchar() - '0';
			cin.ignore();
			switch (gameMode)
			{
			case 0:
				playerType[0] = PlayerType::Human;
				playerType[1] = PlayerType::AI;
				playerName[0] = "Human";
				playerName[1] = AI_VERSION;
				break;
			case 1:
				playerType[0] = PlayerType::AI;
				playerType[1] = PlayerType::Human;
				playerName[0] = AI_VERSION;
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
				playerName[0] = AI_VERSION;
				playerName[1] = AI_VERSION;
				break;
			case 4:
				if (!GetOpenFileNameString(gameDirStr)) {
					continue;
				}
				cout << "等待程式開啟...\n";
				system(("start \"\" \"" + gameDirStr + "\" " + to_string((int)GetConsoleWindow()) + " " + currTimeStr).c_str());
				int bufHWND;
				cin >> bufHWND;
				opponentHWND = (HWND)bufHWND;
				playerType[0] = PlayerType::AI; 
				playerType[1] = PlayerType::OtherAI;
				playerName[0] = AI_VERSION;
				playerName[1] = "";
				break;
			default:
				continue;
			}
			break;
		}
	}
	
	if (playerType[0] == PlayerType::AI || playerType[1] == PlayerType::AI) {
		cout << "輸入搜尋的深度\n";
		cin >> Observer::depth;
		cin.ignore();
	}
	cout << "從board//" << CUSTOM_BOARD_FILE << "讀取多個盤面 並連續對打?\n";
	isCustomBoard = getchar() != '0';
	cin.ignore();
	cout << "結束時匯出紀錄?\n";
	Observer::isSaveRecord = getchar() != '0';
	cin.ignore();
	if (gameMode != 5) {
		cout << "確定要開始? ";
		system("pause");
		SendMessageByHWND(opponentHWND, "");
	}
	else {
		cout << "等待對方程式回應... 請不要按任意鍵 ";
		system("pause");
	}
	CreateDirectory(CA2W(REPORT_PATH), NULL);

	/*    AI初始化    */
	Zobrist::Initialize();
	if (playerType[0] == PlayerType::AI || playerType[1] == PlayerType::AI) {
		InitializeNS();
		InitializeTP();
	}
	do {
		/*    遊戲初始化    */
		minishogi.Initialize();
		cout << "---------- Game " << Observer::gameNum << " ----------\n";
		if (isCustomBoard && !minishogi.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
			if ((gameMode == 4 || gameMode == 5) && !isSwap) {
				// 先後手交換
				isSwap = true;
				swap(playerType[0], playerType[1]);
				swap(playerName[0], playerName[1]);
				readBoardOffset = 0;
				if (!minishogi.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
					break;
				}
			}
			else { 
				// 對打結束
				break;
			}
		}
		playDetailStr = REPORT_PATH + currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt";
		kifuStr = currTimeStr + "_Kifu_" + to_string(Observer::gameNum) + ".txt";
		if (Observer::isSaveRecord) {
			file.open(playDetailStr, ios::app);
			if (file) {
				if (playerName[0] != "") file << "#△ : " << playerName[0] << "\n";
				if (playerName[1] != "") file << "#▼ : " << playerName[1] << "\n";
				file.close();
			}
			else cout << "Error : Fail to Save PlayDetail Title.\n";
			file.open(KIFU_PATH + kifuStr, ios::app);
			if (file) {
				if (playerName[0] != "") file << "#△ : " << playerName[0] << "\n";
				if (playerName[1] != "") file << "#▼ : " << playerName[1] << "\n";
				file.close();
			}
			else cout << "Error : Fail to Save Kifu Title.\n";
		}
		CleanTP();

		/*    遊戲開始    */
		Observer::GameStart();
		while (!minishogi.IsGameOver()) {
			int actionU32, eval;
			cout << "---------- Game " << Observer::gameNum << " Step " << minishogi.GetStep() << " ----------\n";
			minishogi.PrintChessBoard();

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
				action.srcChess = 0;
				action.dstChess = 0;
				SendMessageByHWND(opponentHWND, to_string(action.ToU32()));
				break;
			case OtherAI:
				cin >> actionU32;
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

			if (action.mode == Action::SURRENDER) {
				cout << (!minishogi.GetTurn() ? "△" : "▼") << "投降! I'm lose\n";
				break;
			}
			else if (action.mode == Action::UNDO) {
				if (minishogi.GetStep() >= 2) {
					minishogi.UndoMove();
					minishogi.UndoMove();
					cout << "Success Undo!\n";
				}
				else {
					cout << "Error : Cannot Undo!\n";
				}
			}
			else if (action.mode == Action::SAVEBOARD) {
				minishogi.SaveBoard(GetCurrentTimeString() + "_Kifu", "");
			}
			else {
				minishogi.DoMove(action);
			}
			if (minishogi.GetStep() == 100) {
				cout << "千日手! I'm lose\n";
				break;
			}
		}
		/*    遊戲結束    */
		Observer::GameOver(minishogi.GetTurn() != isSwap, minishogi.GetKifuHash());
		minishogi.PrintChessBoard();
		cout << "-------- Game Over! " << (minishogi.GetTurn() ? "△" : "▼") << " Win! --------\n\n";
		Observer::PrintGameReport(cout);

		if (Observer::isSaveRecord) {
			if (gameMode != 5) minishogi.SaveKifu(kifuStr);
			file.open(playDetailStr, ios::app);
			if (file) {
				file << "#" << AI_VERSION << "\n";
				Observer::PrintGameReport(file);
				file.close();
			}
			else cout << "Error : Fail to Save PlayDetail.\n";
		}
	} while (isCustomBoard);
	if (Observer::isSaveRecord) {
		file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::app);
		if (file) {
			file << "#" << AI_VERSION << "\n";
			Observer::PrintTotalReport(file);
			file.close();
		}
		else cout << "Error : Fail to Save AI Report.\n";
	}
	cout << "---------- Game Over ----------\n";
	Observer::PrintTotalReport(cout);

	cout << "\a\a\a";
	//delete(transpositTable);
    system("pause");
    return 0;
}


bool Human_DoMove(Minishogi &board, Action &action) {
	cin.clear();
	cout << "請輸入移動指令(E5D5+)或其他指令(UNDO, SURRENDER, SAVEBOARD) : " << endl;
	cin >> action;
	cin.ignore();
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

void SendMessageByHWND(const HWND hwnd, const string message) {
	if (hwnd == 0) return;
	for (int i = 0; i < message.size(); i++) {
		PostMessage(hwnd, WM_KEYDOWN, message[i], 0);
	}
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
}

