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
    /*int choice;
    cout << "¦Û­q´Ñ½L?0:1 = ";
    while (cin >> choice && choice != 0 && choice != 1);
    if (choice) InitializeByInput(m_Board);*/

    while (!m_Board.isGameOver()) {
		m_Board.PrintChessBoard(playerturn);
        Action moveList[128] = { 0 };
        /*int cnt = 0;
        MoveGenerator(m_Board, playerturn, moveList, cnt);
        cout << "Move cnt : " << cnt << "\n";
        cnt = 0;
        HandGenerator(m_Board, playerturn, moveList, cnt);
        cout << "Hand cnt : " << cnt << "\n";*/

        if (playerturn == Human) {
            while (!(action = Human_DoMove(m_Board, playerturn)));
        }
        else {
            while (!(action = Human_DoMove(m_Board, playerturn)));

            //AI_DoMove(m_Board, TURN_BLACK);
        }
        m_Board.DoMove(action);

        playerturn ^= 1;
    }

    return 0;
}