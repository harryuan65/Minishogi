#include "head.h"
#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"
#define AI_VERSION		  "TPSize=1<<30 國籍同構 舊的ver" //輸出報告註解用
using namespace std;

enum PlayerType {
	Human,
	AI,
	OtherAI
};

Action Human_DoMove(Board &board);
Action AI_DoMove(Board &board, PV &pv);

string GetCurrentTimeString();
bool GetOpenFileNameString(string& out);
void SendMessageByHWND(const HWND hwnd, const string message);
string Action2String(Action action);

int main(int argc, char **argv) {
	Board m_Board;
	Action action;
	PV pv;

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
		InitializeTP();
	}
	do {
		/*    遊戲初始化    */
		cout << "---------- Game " << Observer::gameNum << " ----------\n";
		m_Board.Initialize();
		if (isCustomBoard && !m_Board.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
			if ((gameMode == 4 || gameMode == 5) && !isSwap) {
				// 先後手交換
				isSwap = true;
				swap(playerType[0], playerType[1]);
				swap(playerName[0], playerName[1]);
				readBoardOffset = 0;
				if (!m_Board.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
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
		while (!m_Board.IsGameOver()) {
			cout << "---------- Game " << Observer::gameNum << " Step " << m_Board.GetStep() << " ----------\n";
			m_Board.PrintChessBoard();

			switch (playerType[m_Board.GetTurn()]) {
			case Human:
				while (!(action = Human_DoMove(m_Board)));
				cout << "Action : " << Action2String(action) << "\n";
				break;
			case AI:
				Observer::StartSearching();
				action = AI_DoMove(m_Board, pv);
				Observer::EndSearching();

				cout << "Action : " << Action2String(action) << "\n";
				//PrintPV(cout, m_Board, Observer::depth);
				pv.Print(cout, m_Board.GetTurn());
				Observer::PrintSearchReport(cout);
				SendMessageByHWND(opponentHWND, to_string(action));
				break;
			case OtherAI:
				cin >> action;
				cout << "Action : " << Action2String(action) << "\n";
				break;
			}
			if (Observer::isSaveRecord && playerType[m_Board.GetTurn()] != PlayerType::OtherAI) {
				file.open(playDetailStr, ios::app);
				if (file) {
					file << "---------- Game " << Observer::gameNum << " Step " << m_Board.GetStep() << " ----------\n";
					m_Board.PrintNoncolorBoard(file);
					file << "Action : " << Action2String(action) << "\n";
					//PrintPV(file, m_Board, Observer::depth);
					pv.Print(file, m_Board.GetTurn());
					Observer::PrintSearchReport(file);
					file.close();
				}
				else cout << "Error : Fail to Save PlayDetail.\n";
			}

			if (action == ACTION_SURRENDER) {
				cout << (!m_Board.GetTurn() ? "△" : "▼") << "投降! I'm lose\n";
				break;
			}
			else if (action == ACTION_UNDO) {
				if (m_Board.GetTurn() >= 2) {
					m_Board.UndoMove();
					m_Board.UndoMove();
					cout << "Success Undo!\n";
				}
				else {
					cout << "Error : Cannot Undo!\n";
				}
			}
			else if (action == ACTION_SAVEBOARD) {
				m_Board.SaveBoard(GetCurrentTimeString() + "_Kifu", "");
			}
			else {
				m_Board.DoMove(action);
			}
			if (m_Board.GetStep() == 100) {
				cout << "千日手! I'm lose\n";
				break;
			}
		}
		/*    遊戲結束    */
		Observer::GameOver(m_Board.GetTurn() != isSwap, m_Board.GetKifuHash());
		m_Board.PrintChessBoard();
		cout << "-------- Game Over! " << (m_Board.GetTurn() ? "△" : "▼") << " Win! --------\n\n";
		Observer::PrintGameReport(cout);

		if (Observer::isSaveRecord) {
			if (gameMode != 5) m_Board.SaveKifu(kifuStr);
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
    system("pause");
    return 0;
}


Action Human_DoMove(Board &board) {
	string cmd;
	Action action, moveList[MAX_MOVE_NUM] = { 0 };
	U32 cnt = 0;

	cin.clear();
	cout << "請輸入移動指令(E5D5+)或其他指令(UNDO, SURRENDER, SAVEBOARD) : " << endl;
	cin >> cmd;
	cin.ignore();
	/*    Command    */
	if (cmd == "SURRENDER" || cmd == "surrender") {
		return ACTION_SURRENDER;
	}
	if (cmd == "UNDO" || cmd == "undo") {
		return ACTION_UNDO;
	}
	if (cmd == "SAVEBOARD" || cmd == "saveboard") {
		return ACTION_SAVEBOARD;
	}
	if (cmd.length() != 4 || (cmd.length() == 5 && cmd[4] != '+')) {
		cout << "Error : Wrong Command." << endl;
		return 0;
	}
	/*    Move    */
	action = ((cmd.length() == 5) << 24) | (Input2Index(cmd[2], cmd[3]) << 6) | Input2Index(cmd[0], cmd[1]);
	AttackGenerator(board, moveList, cnt);
	MoveGenerator(board, moveList, cnt);
	HandGenerator(board, moveList, cnt); 
	for (int i = 0; i < cnt; i++) {
		if (action == moveList[i]) {
			if (board.IsCheckAfter(ACTION_TO_SRCINDEX(action), ACTION_TO_DSTINDEX(action))) {
				cout << "Error : You have been checked." << endl;
				return 0;
			}
			else {
				return action;
			}
		}
	}
	cout << "Error : Invaild Move." << endl;
	return 0;
}

Action AI_DoMove(Board &board, PV &pv) {
	cout << "AI 思考中..." << endl;
	return IDAS(board, pv);
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

string Action2String(Action action) {
	if (action == ACTION_SURRENDER) {
		return "SURRENDER";
	}
	else if (action == ACTION_UNDO) {
		return "UNDO";
	}
	else if (action == ACTION_SAVEBOARD) {
		return "SAVEBOARD";
	}
	else {
		return Index2Input(ACTION_TO_SRCINDEX(action)) +
			Index2Input(ACTION_TO_DSTINDEX(action)) +
			(ACTION_TO_ISPRO(action) ? "+" : " ");
	}
}