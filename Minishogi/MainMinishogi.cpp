#include <fstream>

#include "FileMapping.h"
#include "Thread.h"
#include "Evaluate.h"
#include "Minishogi.h"
#include "Search.h"
#include "Observer.h"
#include "Transposition.h"
using namespace std;

#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"

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
			pthread[0] = new Thread(pos, WHITE);
		}
		if (pType[1] == AI) {
			if (pthread[1])	delete pthread[1];
			pthread[1] = new Thread(pos, BLACK);
		}
	}
	void StartThread() {
		if (pType[0] == AI && pthread[0]) {
			pthread[0]->Start();
		}
		if (pType[1] == AI && pthread[1]) {
			pthread[1]->Start();
		}
	}
	void DeleteThread() {
		if (pType[0] == AI && pthread[0]) {
			delete pthread[0];
		}
		if (pType[1] == AI && pthread[1]) {
			delete pthread[1];
		}
	}
	inline void PrintNames(ostream &os) {
		os << "#▼ : " << pName[1] << "\n";
		os << "#△ : " << pName[0] << "\n";
	}
	inline bool IsAnyAI() { return pType[0] == AI || pType[1] == AI; }
	inline bool IsAnyOtherAI() { return pType[0] == OtherAI || pType[1] == OtherAI; }
};

const char selectModeStr[] =
"請選擇對手:\n"
"(0)玩家vs電腦\n"
"(1)電腦vs玩家\n"
"(2)玩家對打\n"
"(3)電腦對打(Debug用 測試數據與決策結果不準)\n"
"(4)電腦對打 本機vs其他程式";

bool GetOpenFileNameString(string& out);

int main(int argc, char **argv) {
	Minishogi pos;
	Players players;
	int gameMode;
	bool isCustomBoard;

	FileMapping fm;
	fstream file;
	streamoff readBoardOffset = 0;
	string currTimeStr, gameDirStr, playDetailPath, buf;

	// Game Setting
	setlocale(LC_ALL, "");
	SetConsoleTitle("Minishogi - " AI_VERSION);
	sync_cout << "AI Version : " << AI_VERSION << "\n" << Search::GetSettingStr() << sync_endl;

	if (argc != 4) {
		currTimeStr = Observer::GetTimeStamp();
		fm.Open(currTimeStr);
		while (true) {
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
			default:
				continue;
			}
			players.Set(gameMode, AI_VERSION, "");
			sync_cout << "從board//" << CUSTOM_BOARD_FILE << "讀取多個盤面 並連續對打(y/n)?" << sync_endl;
			cin >> buf;
			isCustomBoard = (buf == "y" || buf == "Y");
			break;
		}
	}
	else {
		// argv[4] = 遊戲路徑 輸出檔名 玩家名 是否自訂棋盤
		gameMode = 5;
		currTimeStr = argv[1];
		fm.Open(currTimeStr);
		players.Set(gameMode, argv[2], AI_VERSION);
		isCustomBoard = argv[3][0] != '0';
	}

	if (players.IsAnyAI()) {
		sync_cout << "輸入搜尋的深度" << sync_endl;
		cin >> Observer::depth;

#ifndef KPPT_DISABLE
		cout << "輸入KPP名稱" << endl;
		cin >> Observer::kpptName;
		if (!Evaluate::evaluater.Load(Observer::kpptName)) {
			Observer::kpptName = "";
			Evaluate::evaluater.Clean();
		}
#else
		Evaluate::evaluater.Clean();
#endif
	}

	sync_cout << "確定要開始 不開始則進入進階選項(y/n)?" << sync_endl;
	cin >> buf;
	if (buf != "y" && buf != "Y") {
		if (players.IsAnyAI()) {
#ifndef TRANSPOSITION_DISABLE
			int size;
			sync_cout << "輸入同型表entry數量(2^n)" << sync_endl;
			cin >> size;
			Transposition::TPSize = 1 << (uint64_t)size;
#endif

			sync_cout << "輸入時間限制(ms 0為無限制)" << sync_endl;
			cin >> Observer::limitTime;
		}

		if (Observer::isSaveRecord && (gameMode == 0 || gameMode == 1)) {
			sync_cout << "輸入對手的名字" << sync_endl;
			cin >> players.pName[gameMode];
		}

		sync_cout << Search::GetSettingStr() << "確定要開始?" << sync_endl;
		system("pause");
	}
	if (gameMode == 4) {
		fm.SendMsg(nullptr, 0, false);
		// argv[4] = 遊戲路徑 輸出檔名 玩家名 是否自訂棋盤
		system(("start \"\" \"" + gameDirStr + "\" " + currTimeStr + " " +
			"\"" + AI_VERSION + "\" " +	to_string(isCustomBoard)).c_str());
		char msg[20];
		sync_cout << "等待對方回應..." << sync_endl;
		fm.RecvMsg(msg, 20, true);
		if (strcmp("Start Game", msg)) {
			sync_cout << "Error : 連線失敗" << sync_endl;
			system("pause");
		}
	}
	else if (gameMode == 5) {
		fm.SendMsg("Start Game", 11, true);
	}
	CreateDirectory(REPORT_PATH, NULL);

	// AI Init
	Zobrist::Initialize();
	if (players.IsAnyAI())
		Transposition::Initialize();

	// MultiGame Loop
	do {
		// Game Init
		pos.Initialize();

		// Board Init
		if (isCustomBoard && !pos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
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
		Transposition::Clean();
		players.InitThread(pos);

		// Write Title
		SetConsoleTitle(("Minishogi - " AI_VERSION " Game " + to_string(Observer::gameNum)).c_str());
		sync_cout << "---------- Game " << Observer::gameNum << " ----------" << sync_endl;
		if (Observer::isSaveRecord) {
			playDetailPath = REPORT_PATH + currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt";
			//kifuStr = REPORT_PATH + currTimeStr + "_Kifu_" + to_string(Observer::gameNum) + ".txt";
			if (gameMode != 4) {
				file.open(playDetailPath, ios::app);
				if (file) players.PrintNames(file);
				file.close();
			}
			else {
				Sleep(10); // 先讓另一邊寫完檔案
			}
		}

		// Game Loop
		Observer::GameStart();
		players.StartThread();
		while (true) {
			int turn = pos.GetTurn();
			Move move;
			// Print & Write ChessBoard
			cout << SyncCout::IO_LOCK;
			pos.PrintChessBoard();
			cout << "Evaluate : " << setw(15) << pos.GetEvaluate() << "\n";
			cout << SyncCout::IO_UNLOCK;
			if (Observer::isSaveRecord && players.pType[turn] != Players::OtherAI) {
				file.open(playDetailPath, ios::app);
				if (file) {
					file << "---------- Game " << Observer::gameNum << " Step " << pos.GetStep() << " ----------\n";
					pos.PrintNoncolorBoard(file);
					file << "Evaluate : " << setw(15) << pos.GetEvaluate() << "\n";
					if (pos.IsGameOver()) 
						file << COLOR_WORD[turn] << " Cannot Move.\n";
					file.close();
				}
			}

			// Game Over, Break game loop
			if (pos.IsGameOver()) {
				sync_cout << COLOR_WORD[turn] << " Cannot Move." << sync_endl;
				break;
			}

			// Input Move
			switch (players.pType[turn]) {
			case Players::Human:
				while (true) {
					sync_cout << "請輸入移動指令或其他指令(UNDO, SURRENDER) : " << sync_endl;
					cin >> move;

					if (IsDoMove(move) && !pos.IsLegelAction(move) &&
						!pos.IsLegelAction(make_move(from_sq(move), to_sq(move), true))) {
						sync_cout << "Error : Illegal move." << sync_endl;
						continue;
					}
					else if (!IsDoMove(move) && move != MOVE_UNDO) {
						sync_cout << "Error : Unrecognized command." << sync_endl;
						continue;
					}
					break;
				}

				file.open(playDetailPath, ios::app);
				if (file) file << COLOR_WORD[turn] << "DoMove : " << move << "\n";
				file.close();
				break;
			case Players::OtherAI:
				uint32_t actionU32;
				fm.RecvMsg(&actionU32, sizeof(uint32_t), true);
				move = setU32(actionU32);
				sync_cout << COLOR_WORD[turn] << "DoMove : " << move << sync_endl;
				break;
			case Players::AI:
				RootMove rm = players.pthread[turn]->GetBestMove();
				
				cout << endl << SyncCout::IO_LOCK;
				Observer::PrintSearchReport(cout);
				cout << SyncCout::IO_UNLOCK;
				file.open(playDetailPath, ios::app);
				Observer::PrintSearchReport(file);
				file.close();
				move = rm.pv[0];
				sync_cout << COLOR_WORD[turn] << "DoMove : " << move << sync_endl;

				file.open(playDetailPath, ios::app);
				if (file) {
					players.pthread[turn]->Dump(file);
					file << COLOR_WORD[turn] << "DoMove : " << move << endl;
				}
				file.close();
				break;
			}
				
			// Execute Move Type
			if (move == MOVE_NULL) {
				sync_cout << COLOR_WORD[turn] << "Surrender! I'm lose." << sync_endl;
				break;
			}
			else if (move == MOVE_UNDO) {
				if (pos.GetStep() >= 2) {
					pos.UndoMove();
					pos.UndoMove();
					sync_cout << "Succeed to Undo!" << sync_endl;
				}
				else {
					sync_cout << "Error : Cannot Undo!" << sync_endl;
				}
			}
			else {
				pos.DoMove(move);
			}

			// Send to Another Program or Thread
			if (players.pType[!turn] == Players::OtherAI) {
				uint32_t actionU32 = toU32(move);
				fm.SendMsg(&actionU32, sizeof(uint32_t), true);
			}
			else if (players.pType[!turn] == Players::AI) {
				players.pthread[!turn]->SetEnemyMove(move);
			}
		}

		// Game Over
		Observer::GameOver(!pos.GetTurn(), players.isSwap, pos.GetKey(0), pos.GetKifuHash());
		cout << "-------- Game Over! " << (!pos.GetTurn() ? "▼" : "△") << " Win! --------\n";
		cout << SyncCout::IO_LOCK;
		Observer::PrintGameReport(cout);
		cout << SyncCout::IO_UNLOCK;

		// Save Record
		if (Observer::isSaveRecord) {
			if (gameMode == 5) {
				char msg[20];
				fm.RecvMsg(msg, 20, true);
				if (strcmp("Save Report", msg)) {
					sync_cout << "Error : 與另一支程式棋盤不同步" << sync_endl;
					system("pause");
				}
			}
			// Save PlayDetail
			file.open(playDetailPath, ios::app);
			if (file) {
				file << "-------- Game Over! " << COLOR_WORD[!pos.GetTurn()] << " Win! --------\n";
				file << "#" << COLOR_WORD[(gameMode == 5) ^ players.isSwap] << "Player " << (gameMode == 5 ? "2 : " : "1 : ") << AI_VERSION << "\n";
				file << Search::GetSettingStr() << "\n";
				Observer::PrintGameReport(file);
				if (gameMode != 4) {
					pos.PrintKifu(file);
				}
				file.close();
			}
			// Save Kifu
			/*if (gameMode != 4) {
				file.open(kifuStr, ios::app);
				if (file) {
					players.PrintNames(file);
					pos.PrintKifu(file);
					file.close();
				}
				else sync_cout << "Error : Fail to Save Kifu." << sync_endl;
			}*/
			// Save AIReport
			if (gameMode != 5) {
				file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::out);
				if (file) {
					file << "Start at : " << currTimeStr << "\nEnd at   : " << Observer::GetTimeStamp() << "\n\n";
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
			if (gameMode == 4)
				fm.SendMsg("Save Report", 12, true);
		}
	} while (isCustomBoard);

	// Print Total Report;
	cout << SyncCout::IO_LOCK;
	cout << "---------- Total Report ----------\n";
	Observer::PrintWinnerReport(cout);
	Observer::PrintTotalReport(cout);;
	cout << SyncCout::IO_UNLOCK;

	SetConsoleTitle("Minishogi - " AI_VERSION " Stop");
	cout << "\a\a\a";
	system("pause");
	return 0;
}

bool GetOpenFileNameString(string& out) {
	char szFile[MAX_PATH];
	OPENFILENAME ofn;
	char currDir[MAX_PATH];
	char lpstrBuf[] = "選擇對手程式";
	GetCurrentDirectory(MAX_PATH, currDir);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetConsoleWindow();
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "*.exe\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = lpstrBuf;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = currDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	bool isOK = GetOpenFileName(&ofn);
	out.assign(ofn.lpstrFile);
	return isOK;
}

