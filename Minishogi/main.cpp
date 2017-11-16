#include<iostream>
#include"head.h"
using namespace std;

int main()
{
	int chessboard[CHESS_BOARD_SIZE] = {BLANK};
	playerboard pboard;
	Player p1;
	AI a1;
	Initalize(pboard, chessboard);
	PrintChessBoard(chessboard);
	
	
	bool end = false;
	
	while (!end)
	{
		string from,to;
		int pro = 0;
		cout << "²¾°Ê: X#, X#, 0/1"<<endl;
		cin >> from >> to >> pro;

		p1.Move(chessboard,pboard,from,to,pro,true);

	}
	return 0;
}