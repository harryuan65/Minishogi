#include <fstream>

#include "FileMapping.h"
#include "Thread.h"
#include "Evaluate.h"
#include "Minishogi.h"
#include "Search.h"
#include "Observer.h"
#include "Transposition.h"
#include "GamePlay.h"
using namespace std;
using namespace Observer;

#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"


namespace GamePlay {
	const char selectModeStr[] =
		"請選擇對手:\n"
		"(0)玩家vs電腦\n"
		"(1)電腦vs玩家\n"
		"(2)玩家對打\n"
		"(3)電腦對打\n"
		"(4)電腦對打 本機vs其他程式";
	enum PlayerType : int { Human, AI, OtherAI };
	enum GameMode : int { Human_AI, AI_Human, Human_Human, AI_AI, AI_OtherAI, OtherAI_AI };
	std::istream& operator >> (std::istream &is, GameMode &m) {
		int gm;
		is >> gm;
		m = (GameMode)gm;
		return is;
	}

	PlayerType pType[2];
	string pName[2];
	Thread *pthread[2] = { nullptr };
	GameMode gameMode;
	bool isSwap;
	bool isCustomBoard;
	FileMapping fm;
	string currTimeStr;

	static bool GetOpenFileNameString(string& out) {
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

	void Playing() {
		Minishogi pos(nullptr);
		fstream file;
		streamoff readBoardOffset = 0;
		string playDetailPath;

		do {
			if (isCustomBoard && !pos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
				if ((pType[0] == OtherAI || pType[1] == OtherAI) && !isSwap) { // 先後手交換
					swap(pType[0], pType[1]);
					swap(pName[0], pName[1]);
					swap(pthread[0], pthread[1]);
					isSwap = true;
					sync_cout << "Swap player." << sync_endl;
					readBoardOffset = 0;
					continue;
				}
				else { // 對打結束
					break;
				}
			}
			else if (!isCustomBoard) {
				pos.Initialize();
			}
			if (pType[0] == AI)
				pthread[0] = new Thread(USI::Options["HashEntry"]);
			if (pType[1] == AI)
				pthread[1] = new Thread(USI::Options["HashEntry"]);
			assert(pType[0] != AI || pthread[0]);
			assert(pType[0] != AI || pthread[1]);

			// Write Title
			SetConsoleTitle(("Nyanpass " AI_NAME " - GamePlay : Game " + to_string(gameNum) + " " + to_string(player1WinNum) + ":" + to_string(player2WinNum)).c_str());
			sync_cout << "---------- Game " << gameNum << " ----------" << sync_endl;
			if (isSaveRecord) {
				playDetailPath = REPORT_PATH + currTimeStr + "_PlayDetail_" + to_string(gameNum) + ".txt";
				//kifuStr = REPORT_PATH + currTimeStr + "_Kifu_" + to_string(gameNum) + ".txt";
				if (gameMode != AI_OtherAI) {
					file.open(playDetailPath, ios::app);
					if (file) file << "#▼ : " << pName[1] << "\n#△ : " << pName[0] << "\n";
					file.close();
				}
				else {
					Sleep(10); // 先讓另一邊寫完檔案
				}
			}

			GameStart();
			//if (pType[0] == AI)
			//	pthread[0]->StartGameLoop();
			//if (pType[1] == AI)
			//	pthread[1]->StartGameLoop();
			// Game Loop
			while (true) {
				Color turn = pos.GetTurn();
				Move move;
				// Print & Write ChessBoard
				cout << SyncCout::IO_LOCK;
				pos.PrintChessBoard();
				cout << "Evaluate : " << setw(15) << pos.GetEvaluate() << "\n";
				cout << SyncCout::IO_UNLOCK;
				if (isSaveRecord && pType[turn] != OtherAI) {
					file.open(playDetailPath, ios::app);
					if (file) {
						file << "---------- Game " << gameNum << " Step " << pos.GetPly() << " ----------\n";
						file << pos << "\n";
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
				switch (pType[turn]) {
				case Human:
					while (true) {
						sync_cout << "請輸入移動指令或其他指令(UNDO, SURRENDER) : " << sync_endl;
						string buf;
						cin >> buf;
						move = usi2move(buf, turn);

						if (IsDoMove(move) && 
							!pos.IsLegelAction(move) &&
							!pos.IsLegelAction(make_move(from_sq(move), to_sq(move), true))) {
							sync_cout << "Error : Illegal move." << sync_endl;
							continue;
						}
						else if (!IsDoMove(move) && move != MOVE_NULL && move != MOVE_UNDO) {
							sync_cout << "Error : Unrecognized command." << sync_endl;
							continue;
						}
						break;
					}

					file.open(playDetailPath, ios::app);
					if (file) file << COLOR_WORD[turn] << "DoMove : " << move << "\n";
					file.close();
					break;
				case OtherAI:
					uint32_t actionU32;
					fm.RecvMsg(&actionU32, sizeof(uint32_t), true);
					move = setU32(actionU32);
					sync_cout << COLOR_WORD[turn] << "DoMove : " << move << sync_endl;
					break;
				case AI:
					//RootMove rm = pthread[turn]->GetBestMove();
					cout << endl << SyncCout::IO_LOCK;
					PrintSearchReport(cout);
					cout << SyncCout::IO_UNLOCK;
					file.open(playDetailPath, ios::app);
					if (file) PrintSearchReport(file);
					file.close();
					//move = rm.pv[0];
					sync_cout << COLOR_WORD[turn] << "DoMove : " << move << sync_endl;

					file.open(playDetailPath, ios::app);
					if (file) {
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
					if (pos.GetPly() >= 2) {
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
				if (pType[~turn] == OtherAI) {
					uint32_t actionU32 = toU32(move);
					fm.SendMsg(&actionU32, sizeof(uint32_t), true);
				}
				else if (pType[~turn] == AI) {
					//pthread[~turn]->SetEnemyMove(move);
				}
			}

			// Game Over
			if (pType[0] == AI && pthread[0])
				delete pthread[0];
			if (pType[1] == AI && pthread[1])
				delete pthread[1];
			GameOver(!pos.GetTurn(), isSwap, pos.GetKifuHash());
			cout << "-------- Game Over! " << (!pos.GetTurn() ? "▼" : "△") << " Win! --------\n";
			cout << SyncCout::IO_LOCK;
			PrintGameReport(cout);
			cout << SyncCout::IO_UNLOCK;

			// Save Record
			if (isSaveRecord) {
				if (gameMode == OtherAI_AI) {
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
					file << COLOR_WORD[(gameMode == OtherAI_AI) ^ isSwap] 
						 << "Player " << (gameMode == OtherAI_AI ? "2 : " : "1 : ") << AI_NAME << "\n";
					file << GetSettingStr() << "\n";
					PrintGameReport(file);
					if (gameMode != AI_OtherAI) {
						pos.PrintKifu(file);
					}
					file.close();
				}
				// Save Kifu
				/*if (gameMode != AI_OtherAI) {
					file.open(kifuStr, ios::app);
					if (file) {
						file << "#▼ : " << pName[1] << "\n#△ : " << pName[0] << "\n";
						pos.PrintKifu(file);
						file.close();
					}
					else sync_cout << "Error : Fail to Save Kifu." << sync_endl;
				}*/
				// Save AIReport
				if (gameMode != OtherAI_AI) {
					file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::out);
					if (file) {
						file << "StartGameLoop at : " << currTimeStr << "\nEnd at   : " << GetTimeStamp() << "\n\n";
						PrintWinnerReport(file);
						file << "#" << "Player " << (gameMode == OtherAI_AI ? "2 : " : "1 : ") << AI_NAME << "\n";
						file << GetSettingStr() << "\n";
						PrintTotalReport(file);
						file.close();
					}
					else sync_cout << "Error : Fail to Save AI Report." << sync_endl;
				}
				else {
					file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::app);
					if (file) {
						file << "#" << "Player " << (gameMode == OtherAI_AI ? "2 : " : "1 : ") << AI_NAME << "\n";
						file << GetSettingStr() << "\n";
						PrintTotalReport(file);
						file.close();
					}
					else sync_cout << "Error : Fail to Save AI Report." << sync_endl;
				}
				if (gameMode == AI_OtherAI)
					fm.SendMsg("Save Report", 12, true);
			}
		} while (isCustomBoard);
	}

	void GamePlay(int argc, char **argv) {
		string buf, gameDirStr;

		cout << "AI Version : " << AI_NAME << "\n" << GetSettingStr() << endl;
		SetConsoleTitle("Nyanpass " AI_NAME " - GamePlay");

		if (argc != 4) {
			currTimeStr = GetTimeStamp();
			fm.Open(currTimeStr);
			while (true) {
				cout << selectModeStr << endl;
				cin >> gameMode;
				switch (gameMode) {
				case Human_AI:
					pType[0] = Human;
					pType[1] = AI;
					pName[0] = "Human";
					pName[1] = AI_NAME;
					break;
				case AI_Human:
					pType[0] = AI;
					pType[1] = Human;
					pName[0] = AI_NAME;
					pName[1] = "Human";
					break;
				case Human_Human:
					pType[0] = Human;
					pType[1] = Human;
					pName[0] = "Human";
					pName[1] = "Human";
					break;
				case AI_AI:
					pType[0] = AI;
					pType[1] = AI;
					pName[0] = AI_NAME;
					pName[1] = AI_NAME;
					break;
				case AI_OtherAI:
					if (!GetOpenFileNameString(gameDirStr))
						continue;
					pType[0] = AI;
					pType[1] = OtherAI;
					pName[0] = AI_NAME;
					pName[1] = "";
					break;
				default:
					continue;
				}
				cout << "從board//" << CUSTOM_BOARD_FILE << "讀取多個盤面 並連續對打(y/n)?" << endl;
				cin >> buf;
				isCustomBoard = (buf == "y" || buf == "Y");
				break;
			}
		}
		else {
			// argv[4] = 遊戲路徑 輸出檔名 玩家名 是否自訂棋盤
			gameMode = OtherAI_AI;
			currTimeStr = argv[1];
			fm.Open(currTimeStr);
			pType[0] = OtherAI;
			pType[1] = AI;
			pName[0] = argv[2];
			pName[1] = AI_NAME;
			isCustomBoard = argv[3][0] != '0';
		}

		cout << "確定要開始 不開始則進入進階選項(y/n)?" << endl;
		cin >> buf;
		if (buf != "y" && buf != "Y") {
			if (pType[0] == AI || pType[1] == AI) {
				cout << "輸入時間限制(ms 0為無限制)" << endl;
				cin >> limitTime;
			}

			if (isSaveRecord && (pType[0] == Human || pType[1] == Human)) {
				cout << "輸入對手的名字" << endl;
				cin >> pName[gameMode];
			}

			cout << GetSettingStr() << "確定要開始?" << endl;
			system("pause");
		}
		if (gameMode == AI_OtherAI) {
			fm.SendMsg(nullptr, 0, false);
			// argv[4] = 遊戲路徑 輸出檔名 玩家名 是否自訂棋盤
			system(("start \"\" \"" + gameDirStr + "\" " + currTimeStr + " " +
				"\"" + AI_NAME + "\" " + to_string(isCustomBoard)).c_str());
			char msg[20];
			cout << "等待對方回應..." << endl;
			fm.RecvMsg(msg, 20, true);
			if (strcmp("StartGameLoop Game", msg)) {
				cout << "Error : 連線失敗" << endl;
				system("pause");
			}
		}
		else if (gameMode == OtherAI_AI) {
			fm.SendMsg("StartGameLoop Game", 11, true);
		}

		CreateDirectory(REPORT_PATH, NULL);
		Zobrist::Initialize();

		Playing();

		cout << SyncCout::IO_LOCK;
		cout << "---------- Total Report ----------\n";
		PrintWinnerReport(cout);
		PrintTotalReport(cout);;
		cout << SyncCout::IO_UNLOCK;
		SetConsoleTitle("Nyanpass " AI_NAME " - GamePlay : Stop");
	}
}