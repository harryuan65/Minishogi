
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

	string from, to;
	int pro;
	bool playerturn = Human;
	
	m_Board.PrintChessBoard();

	while (!m_Board.isGameOver())
	{
		if (playerturn == Human) {
			cout << "Input X# X# 0/1:";
			cin >> from >> to >> pro;
			if (pro == 0) break;
			Human_DoMove(m_Board, from, to, pro, TURN_WHITE);
		}
		else
		{
			AI_DoMove(m_Board, TURN_BLACK);
		}
		
	}
	//board_str.clear();
	
	
	return 0;
}