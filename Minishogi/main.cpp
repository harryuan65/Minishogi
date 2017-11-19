
#include "board.h"
#include"head.h"
void InitializeByInput(Board &b)
{
	string board_str;
	cout << "Give initialize input:";
	getline(cin, board_str);
	b.Initialize(board_str);
}
enum {
	Human,Ai
};
int main()
{
	Board m_Board;
	//Initialize by input
	
	m_Board.Initialize();

	
	int playerturn = Human;
	
	m_Board.PrintChessBoard();
	Action a = 0;
	bool moveok = false;
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
		playerturn ^= 1;
		system("pause");
		system("cls");
		m_Board.PrintChessBoard();
		
	}
	//board_str.clear();
	
	
	return 0;
}