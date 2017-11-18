
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

	
	bool playerturn = Human;
	
	m_Board.PrintChessBoard();
	Action a = 0;
	
	while (!m_Board.isGameOver())
	{
		if (playerturn == Human) {
			
			Human_DoMove(m_Board, TURN_WHITE);
		}
		else
		{
			AI_DoMove(m_Board, TURN_BLACK);
		}
		system("pause");
		system("cls");
		m_Board.PrintChessBoard();
		
	}
	//board_str.clear();
	
	
	return 0;
}