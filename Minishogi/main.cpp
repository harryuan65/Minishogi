
#include "board.h"
#include"head.h"
void InitializeByInput(Board &b)
{
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
int main()
{
	Board m_Board;
	int playerturn = Human;
	bool moveok = false;
	int def = -1;
	//1 2 3 4 5 6 17 18 19 20 21 22 1 2 3 4 5 6 17 18 19 20 21 22 0 0
	cout << "自訂棋盤?0:1 = ";
	while (def != 0&&def != 1)	{
		cin >> def;
	}
	if (def == 0)
	{
		m_Board.Initialize();
	}
	else
	{
		InitializeByInput(m_Board);
	}


	m_Board.PrintChessBoard();
	while (!m_Board.isGameOver())
	{
		if (playerturn == Human) {
			
			cout << "[現在是白方]" << endl;
			while (!moveok) {
				moveok = Human_DoMove(m_Board, TURN_WHITE);
			};
			moveok = !moveok;
				
		}
		else
		{
			cout << "[現在是黑方]"<< endl;
			while (!moveok) {
				moveok = Human_DoMove(m_Board, TURN_BLACK);
			};
			moveok = !moveok;
			//AI_DoMove(m_Board, TURN_BLACK);
		}

		system("pause");
		system("cls");
		m_Board.PrintChessBoard();


		cout << "是否要UndoMove?:";
		int c = -1;
		while (c != 0 && c != 1)
		{
			cin >> c;
		}
		if (c == 1)
		{
			m_Board.UndoMove();
			system("pause");
			system("cls");
			m_Board.PrintChessBoard();
		}

		playerturn ^= 1;

	}
	//board_str.clear();
	
	
	return 0;
}