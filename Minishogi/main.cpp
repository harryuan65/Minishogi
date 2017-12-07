// NEED DEBUG : Zobrist Hashing �P�c, �P�Ϊ��줸�ഫ, Domove, Undomove
#include "head.h"
U64 nnnn = 0;

int main() {
    Board m_Board;
    Action action;
    int player[2];
    int gameMode;

    for (;;) {
        cout << "�п�ܹ��:\n(0)���avs�q��\n(1)�q��vs���a\n(2)���a�若\n(3)�q���若\n(4)�q���若 ����vs��L�{��\n(5)�q���若 ��L�{��vs����\n";
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
    cout << "�ۭq�ѽL?0:1 = ";
    while (cin >> choice && choice != 0 && choice != 1);
    if (choice) {
        cout << "�¨B 17 | �»� 18 | �ª� 19 | �¨� 20 | �¨� 21 | �¤� 22" << endl;
        cout << "�ըB  1 | �ջ�  2 | �ժ�  3 | �ը�  4 | �ը�  5 | �դ�  6" << endl << endl;
        cout << "���� + 8" << endl;
        cout << "���: �B | �� | �� | �� | �� �����ǿ�J 0~2 (�i��)" << endl;
        cout << "�п�J 25 �ӼƦr (+ �¥� 10 �Ӥ��)�A�åH '.' ���� :" << endl;
        char s[128];
        cin.clear(); cin.ignore(128, '\n');
        cin.getline(s, 128, '.'); // ����y�I�����A�ҥH�i�H�_��
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

        /*cout << "�O�_���� ? : (�_: 0 / �s: 1 / Ū : 2)\n";
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