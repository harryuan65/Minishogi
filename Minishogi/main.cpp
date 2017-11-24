#include <bitset>
#include "board.h"
#include"head.h"
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
enum {
	Human,Ai
};

int main() {
	Board m_Board;
    Action action;
	int playerturn = Human;
	int choice;
	//1 2 3 4 5 6 17 18 19 20 21 22 1 2 3 4 5 6 17 18 19 20 21 22 0 0
	cout << "�ۭq�ѽL?0:1 = ";
    while (cin >> choice && choice != 0 && choice != 1);
	if (choice) InitializeByInput(m_Board);

	m_Board.PrintChessBoard();
	while (!m_Board.isGameOver()) {
		if (playerturn == Human) {
			cout << "[�{�b�O�դ�]" << endl;

            /* ���ե� */
            Action moveList[128] = { 0 };
            int cnt = 0;
            MoveGenerator(m_Board, moveList, cnt, playerturn);
            cout << "movable : " << cnt << endl;
            while (!(action = Human_DoMove(m_Board, playerturn)));

            cout << "Action: " << bitset<25>(action) << endl;
            /* ���ե� */
		}
		else {
			cout << "[�{�b�O�¤�]"<< endl;

            /* ���ե� */
            Action moveList[128] = { 0 };
            int cnt = 0;
            MoveGenerator(m_Board, moveList, cnt, playerturn);
            cout << "movable : " << cnt << endl;
			while (!(action = Human_DoMove(m_Board, playerturn)));

            cout << "Action: " << bitset<25>(action) << endl;
            /* ���ե� */

			//AI_DoMove(m_Board, TURN_BLACK);
		}
        m_Board.DoMove(action);
		m_Board.PrintChessBoard();

		cout << "�O�_�nUndoMove?:";
        while (cin >> choice && choice != 0 && choice != 1);

		if (choice) {
			m_Board.UndoMove();
			m_Board.PrintChessBoard();
		}
		else {
			playerturn ^= 1;
		}
	}

	return 0;
}