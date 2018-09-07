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

#define CUSTOM_BOARD_FILE "custom_board.txt"
#define REPORT_PATH       "output//"


namespace GamePlay {
	const char selectModeStr[] =
		"�п�ܹ��:\n"
		"(0)���avs�q��\n"
		"(1)�q��vs���a\n"
		"(2)���a�若\n"
		"(3)�q���若\n"
		"(4)�q���若 ����vs��L�{��";
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
		char lpstrBuf[] = "��ܹ��{��";
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
		Minishogi pos;
		fstream file;
		streamoff readBoardOffset = 0;
		string playDetailPath;

		do {
			if (isCustomBoard && !pos.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
				if ((pType[0] == OtherAI || pType[1] == OtherAI) && !isSwap) { // �����洫
					swap(pType[0], pType[1]);
					swap(pName[0], pName[1]);
					swap(pthread[0], pthread[1]);
					isSwap = true;
					sync_cout << "Swap player." << sync_endl;
					readBoardOffset = 0;
					continue;
				}
				else { // �若����
					break;
				}
			}
			else if (!isCustomBoard) {
				pos.Initialize();
			}
			if (pType[0] == AI)
				pthread[0] = new Thread(pos, WHITE, Observer::ttBit);
			if (pType[1] == AI)
				pthread[1] = new Thread(pos, BLACK, Observer::ttBit);
			assert(pType[0] != AI || pthread[0]);
			assert(pType[0] != AI || pthread[1]);

			// Write Title
			SetConsoleTitle(("Nyanpass " AI_VERSION " - GamePlay : Game " + to_string(Observer::gameNum)).c_str());
			sync_cout << "---------- Game " << Observer::gameNum << " ----------" << sync_endl;
			if (Observer::isSaveRecord) {
				playDetailPath = REPORT_PATH + currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt";
				//kifuStr = REPORT_PATH + currTimeStr + "_Kifu_" + to_string(Observer::gameNum) + ".txt";
				if (gameMode != AI_OtherAI) {
					file.open(playDetailPath, ios::app);
					if (file) file << "#�� : " << pName[1] << "\n#�� : " << pName[0] << "\n";
					file.close();
				}
				else {
					Sleep(10); // �����t�@��g���ɮ�
				}
			}

			Observer::GameStart();
			if (pType[0] == AI)
				pthread[0]->Start();
			if (pType[1] == AI)
				pthread[1]->Start();
			// Game Loop
			while (true) {
				Color turn = pos.GetTurn();
				Move move;
				// Print & Write ChessBoard
				cout << SyncCout::IO_LOCK;
				pos.PrintChessBoard();
				cout << "Evaluate : " << setw(15) << pos.GetEvaluate() << "\n";
				cout << SyncCout::IO_UNLOCK;
				if (Observer::isSaveRecord && pType[turn] != OtherAI) {
					file.open(playDetailPath, ios::app);
					if (file) {
						file << "---------- Game " << Observer::gameNum << " Step " << pos.GetPly() << " ----------\n";
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
				switch (pType[turn]) {
				case Human:
					while (true) {
						sync_cout << "�п�J���ʫ��O�Ψ�L���O(UNDO, SURRENDER) : " << sync_endl;
						string buf;
						cin >> buf;
						move = string2move(buf, turn);

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
					RootMove rm = pthread[turn]->GetBestMove();
					cout << endl << SyncCout::IO_LOCK;
					Observer::PrintSearchReport(cout);
					cout << SyncCout::IO_UNLOCK;
					file.open(playDetailPath, ios::app);
					if (file) Observer::PrintSearchReport(file);
					file.close();
					move = rm.pv[0];
					sync_cout << COLOR_WORD[turn] << "DoMove : " << move << sync_endl;

					file.open(playDetailPath, ios::app);
					if (file) {
						pthread[turn]->Dump(file);
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
					pthread[~turn]->SetEnemyMove(move);
				}
			}

			// Game Over
			if (pType[0] == AI && pthread[0])
				delete pthread[0];
			if (pType[1] == AI && pthread[1])
				delete pthread[1];
			Observer::GameOver(!pos.GetTurn(), isSwap, pos.GetKey(0), pos.GetKifuHash());
			cout << "-------- Game Over! " << (!pos.GetTurn() ? "��" : "��") << " Win! --------\n";
			cout << SyncCout::IO_LOCK;
			Observer::PrintGameReport(cout);
			cout << SyncCout::IO_UNLOCK;

			// Save Record
			if (Observer::isSaveRecord) {
				if (gameMode == OtherAI_AI) {
					char msg[20];
					fm.RecvMsg(msg, 20, true);
					if (strcmp("Save Report", msg)) {
						sync_cout << "Error : �P�t�@��{���ѽL���P�B" << sync_endl;
						system("pause");
					}
				}
				// Save PlayDetail
				file.open(playDetailPath, ios::app);
				if (file) {
					file << "-------- Game Over! " << COLOR_WORD[!pos.GetTurn()] << " Win! --------\n";
					file << COLOR_WORD[(gameMode == OtherAI_AI) ^ isSwap] 
						 << "Player " << (gameMode == OtherAI_AI ? "2 : " : "1 : ") << AI_VERSION << "\n";
					file << Observer::GetSettingStr() << "\n";
					Observer::PrintGameReport(file);
					if (gameMode != AI_OtherAI) {
						pos.PrintKifu(file);
					}
					file.close();
				}
				// Save Kifu
				/*if (gameMode != AI_OtherAI) {
					file.open(kifuStr, ios::app);
					if (file) {
						file << "#�� : " << pName[1] << "\n#�� : " << pName[0] << "\n";
						pos.PrintKifu(file);
						file.close();
					}
					else sync_cout << "Error : Fail to Save Kifu." << sync_endl;
				}*/
				// Save AIReport
				if (gameMode != OtherAI_AI) {
					file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::out);
					if (file) {
						file << "Start at : " << currTimeStr << "\nEnd at   : " << Observer::GetTimeStamp() << "\n\n";
						Observer::PrintWinnerReport(file);
						file << "#" << "Player " << (gameMode == OtherAI_AI ? "2 : " : "1 : ") << AI_VERSION << "\n";
						file << Observer::GetSettingStr() << "\n";
						Observer::PrintTotalReport(file);
						file.close();
					}
					else sync_cout << "Error : Fail to Save AI Report." << sync_endl;
				}
				else {
					file.open(REPORT_PATH + currTimeStr + "_AIReport_AI.txt", ios::app);
					if (file) {
						file << "#" << "Player " << (gameMode == OtherAI_AI ? "2 : " : "1 : ") << AI_VERSION << "\n";
						file << Observer::GetSettingStr() << "\n";
						Observer::PrintTotalReport(file);
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

		cout << "AI Version : " << AI_VERSION << "\n" << Observer::GetSettingStr() << endl;
		SetConsoleTitle("Nyanpass " AI_VERSION " - GamePlay");

		if (argc != 4) {
			currTimeStr = Observer::GetTimeStamp();
			fm.Open(currTimeStr);
			while (true) {
				cout << selectModeStr << endl;
				cin >> gameMode;
				switch (gameMode) {
				case Human_AI:
					pType[0] = Human;
					pType[1] = AI;
					pName[0] = "Human";
					pName[1] = AI_VERSION;
					break;
				case AI_Human:
					pType[0] = AI;
					pType[1] = Human;
					pName[0] = AI_VERSION;
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
					pName[0] = AI_VERSION;
					pName[1] = AI_VERSION;
					break;
				case AI_OtherAI:
					if (!GetOpenFileNameString(gameDirStr))
						continue;
					pType[0] = AI;
					pType[1] = OtherAI;
					pName[0] = AI_VERSION;
					pName[1] = "";
					break;
				default:
					continue;
				}
				cout << "�qboard//" << CUSTOM_BOARD_FILE << "Ū���h�ӽL�� �ós��若(y/n)?" << endl;
				cin >> buf;
				isCustomBoard = (buf == "y" || buf == "Y");
				break;
			}
		}
		else {
			// argv[4] = �C�����| ��X�ɦW ���a�W �O�_�ۭq�ѽL
			gameMode = OtherAI_AI;
			currTimeStr = argv[1];
			fm.Open(currTimeStr);
			pType[0] = OtherAI;
			pType[1] = AI;
			pName[0] = argv[2];
			pName[1] = AI_VERSION;
			isCustomBoard = argv[3][0] != '0';
		}

		if (pType[0] == AI || pType[1] == AI) {
			cout << "��J�j�M���`��" << endl;
			cin >> Observer::depth;

#ifndef KPPT_DISABLE
			cout << "��JKPP�W��" << endl;
			cin >> Observer::kpptName;
			if (!Evaluate::evaluater.Load(Observer::kpptName)) {
				Observer::kpptName = "";
				Evaluate::evaluater.Clean();
			}
#else
			Evaluate::evaluater.Clean();
#endif
		}

		cout << "�T�w�n�}�l ���}�l�h�i�J�i���ﶵ(y/n)?" << endl;
		cin >> buf;
		if (buf != "y" && buf != "Y") {
			if (pType[0] == AI || pType[1] == AI) {
#ifndef TRANSPOSITION_DISABLE
				int size;
				cout << "��J�P����entry�ƶq(2^n)" << endl;
				cin >> size;
				Observer::ttBit = size;
#endif

				cout << "��J�ɶ�����(ms 0���L����)" << endl;
				cin >> Observer::limitTime;
			}

			if (Observer::isSaveRecord && (pType[0] == Human || pType[1] == Human)) {
				cout << "��J��⪺�W�r" << endl;
				cin >> pName[gameMode];
			}

			cout << Observer::GetSettingStr() << "�T�w�n�}�l?" << endl;
			system("pause");
		}
		if (gameMode == AI_OtherAI) {
			fm.SendMsg(nullptr, 0, false);
			// argv[4] = �C�����| ��X�ɦW ���a�W �O�_�ۭq�ѽL
			system(("start \"\" \"" + gameDirStr + "\" " + currTimeStr + " " +
				"\"" + AI_VERSION + "\" " + to_string(isCustomBoard)).c_str());
			char msg[20];
			cout << "���ݹ��^��..." << endl;
			fm.RecvMsg(msg, 20, true);
			if (strcmp("Start Game", msg)) {
				cout << "Error : �s�u����" << endl;
				system("pause");
			}
		}
		else if (gameMode == OtherAI_AI) {
			fm.SendMsg("Start Game", 11, true);
		}

		CreateDirectory(REPORT_PATH, NULL);
		Zobrist::Initialize();

		Playing();

		cout << SyncCout::IO_LOCK;
		cout << "---------- Total Report ----------\n";
		Observer::PrintWinnerReport(cout);
		Observer::PrintTotalReport(cout);;
		cout << SyncCout::IO_UNLOCK;
		SetConsoleTitle("Nyanpass " AI_VERSION " - GamePlay : Stop");
	}
}