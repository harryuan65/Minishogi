
#include"head.h"

int main()
{
	Board m_Board;
	string board_str;
	//getline(cin,board_str);
	m_Board.Initialize();
	//m_Board.Initialize(board_str);
	try {
		m_Board.PrintChessBoard();

	}
	catch (...)
	{
		throw;
	}
	//board_str.clear();
	
	
	return 0;
}