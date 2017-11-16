#include<iostream>
#include"head.h"
using namespace std;

int main()
{
	Board m_Board;
	//Initalize(pboard, chessboard);
	//PrintChessBoard(chessboard);
	
	
	bool end = false;
	string board_str;
	getline(cin,board_str);
	m_Board.Initialize(board_str);
	board_str.clear();

	while (!m_Board.isGameOver())
	{
		
		/*string from,to;
		int pro = 0;
		cout << "²¾°Ê: X#, X#, 0/1"<<endl;
		cin >> from >> to >> pro;*/
		//Human_DoMove(chessboard,pboard,from,to,pro,true);

	}
	return 0;
}