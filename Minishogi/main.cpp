/*
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
using namespace std;

Action Human_DoMove(Board &board);
Action AI_DoMove(Board &board, PV &pv);

bool SavePlayDetail(const string filename, const string comment, Board &board, Action action, PV &pv);
bool SaveAIReport(const string filename, const string comment);

int main() {
    Board m_Board;
    Action action;
    int player[2];
    int gameMode;
	bool isCustomBoard;
	string currTimeStr;
	streamoff readBoardOffset = 0;

	/*    取得現在時間字串    */
	char buffer[80];
	time_t rawtime;
	time(&rawtime);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H-%M-%S", localtime(&rawtime));
	currTimeStr = buffer;

    for (;;) {
        cout << "請選擇對手:\n(0)玩家vs電腦\n(1)電腦vs玩家\n(2)玩家對打\n(3)電腦對打\n(4)電腦對打 本機vs其他程式\n"; //(5)電腦對打 其他程式vs本機\n
		gameMode = getchar() - '0';
        switch (gameMode)
        {
        case 0:
            player[0] = HUMAN_CTRL; player[1] = AI_CTRL;
            break;
        case 1:
            player[0] = AI_CTRL; player[1] = HUMAN_CTRL;
            break;
        case 2:
            player[0] = HUMAN_CTRL; player[1] = HUMAN_CTRL;
            break;
        case 3:
            player[0] = AI_CTRL; player[1] = AI_CTRL;
            break;
        case 4:
            player[0] = AI_CTRL; player[1] = OTHERAI_CTRL;
            break;
        /*case 5:
            player[0] = OTHERAI_CTRL; player[1] = AI_CTRL;
            break;*/
        default:
            continue;
        }
        break;
    }
	cin.ignore();
	
	if (player[0] == AI_CTRL || player[1] == AI_CTRL) {
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
	cout << "確定要開始? ";
	system("pause");

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
	if (player[0] == AI_CTRL || player[1] == AI_CTRL) {
		InitializeTP();
	}
	do {
		m_Board.Initialize();
		if (isCustomBoard) {
			if (!m_Board.LoadBoard(CUSTOM_BOARD_FILE, readBoardOffset)) {
				break;
			}
		}
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
			if (player[m_Board.GetTurn()] == HUMAN_CTRL) {
				while (!(action = Human_DoMove(m_Board)));
			}
			else if (player[m_Board.GetTurn()] == AI_CTRL) {
				Observer::StartSearching();
				action = AI_DoMove(m_Board, pv);
				Observer::EndSearching();
				cout << endl;

				pv.Print(cout, m_Board.GetTurn());
				Observer::PrintReport(cout);
				//if (Observer::searchTime > 60) cout << "\a"; //很吵
			}
			else {
				cin >> action;
			}

			if (Observer::isSaveRecord) {
				if (isCustomBoard) {
					SavePlayDetail(currTimeStr + "_PlayDetail_" + to_string(Observer::gameNum) + ".txt", currTimeStr, m_Board, action, pv);
				}
				else {
					SavePlayDetail(currTimeStr + "_PlayDetail.txt", currTimeStr, m_Board, action, pv);
				}
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
		Observer::GameOver(!m_Board.GetTurn(), m_Board.GetKifuHash());
		cout << "-------- Game Over! " << (m_Board.GetTurn() ? "△" : "▼") << " Win! --------\n\n";
		if (Observer::isSaveRecord) {
			if (isCustomBoard) {
				m_Board.SaveKifu(currTimeStr + "_Kifu_" + to_string(Observer::gameNum - 1) + ".txt", currTimeStr);
			}
			else {
				m_Board.SaveKifu(currTimeStr + "_Kifu.txt", currTimeStr);
			}
		}
		if (Observer::isSaveRecord) {
			SaveAIReport(currTimeStr + "_AiReport.txt", currTimeStr);
		}
		cout << "\n";
		Observer::PrintGameReport(cout);
	} while (isCustomBoard);
	Observer::PrintObserverReport(cout);

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

bool SavePlayDetail(const string filename, const string comment, Board &board, Action action, PV &pv) {
	string filepath = REPORT_PATH + filename;
	fstream file(filepath, ios::out | ios::app);
	if (!file) {
		CreateDirectory(LREPORT_PATH, NULL);
		file.open(filepath, ios::out | ios::app);
	}
	if (file) {
		if (board.GetStep() == 0) {
			file << "#" << comment << "\n";
			file << "Zobrist Table Seed : " << Zobrist::SEED << "\n";
		}
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

bool SaveAIReport(const string filename, const string comment) {
	if (Observer::searchNum == 0)
		return false;
	string filepath = REPORT_PATH + filename;
	fstream file(filepath, ios::app);
	if (!file) {
		CreateDirectory(LREPORT_PATH, NULL);
		file.open(filepath);
	}
	if (file) {
		if (Observer::gameNum <= 1) {
			file << "#" << comment << "\n";
			file << "Zobrist Table Seed : " << Zobrist::SEED << "\n";
		}
		else {
			file << "\n";
		}
		Observer::PrintGameReport(file);
		file.close();
		cout << "Success Save AI Report to " << filepath << "\n";
		return true;
	}
	cout << "Fail Save AI Report to " << filepath << "\n";
	return false;
}