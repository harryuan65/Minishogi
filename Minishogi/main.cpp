﻿/*
+Example Board
▼飛▼角▼銀▼金▼玉
．  ．  ．  ． ▼步
．  ．  ．  ．  ．
△步 ．  ．  ．  ．
△王△金△銀△角△飛
▼0步0銀2金0角0飛
△0步0銀0金1角0飛
*/
#include "head.h"
#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"
#define AI_VERSION		  "TPSize=0x8000000 國籍同構" //輸出報告註解用 "無同型表 pv enable"
using namespace std;

enum PlayerType {
	Human,
	AI,
	OtherAI
};

Action Human_DoMove(Board &board);
Action AI_DoMove(Board &board, PV &pv);

bool SavePlayDetail(string filename, string comment, string whiteName, string blackName);
bool SavePlayDetail(string filename, Board &board, Action action, PV &pv);
bool SaveAIReport(string filename, string comment, string player1Name, string player2Name);
bool SaveAIReport(string filename);
bool SaveAITotalReport(string filename);

void GetCurrentTimeString(string& out);
bool GetOpenFileNameString(string& out);
void SendMessageByHWND(const HWND hwnd, const string message);

int main(int argc, char **argv) {
	Board m_Board;
	Action action;
	int playerType[2];
	string playerName[2];
	int gameMode;
	bool isCustomBoard;
	bool isSwap = false;
	HWND opponentHWND;

	string currTimeStr;
	string gameDirStr;
	streamoff readBoardOffset = 0;

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
		GetCurrentTimeString(currTimeStr);
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

    /*int choice;
    cout << "自訂棋盤?0:1 = ";
    while (cin >> choice && choice != 0 && choice != 1);
    if (choice) {
        cout << "黑步 17 | 黑銀 18 | 黑金 19 | 黑角 20 | 黑車 21 | 黑王 22" << endl;
        cout << "白步  1 | 白銀  2 | 白金  3 | 白角  4 | 白車  5 | 白王  6" << endl << endl;
        cout << "升變 + 8" << endl;
        cout << "手排: 步 | 銀 | 金 | 角 | 車 的順序輸入 0~2 (可選)" << endl;
        cout << "請輸入 25 個數字 (+ 黑白 10 個手排)，並以 '.' 結尾 :" << endl;
        char s[128];
        cin.clear(); cin.ignore(128, '\n');
        cin.getline(s, 128, '.'); // 直到句點結束，所以可以斷行
        m_Board.Initialize(s);
    }
    else */

	Zobrist::Initialize();
	if (playerType[0] == PlayerType::AI || playerType[1] == PlayerType::AI) {
		InitializeTP();
		if (Observer::isSaveRecord) {
			if (gameMode != 5)
				SaveAIReport(currTimeStr + "_AIReport_AI1.txt", currTimeStr, playerName[0], playerName[1]);
			else
				SaveAIReport(currTimeStr + "_AIReport_AI2.txt", currTimeStr, playerName[0], playerName[1]);
		}
	}
	do {
		m_Board.Initialize();
		if (isCustomBoard && !m_Board.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
			if ((gameMode == 4 || gameMode == 5) && !isSwap) {
				isSwap = true;
				swap(playerType[0], playerType[1]);
				swap(playerName[0], playerName[1]);
				readBoardOffset = 0;
				if (!m_Board.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
					break;
				}
			}
			else {
				break;
			}
		}
		if (Observer::isSaveRecord) {
			SavePlayDetail(currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt", 
				currTimeStr, playerName[0], playerName[1]);
			m_Board.SaveKifu(currTimeStr + "_Kifu_" + to_string(Observer::gameNum) + ".txt",
				currTimeStr, playerName[0], playerName[1]);
		}
		CleanTP();
		Observer::GameStart(m_Board.GetZobristHash());
		while (!m_Board.IsGameOver()) {
			/*cout << (m_Board.GetTurn() ? "GREEN" : "RED") << " turn\n";
			Action moveList[128] = { 0 };
			U32 cnt = 0;
			AttackGenerator(m_Board, moveList, cnt);
			cout << "Atta cnt : " << cnt << "\n";
			cnt = 0;
			MoveGenerator(m_Board, moveList, cnt);
			cout << "Move cnt : " << cnt << "\n";
			cnt = 0;
			HandGenerator(m_Board, moveList, cnt);
			cout << "Hand cnt : " << cnt << "\n";*/
			/*cout << "是否紀錄 ? : (否: 0 / 存: 1 / 讀 : 2)\n";
			while (cin >> choice && choice != 0 && choice != 1 && choice != 2);
			if (choice == 1) {
				m_Board.SaveBoard("board.txt");
				continue;
			}
			else if (choice == 2) {
				m_Board.LoadBoard("board.txt");
				continue;
			}*/

			PV pv;
			cout << "---------- Game " << Observer::gameNum << " Step " << m_Board.GetStep() << " ----------\n";
			m_Board.PrintChessBoard();
			cout << (m_Board.GetTurn() ? "[▼ Turn]\n" : "[△ Turn]\n") << "\n";
			if (playerType[m_Board.GetTurn()] == PlayerType::Human) {
				if (m_Board.IsGameOver()) {
					action = 0;
				}
				else {
					while (!(action = Human_DoMove(m_Board)));
				}
			}
			else if (playerType[m_Board.GetTurn()] == PlayerType::AI) {
				Observer::StartSearching();
				action = AI_DoMove(m_Board, pv);
				Observer::EndSearching();

				cout << "Action : ";
				PrintAction(cout, action);
				cout << "\n";
				pv.Print(cout, m_Board.GetTurn());
				Observer::PrintReport(cout);
				if (gameMode == 4 || gameMode == 5) {
					SendMessageByHWND(opponentHWND, to_string(action));
				}
			}
			else if (playerType[m_Board.GetTurn()] == PlayerType::OtherAI) {
				cin >> action;
				PrintAction(cout, action);
			}

			if (Observer::isSaveRecord && playerType[m_Board.GetTurn()] != PlayerType::OtherAI) {
				SavePlayDetail(currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt", m_Board, action, pv);
			}
			if (!action) {
				cout << "投降! I'm lose" << endl;
				break;
			}
			if (m_Board.GetStep() == 100) {
				cout << "千日手! I'm lose" << endl;
				break;
			}
			m_Board.DoMove(action);
		}
		Observer::GameOver(m_Board.GetTurn() != isSwap, m_Board.GetKifuHash());
		cout << "-------- Game Over! " << (m_Board.GetTurn() ? "△" : "▼") << " Win! --------\n\n";
		if (Observer::isSaveRecord) {
			m_Board.SaveKifu(currTimeStr + "_Kifu_" + to_string(Observer::gameNum - 1) + ".txt");
			if (playerType[0] == PlayerType::AI || playerType[1] == PlayerType::AI) {
				if (gameMode != 5)
					SaveAIReport(currTimeStr + "_AIReport_AI1.txt");
				else
					SaveAIReport(currTimeStr + "_AIReport_AI2.txt");
			}
		}
		cout << "\n";
		Observer::PrintGameReport(cout);
	} while (isCustomBoard);
	if (Observer::isSaveRecord && (playerType[0] == PlayerType::AI || playerType[1] == PlayerType::AI)) {
		if (gameMode != 5)
			SaveAITotalReport(currTimeStr + "_AIReport_AI1.txt");
		else
			SaveAITotalReport(currTimeStr + "_AIReport_AI2.txt");
	}
	Observer::PrintTotalReport(cout);

	cout << "\a\a\a"; //終わり　びびびー
    system("pause");
    return 0;
}


Action Human_DoMove(Board &board) {
	string cmd;
	cin.clear();
	cout << "請輸入移動指令 (例 E5D5+) : " << endl;
	cin >> cmd;
	cin.ignore();
	if (cmd.length() != 4 && cmd.length() != 5) {
		cout << "Invalid Move : Wrong input length" << endl;
		return 0;
	}

	int srcIndex = Input2Index(cmd[0], cmd[1]);
	int dstIndex = Input2Index(cmd[2], cmd[3]);
	if (srcIndex == -1 || dstIndex == -1 || (cmd.length() == 5 && cmd[4] != '+')) {
		cout << "Invalid Move : Bad Input" << endl;
		return 0;

	}
	bool isPro = cmd.length() == 5;
	int srcChess;

	//src位置的棋是什麼
	if (srcIndex < BOARD_SIZE)
		srcChess = board.board[srcIndex];
	else {
		if (!board.board[srcIndex]) {
			cout << "Invalid Move: No chess to handmove" << endl;
			return 0;
		}
		srcChess = (srcIndex < 30 ? BLACKCHESS : 0) | (srcIndex % 5 + 1);
	}

	if (srcChess >> 4 != board.GetTurn()) {
		cout << "Invalid Move : " << CHESS_WORD[srcChess] << " is not your chess" << endl;
		return 0;
	}

	if (srcIndex >= BOARD_SIZE) { // 打入
		if (isPro) {
			cout << "Invalid Move : Promotion prohobited on handmove" << endl;
			return 0;
		}

		if (board.board[dstIndex]) {
			cout << "Invalid Move : " << CHESS_WORD[srcChess] << " 該位置有棋子" << endl;
			return 0;
		}

		if ((srcChess & 15) == PAWN) {
			if (!Movement[srcChess][dstIndex]) {
				cout << "Invalid Move : " << CHESS_WORD[srcChess] << " 打入該位置不能移動" << endl;
				return 0;
			}

			if (column_mask(dstIndex) & board.bitboard[srcChess]) {
				cout << "Invalid Move : " << CHESS_WORD[srcChess] << " 二步" << endl;
				return 0;
			}

			/* TODO: 步兵不可立即將死>打步詰(未做) */
		}
	}
	else {
		//處理升變 + 打入規則一：不能馬上升變
		if (isPro) {
			if ((srcChess == PAWN || srcChess == SILVER || srcChess == BISHOP || srcChess == ROOK ||
				srcChess == (PAWN | BLACKCHESS) || srcChess == (SILVER | BLACKCHESS) || srcChess == (BISHOP | BLACKCHESS) || srcChess == (ROOK | BLACKCHESS))) {
				//是否在敵區內
				if (!((1 << dstIndex) & (srcChess < BLACKCHESS ? BLACK_CAMP : WHITE_CAMP)) &&
					!((1 << srcIndex) & (srcChess < BLACKCHESS ? BLACK_CAMP : WHITE_CAMP))) {
					cout << "你不在敵區，不能升變" << endl;
					return 0;
				}
			}
			else {
				cout << "你又不能升變" << endl;
				return 0;
			}
		}

		//將該位置可走的步法跟目的步法 & 比對，產生該棋的board結果，不合法就會是0
		if (!(Movable(board, srcIndex) & (1 << dstIndex))) {
			cout << "Invalid Move :invalid " << CHESS_WORD[srcChess] << " move." << endl;
			return 0;
		}
	}

	return (isPro << 24) | (dstIndex << 6) | srcIndex;
}

Action AI_DoMove(Board &board, PV &pv) {
	cout << "AI 思考中..." << endl;
	return IDAS(board, pv);
}

bool SavePlayDetail(string filename, string comment, string whiteName, string blackName) {
	string filepath = REPORT_PATH + filename;
	fstream file(filepath, ios::out | ios::app);
	if (!file) {
		CreateDirectory(CA2W(REPORT_PATH), NULL);
		file.open(filepath, ios::out | ios::app);
	}
	if (file) {
		file << "#" << comment << "\n";
		file << "Zobrist Table Seed : " << Zobrist::SEED << "\n";
		if (whiteName != "") file << "△ : " << whiteName << "\n";
		if (blackName != "") file << "▼ : " << blackName << "\n";
		file.close();
		return true;
	}
	cout << "Fail Save PlayDetail to " << filepath << endl;
	return false;
}

bool SavePlayDetail(string filename, Board &board, Action action, PV &pv) {
	string filepath = REPORT_PATH + filename;
	fstream file(filepath, ios::out | ios::app);
	if (!file) {
		CreateDirectory(CA2W(REPORT_PATH), NULL);
		file.open(filepath, ios::out | ios::app);
		file.close();
		return true;
	}
	if (file) {
		file << "---------- Game " << Observer::gameNum << " Step " << board.GetStep() << " ----------\n";
		board.PrintNoncolorBoard(file);
		file << (board.GetTurn() ? "[▼ Turn]\n" : "[△ Turn]\n");
		if (pv.count != 0) {
			if (action) {
				file << "Action : ";
				PrintAction(file, action);
				file << "\n";
			}
			pv.Print(file, board.GetTurn());
			Observer::PrintReport(file);
		}
		file.close();
		return true;
	}
	cout << "Fail Save PlayDetail to " << filepath << endl;
	return false;
}

bool SaveAIReport(string filename, string comment, string player1Name, string player2Name) {
	string filepath = REPORT_PATH + filename;
	fstream file(filepath, ios::app);
	if (!file) {
		CreateDirectory(CA2W(REPORT_PATH), NULL);
		file.open(filepath);
	}
	if (file) {
		file << "#" << comment << "\n";
		file << "Zobrist Table Seed : " << Zobrist::SEED << "\n";
		if (player1Name != "") file << "Player 1 : " << player1Name << "\n";
		if (player2Name != "") file << "Player 2 : " << player2Name << "\n";
		cout << "Success Save AI Report to " << filepath << "\n";
		file.close();
		return true;
	}
	cout << "Fail Save AI Report to " << filepath << "\n";
	return false;
}

bool SaveAIReport(string filename) {
	string filepath = REPORT_PATH + filename;
	fstream file(filepath, ios::app);
	if (!file) {
		CreateDirectory(CA2W(REPORT_PATH), NULL);
		file.open(filepath);
	}
	if (file) {
		Observer::PrintGameReport(file);
		file << "\n";
		cout << "Success Save AI Report to " << filepath << "\n";
		file.close();
		return true;
	}
	cout << "Fail Save AI Report to " << filepath << "\n";
	return false;
}

bool SaveAITotalReport(string filename) {
	string filepath = REPORT_PATH + filename;
	fstream file(filepath, ios::app);
	if (!file) {
		CreateDirectory(CA2W(REPORT_PATH), NULL);
		file.open(filepath);
	}
	if (file) {
		Observer::PrintTotalReport(file);
		file << "\n";
		cout << "Success Save AI Report to " << filepath << "\n";
		file.close();
		return true;
	}
	cout << "Fail Save AI Report to " << filepath << "\n";
	return false;
}

void GetCurrentTimeString(string &out) {
	char buffer[80];
	time_t rawtime;
	time(&rawtime);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", localtime(&rawtime));
	out.assign(buffer);
}

bool GetOpenFileNameString(string& out) {
	char szFile[100];
	OPENFILENAME ofn;

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
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	bool isOK = GetOpenFileName(&ofn);
	out.assign(CW2A(ofn.lpstrFile));
	return isOK;
}

void SendMessageByHWND(const HWND hwnd, const string message) {
	for (int i = 0; i < message.size(); i++) {
		PostMessage(hwnd, WM_KEYDOWN, message[i], 0);
	}
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
}