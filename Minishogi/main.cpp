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
    cout << "自訂棋盤?0:1 = ";
    while (cin >> choice && choice != 0 && choice != 1);
    if (choice) InitializeByInput(m_Board);

    m_Board.PrintChessBoard();
    while (!m_Board.isGameOver()) {
        Action moveList[128] = { 0 };
        int cnt = 0;
        MoveGenerator(m_Board, playerturn, moveList, cnt);
        cout << "Move cnt : " << cnt << "\n";
        cnt = 0;
        HandGenerator(m_Board, playerturn, moveList, cnt);
        cout << "Hand cnt : " << cnt << "\n";

        /*for (int i = 0; i < cnt; i++) {
            m_Board.DoMove(moveList[i]);
            cout << "The " << i << " position";
            m_Board.PrintChessBoard();
            m_Board.UndoMove();
        }

        if (cnt) {
            cout << "請輸入選擇: ";
            while (cin >> choice && 0 > choice && choice >= cnt);
            m_Board.DoMove(moveList[choice]);
        }*/

        if (playerturn == Human) {
            cout << "[現在是白方]" << endl;
            while (!(action = Human_DoMove(m_Board, playerturn)));
        }
        else {
            cout << "[現在是黑方]"<< endl;
            while (!(action = Human_DoMove(m_Board, playerturn)));

            //AI_DoMove(m_Board, TURN_BLACK);
        }
        m_Board.DoMove(action);
        m_Board.PrintChessBoard();

        playerturn ^= 1;
    }

    return 0;
}