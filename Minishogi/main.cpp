#define _CRT_SECURE_NO_WARNINGS
#include <atlstr.h>
#include <fstream>

#include "FileMapping.h"
#include "Thread.h"
#include "Minishogi.h"
#include "Search.h"
#include "Observer.h"
#include "Transposition.h"

#define AI_VERSION		  "Add Pinner"
#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"
#define BUFFER_SIZE		  200 * sizeof(int)
using namespace std;

struct Players {
	enum PlayerType : int {
		Human,
		AI,
		OtherAI
	} pType[2];
	string pName[2];
	Thread *pthread[2] = { nullptr };
	bool isSwap;

	void Set(int gm, string ai1, string ai2) {
		switch (gm) {
		case 0:
			pType[0] = PlayerType::Human;
			pType[1] = PlayerType::AI;
			pName[0] = "Human";
			pName[1] = ai1;
			break;
		case 1:
			pType[0] = PlayerType::AI;
			pType[1] = PlayerType::Human;
			pName[0] = ai1;
			pName[1] = "Human";
			break;
		case 2:
			pType[0] = PlayerType::Human;
			pType[1] = PlayerType::Human;
			pName[0] = "Human";
			pName[1] = "Human";
			break;
		case 3:
			pType[0] = PlayerType::AI;
			pType[1] = PlayerType::AI;
			pName[0] = ai1;
			pName[1] = ai1;
			break;
		case 4:
			pType[0] = PlayerType::AI;
			pType[1] = PlayerType::OtherAI;
			pName[0] = ai1;
			pName[1] = ai2;
			break;
		case 5:
			pType[0] = PlayerType::OtherAI;
			pType[1] = PlayerType::AI;
			pName[0] = ai1;
			pName[1] = ai2;
			break;
		}
	}
	void SwapPlayer() {
		swap(pType[0], pType[1]);
		swap(pName[0], pName[1]);
		swap(pthread[0], pthread[1]);
		isSwap = true;
		sync_cout << "Swap player." << sync_endl;
	}
	void InitThread(const Minishogi &pos) {
		if (pType[0] == AI) {
			if (pthread[0])	delete pthread[0];
			pthread[0] = new Thread(pos, Color::WHITE);
		}
		if (pType[1] == AI) {
			if (pthread[1])	delete pthread[1];
			pthread[1] = new Thread(pos, Color::BLACK);
		}
	}
	inline void PrintNames(ostream &os) {
		os << "#▼ : " << pName[1] << "\n";
		os << "#△ : " << pName[0] << "\n";
	}
	inline bool IsAnyAI() { return pType[0] == AI || pType[1] == AI; }
	inline bool IsAnyOtherAI() { return pType[0] == OtherAI || pType[1] == OtherAI; }
};

enum IndicatorType {
	GAMEMODE = 1, ACK, MOVELIST, DOMOVE, AIPV, GAMEOVER, RESTART, BREAK
};

const char selectModeStr[] =
"請選擇對手:\n"
"(0)玩家vs電腦\n"
"(1)電腦vs玩家\n"
"(2)玩家對打\n"
"(3)電腦對打\n"
"(4)電腦對打 本機vs其他程式\n"
"(6)介面Mode\n"
"(7)介面Debug";

bool Human_DoMove(Minishogi &board, Move &move);
bool UI_DoMove(Minishogi &board, Move &move, FileMapping &fm);
string GetCurrentTimeString();
bool GetOpenFileNameString(string& out);

int main(int argc, char **argv) {
	Minishogi minishogi;
	Players players;
	int gameMode;
	bool isCustomBoard = false;
	bool isConnectUI = false;

	FileMapping fm_mg, fm_gm;
	int buffer[BUFFER_SIZE], indicator[2];

	FileMapping fm_game;
	fstream file;
	streamoff readBoardOffset = 0;
	string gameDirStr;
	string currTimeStr;
	string playDetailStr = "";
	string kifuStr = "";

	// Game Setting
	SetConsoleTitle(L"Minishogi - " AI_VERSION);
	sync_cout << "AI Version : " << AI_VERSION << "\n" << Search::GetSettingStr() << sync_endl;
	if (argc != 6) {
		string buf;
		currTimeStr = GetCurrentTimeString();
		fm_game.Open(currTimeStr);
		for (;;) {
			sync_cout << selectModeStr << sync_endl;
			cin >> gameMode;
			switch (gameMode)
			{
			case 0:	case 1:	case 2:	case 3:	
				break;
			case 4:
				if (!GetOpenFileNameString(gameDirStr))
					continue;
				break;
			case 6:
				fm_mg.Open("fm_mg_" + currTimeStr);
				fm_gm.Open("fm_gm_" + currTimeStr);
				isConnectUI = true;
				break;
			case 7:
				fm_mg.Open("fm_mg_");
				fm_gm.Open("fm_gm_");
				gameMode = 6;
				isConnectUI = true;
				break;
			default:
				continue;
			}
			players.Set(gameMode, AI_VERSION, "");
			break;
		}

		if (players.IsAnyAI() || isConnectUI) {
			sync_cout << "輸入搜尋的深度" << sync_endl;
			cin >> Observer::depth;
		}
		sync_cout << "從board//" << CUSTOM_BOARD_FILE << "讀取多個盤面 並連續對打(y/n)?" << sync_endl;
		cin >> buf;
		isCustomBoard = (buf == "y" || buf == "Y");

		sync_cout << "確定要開始 不開始則進入進階選項(y/n)?" << sync_endl;
		cin >> buf;
		if (buf != "y" && buf != "Y") {
			sync_cout << "輸入時間限制(ms 0為無限制)" << sync_endl;
			cin >> Observer::limitTime;

			sync_cout << "結束時匯出紀錄(y/n)?" << sync_endl;
			cin >> buf;
			Observer::isSaveRecord = (buf == "y" || buf == "Y");

			sync_cout << Search::GetSettingStr() << "確定要開始?" << sync_endl;
			system("pause");
		}
		if (gameMode == 4) {
			fm_game.SendMsg(nullptr, 0, false);
			// argv[6] = 遊戲路徑 輸出檔名 玩家名 深度 是否自訂棋盤 是否輸出
			system(("start \"\" \"" + gameDirStr + "\" " +
				currTimeStr + " " +
				"\"" + AI_VERSION + "\" " +
				to_string(Observer::depth) + " " +
				to_string(isCustomBoard) + " " +
				to_string(Observer::isSaveRecord)).c_str());
		}
		else if (isConnectUI) {
			system(("start Shogi.exe " + currTimeStr).c_str());
		}
	}
	else {
		// argv[6] = 遊戲路徑 輸出檔名 玩家名 深度 是否自訂棋盤 是否輸出
		gameMode = 5;
		currTimeStr = argv[1];
		fm_game.Open(currTimeStr);
		players.Set(gameMode, argv[2], AI_VERSION);
		Observer::depth = atoi(argv[3]);
		isCustomBoard = argv[4][0] != '0';
		Observer::isSaveRecord = argv[5][0] != '0';
	}
	CreateDirectory(CA2W(REPORT_PATH), NULL);

	// AI Init
	Zobrist::Initialize();
	if (players.IsAnyAI() || isConnectUI) {
		Transposition::Initialize();
	}

	// MultiGame Loop
	do {
		// Game Init    
		minishogi.Initialize();
		Transposition::Clean();

		// Board Init
		if (isConnectUI) {
			sync_cout << "[NewGame]Waiting for UI's gamemode..." << sync_endl;
			fm_gm.RecvMsg(buffer, sizeof(buffer), true); //*********Recv gamemode
			if (buffer[0] == GAMEMODE) {
				sync_cout << "[NewGame]Waiting for mode... " << sync_endl;
				fm_gm.RecvMsg(buffer, sizeof(buffer), true);
				indicator[0] = ACK;
				sync_cout << "[NewGame]Sending ACK." << sync_endl;
				fm_mg.SendMsg(indicator, sizeof(int), true);
				if (buffer[0] >= 0 && buffer[0] <= 5)
					players.Set(gameMode, AI_VERSION, "UI_Player");
				else
					sync_cout << "Error : Reciver wrong gameMode from UI : " << buffer[0] << sync_endl;
			}
		}
		else if (isCustomBoard && !minishogi.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
			if (players.IsAnyOtherAI() && !players.isSwap) {
				// 先後手交換
				players.SwapPlayer();
				readBoardOffset = 0;
				continue;
			}
			else {
				// 對打結束
				break;
			}
		}
		players.InitThread(minishogi);

		// Write Title
		string titleStr = "Minishogi - " AI_VERSION " Game " + to_string(Observer::gameNum);
		SetConsoleTitle(CA2W(titleStr.c_str()));
		sync_cout << "---------- Game " << Observer::gameNum << " ----------" << sync_endl;
		if (Observer::isSaveRecord) {
			playDetailStr = REPORT_PATH + currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt";
			kifuStr = REPORT_PATH + currTimeStr + "_Kifu_" + to_string(Observer::gameNum) + ".txt";
			if (gameMode != 4) {
				file.open(playDetailStr, ios::app);
				if (file) players.PrintNames(file);
				file.close();
			}
			else {
				Sleep(10); // 先讓另一邊寫完檔案
			}
		}

		// Game Loop
		Observer::GameStart();
		while (true) {
			int turn = minishogi.GetTurn();
			Move move;
			// Print & Write ChessBoard
			cout << SyncCout::IO_LOCK;
			minishogi.PrintChessBoard();
			cout << SyncCout::IO_UNLOCK;
			if (Observer::isSaveRecord && players.pType[turn] != Players::OtherAI) {
				file.open(playDetailStr, ios::app);
				if (file) {
					file << "---------- Game " << Observer::gameNum << " Step " << minishogi.GetStep() << " ----------\n";
					minishogi.PrintNoncolorBoard(file);
					if (minishogi.IsGameOver()) 
						file << (turn ? "▼" : "△") << " Cannot Move.\n";
					file.close();
				}
			}

			// Game Over, Break game loop
			if (minishogi.IsGameOver()) {
				sync_cout << (turn ? "▼" : "△") << " Cannot Move." << sync_endl;
				break;
			}

			// Input Move
			switch (players.pType[turn]) {
			case Players::Human:
				if (isConnectUI) {
					Move legalMoves[TOTAL_GENE_MAX_ACTIONS], *endMove;
					endMove = minishogi.GetLegalMoves(legalMoves);
					indicator[0] = MOVELIST;
					fm_mg.SendMsg(indicator, sizeof(int), true);
					fm_mg.SendMsg(legalMoves, (endMove - legalMoves) * sizeof(Move), true);
					while (!UI_DoMove(minishogi, move, fm_gm));
					if (move == -1) move = MOVE_NULL;
				}
				else {
					while (!Human_DoMove(minishogi, move));
				}

				file.open(playDetailStr, ios::app);
				if (file) file << (turn ? "▼" : "△") << "DoMove : " << move << "\n";
				file.close();
				break;
			case Players::OtherAI:
				uint32_t actionU32;
				fm_game.RecvMsg(&actionU32, sizeof(uint32_t), true);
				move = setU32(actionU32);
				sync_cout << (turn ? "▼" : "△") << "DoMove : " << move << sync_endl;
				break;
			case Players::AI:
				RootMove rm = players.pthread[turn]->GetBestMove();
				
				cout << endl << SyncCout::IO_LOCK;
				Observer::PrintSearchReport(cout);
				cout << SyncCout::IO_UNLOCK;
				file.open(playDetailStr, ios::app);
				Observer::PrintSearchReport(file);
				file.close();
				if (isConnectUI) {
					indicator[0] = AIPV;
					fm_mg.SendMsg(indicator, sizeof(indicator), true);
					fm_mg.SendMsg(rm.pv, sizeof(rm.pv), true);
				}
				move = rm.pv[0];
				sync_cout << rm.PV() << "\n" << (turn ? "▼" : "△") << "DoMove : " << move << sync_endl;

				file.open(playDetailStr, ios::app);
				if (file) file << rm.PV() << "\n" << (turn ? "▼" : "△") << "DoMove : " << move << endl;
				file.close();
				break;
			}
				
			// Execute Move Type
			if (move == MOVE_NULL) {
				sync_cout << (turn ? "▼" : "△") << "Surrender! I'm lose." << sync_endl;
				break;
			}
			else if (move == MOVE_UNDO) {
				if (minishogi.GetStep() >= 2) {
					minishogi.UndoMove();
					minishogi.UndoMove();
					sync_cout << "Succeed to Undo!" << sync_endl;
				}
				else {
					sync_cout << "Error : Cannot Undo!" << sync_endl;
				}
			}
			else {
				minishogi.DoMove(move);
			}

			// Send to Another Program or Thread
			if (players.pType[!turn] == Players::OtherAI) {
				uint32_t actionU32 = toU32(move);
				fm_game.SendMsg(&actionU32, sizeof(uint32_t), true);
			}
			else if (players.pType[!turn] == Players::AI) {
				players.pthread[!turn]->SetEnemyMove(move);
			}
		}

		// Game Over
		Observer::GameOver(!minishogi.GetTurn(), players.isSwap, minishogi.GetKey(0), minishogi.GetKifuHash());
		cout << "-------- Game Over! " << (!minishogi.GetTurn() ? "▼" : "△") << " Win! --------\n";
		cout << SyncCout::IO_LOCK;
		Observer::PrintGameReport(cout);
		cout << SyncCout::IO_UNLOCK;
		if (isConnectUI) {
			indicator[0] = GAMEOVER;
			indicator[1] = !minishogi.GetTurn();
			fm_mg.SendMsg(indicator, 2 * sizeof(int), true);
		}

		// Save Record
		if (Observer::isSaveRecord) {
			if (gameMode == 5) {
				char msg[20];
				fm_game.RecvMsg(msg, 20, true);
				if (strcmp("Save Report", msg)) {
					sync_cout << "Error : 與另一支程式棋盤不同步" << sync_endl;
					system("pause");
				}
			}
			// Save PlayDetail
			file.open(playDetailStr, ios::app);
			if (file) {
				file << "-------- Game Over! " << (!minishogi.GetTurn() ? "▼" : "△") << " Win! --------\n";
				file << "#" << "Player " << (gameMode == 5 ? "2 : " : "1 : ") << AI_VERSION << "\n";
				file << Search::GetSettingStr() << "\n";
				Observer::PrintGameReport(file);
				file.close();
			}
			// Save Kifu
			if (gameMode != 4) {
				file.open(kifuStr, ios::app);
				if (file) {
					players.PrintNames(file);
					minishogi.PrintKifu(file);
					file.close();
				}
				else sync_cout << "Error : Fail to Save Kifu." << sync_endl;
			}
			// Save AIReport
			if (gameMode != 5) {
				file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::out);
				if (file) {
					file << "Start at : " << currTimeStr << "\nEnd at   : " << GetCurrentTimeString() << "\n\n";
					Observer::PrintWinnerReport(file);
					file << "#" << "Player " << (gameMode == 5 ? "2 : " : "1 : ") << AI_VERSION << "\n";
					file << Search::GetSettingStr() << "\n";
					Observer::PrintTotalReport(file);
					file.close();
				}
				else sync_cout << "Error : Fail to Save AI Report." << sync_endl;
			}
			else {
				file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::app);
				if (file) {
					file << "#" << "Player " << (gameMode == 5 ? "2 : " : "1 : ") << AI_VERSION << "\n";
					file << Search::GetSettingStr() << "\n";
					Observer::PrintTotalReport(file);
					file.close();
				}
				else sync_cout << "Error : Fail to Save AI Report." << sync_endl;
			}
			if (gameMode == 4) {
				fm_game.SendMsg("Save Report", 13, true);
			}

			if (isConnectUI) {
				do {
					sync_cout << "Waiting for RESTART..." << sync_endl;
					fm_gm.RecvMsg(buffer, sizeof(buffer), true);
				} while (buffer[0] != RESTART);
			}
		}
	} while (isCustomBoard);

	// Print Total Report;
	cout << SyncCout::IO_LOCK;
	cout << "---------- Total Report ----------\n";
	Observer::PrintWinnerReport(cout);
	Observer::PrintTotalReport(cout);;
	cout << SyncCout::IO_UNLOCK;

	SetConsoleTitle(L"Minishogi - " AI_VERSION " Stop");
	cout << "\a\a\a";
	system("pause");
	return 0;
}


bool Human_DoMove(Minishogi &board, Move &move) {
	sync_cout << "請輸入移動指令(E5D5+)或其他指令(UNDO, SURRENDER) : " << sync_endl;
	cin >> move;

	if (IsDoMove(move) && !board.IsLegelAction(move) &&
		!board.IsLegelAction(make_move(from_sq(move), to_sq(move), true))) {
		sync_cout << "Error : Illegal move." << sync_endl;
		return false;
	}
	else if (move == MOVE_ILLEGAL) {
		sync_cout << "Error : Unrecognized command." << sync_endl;
		return false;
	}
	return true;
}

bool UI_DoMove(Minishogi &board, Move &move, FileMapping &fm) {
	Move buffer[2];
	sync_cout << "Waiting for player..." << sync_endl;
	fm.RecvMsg(buffer, sizeof(buffer), true); //Recv Playermove
	if (buffer[0] == BREAK)
	{
		move = (Move)-1;
		return true;
	}
	else if (buffer[0] == DOMOVE)//GUI = Ack
	{
		move = (Move)buffer[1];
		return true;
	}
	if (IsDoMove(move) && !board.IsLegelAction(move) &&
		!board.IsLegelAction(make_move(from_sq(move), to_sq(move), true))) {
		sync_cout << "Error : Illegal move." << sync_endl;
		return false;
	}
	else if (move == MOVE_ILLEGAL) {
		sync_cout << "Error : Unrecognized command." << sync_endl;
		return false;
	}
	return true;
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

