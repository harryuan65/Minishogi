// NEED DEBUG : Zobrist Hashing 同構, 同形表的位元轉換, Domove, Undomove
#include "head.h"

void InitializeByInput(Board &b) {
    cin.clear();
    cin.ignore();
    string board_str;
    board_str.clear();
    cout << "Give initialize input:";
    getline(cin, board_str);
    while (!b.Initialize(board_str)) {
        cout << "Give initialize input:";
        board_str.clear();
        getline(cin, board_str);

    };
}

int main() {
    Board m_Board;
    Action action;
    int playerturn = 0;
	int player[2];
	int gameMode;
	HWND hwnd;

	for (;;) {
		cout << "請選擇對手:\n(0)玩家vs電腦\n(1)電腦vs玩家\n(2)玩家對打\n(3)電腦對打\n(4)電腦對打 本機vs其他程式\n(5)電腦對打 其他程式vs本機\n";
		cin >> gameMode;
		if (gameMode == 4 || gameMode == 5) {
			cout << "你的hwnd是 " << (int)GetConsoleWindow() << endl;
			cout << "請輸入目標程式的hwnd : ";
			scanf("%d", &hwnd);
		}
		switch (gameMode) {
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
		case 5:
			player[0] = OTHERAI_CTRL; player[1] = AI_CTRL;
			break;
		default:
			continue;
		}
		break;
	}

    int choice = 0;
    //cout << "是否使用自訂棋盤? : (0/1)\n";
    //while (cin >> choice && choice != 0 && choice != 1);
    if (choice) InitializeByInput(m_Board);

    while (!m_Board.IsGameOver()) {
        Action moveList[128] = { 0 };
		/* Debug : Move gene */
        /*int cnt = 0;
        MoveGenerator(m_Board, playerturn, moveList, cnt);
        cout << "Move cnt : " << cnt << "\n";
        cnt = 0;
        HandGenerator(m_Board, playerturn, moveList, cnt);
        cout << "Hand cnt : " << cnt << "\n";*/

		m_Board.PrintChessBoard(playerturn);
        if (player[playerturn] == HUMAN_CTRL) {
            while (!(action = Human_DoMove(m_Board, playerturn)));
        }
        else if (player[playerturn] == AI_CTRL) {
			if (!(action = AI_DoMove(m_Board, playerturn))) {
				cout << "Error : AI can't gene a action";
				system("pause");
			}
			cout << "AI Action : " << (ACTION_TO_SRCINDEX(action)) << " " << (ACTION_TO_DSTINDEX(action)) << (ACTION_TO_ISEXACT(action) ? "+" : "") << endl;
			/*if (gameMode == 4 || gameMode == 5) {
				stringstream ss;
				ss << action;
				for (int i = 0; i < ss.str().length(); i++) {
					PostMessage(hwnd, WM_KEYDOWN, ss.str()[i], 0);
				}
				PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
			}*/
			//cout << "\a" << endl; //びー
        }
		/*else {
			cin >> action;
		}*/
        m_Board.DoMove(action);

		/* Debug : Undo */
		/*cout << "是否Undo? : (0/1)\n";
		while (cin >> choice && choice != 0 && choice != 1);
		if (choice) {
			m_Board.UndoMove();
			continue;
		}*/

        playerturn ^= 1;
    }
	m_Board.PrintChessBoard(0);
	cout << "Game Over!" << endl;
	system("pause");
    return 0;
}