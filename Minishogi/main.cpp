// NEED DEBUG : Zobrist Hashing 同構, 同形表的位元轉換, Domove, Undomove
#include "head.h"
U64 nnnn = 0;

int main() {
    Board m_Board;
    Action action;
    int player[2];
    int gameMode;

    for (;;) {
        cout << "請選擇對手:\n(0)玩家vs電腦\n(1)電腦vs玩家\n(2)玩家對打\n(3)電腦對打\n(4)電腦對打 本機vs其他程式\n(5)電腦對打 其他程式vs本機\n";
        cin >> gameMode;
        
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
        case 5:
            player[0] = OTHERAI_CTRL; player[1] = AI_CTRL;
            break;
        default:
            continue;
        }
        break;
    }

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
    else */m_Board.Initialize();

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

        m_Board.PrintChessBoard();

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

        if (player[m_Board.GetTurn()] == HUMAN_CTRL) {
            while (!(action = Human_DoMove(m_Board)));
        }
        else if (player[m_Board.GetTurn()] == AI_CTRL) {
            clock_t begin = clock();
            if (!(action = AI_DoMove(m_Board))) {
                cout << "Checkmate! I'm lose";
                system("pause");
                break;
            }
            double durationTime = double(clock() - begin) / CLOCKS_PER_SEC;
            printf("\a"
                "Total Nodes       : %11llu\n"
                "Failed-High Nodes : %11llu\n"
                "Leave Nodes       : %11llu\n"
                "Time              : %14.2lf\n"
                "Node/s            : %14.2lf\n"
                "Evaluate          : %11d\n"
                "PV leaf Evaluate  : %11d\n"
                "cut illgal move   : %11d\n",
                nodes, failed_nodes, leave_nodes, durationTime,
                (durationTime ? nodes / durationTime : 0),
                m_Board.Evaluate(),
                pvEvaluate, nnnn);

           

            nodes = 0;
            failed_nodes = 0;
            leave_nodes = 0;
            nnnn = 0;
        }
        else {
            cin >> action;
        }

        m_Board.DoMove(action);
    }

    cout << "Game Over!";
    system("pause");
    return 0;
}