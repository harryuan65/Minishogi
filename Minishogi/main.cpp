// NEED DEBUG : Zobrist Hashing �P�c, �P�Ϊ��줸�ഫ, Domove, Undomove
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
		cout << "�п�ܹ��:\n(0)���avs�q��\n(1)�q��vs���a\n(2)���a�若\n(3)�q���若\n(4)�q���若 ����vs��L�{��\n(5)�q���若 ��L�{��vs����\n";
		cin >> gameMode;
		if (gameMode == 4 || gameMode == 5) {
			cout << "�A��hwnd�O " << (int)GetConsoleWindow() << endl;
			cout << "�п�J�ؼе{����hwnd : ";
			scanf("%d", &hwnd);
		}
		switch (gameMode)
		{
		case 0: case 5:
			player[0] = HUMAN_CTRL; player[1] = AI_CTRL;
			break;
		case 1: case 4:
			player[0] = AI_CTRL; player[1] = HUMAN_CTRL;
			break;
		case 2:
			player[0] = HUMAN_CTRL; player[1] = HUMAN_CTRL;
			break;
		case 3:
			player[0] = AI_CTRL; player[1] = AI_CTRL;
			break;
		default:
			continue;
		}
		break;
	}

    int choice;
    cout << "�O�_�ϥΦۭq�ѽL? : (0/1)\n";
    while (cin >> choice && choice != 0 && choice != 1);
    if (choice) InitializeByInput(m_Board);

    while (!m_Board.IsGameOver()) {
        Action moveList[128] = { 0 };
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
        else {
			if (!(action = AI_DoMove(m_Board, playerturn))) {
				cout << "Error : AI can't gene a action";
				system("pause");
			}
        }
        m_Board.DoMove(action);

		cout << "�O�_Undo? : (0/1)\n";
		while (cin >> choice && choice != 0 && choice != 1);
		if (choice) {
			m_Board.UndoMove();
			continue;
		}

        playerturn ^= 1;
    }

	cout << "Game Over!";
	system("pause");
    return 0;
}