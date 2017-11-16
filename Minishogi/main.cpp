#include<iostream>
#include"head.h"
using namespace std;

int main()
{
	int chessboard[CHESS_BOARD_SIZE] = {BLANK};
	playerboard pboard;
	Initalize(pboard, chessboard);
	PrintChessBoard(chessboard);
	
	
	bool end = false;
	
	while (!end)
	{
		string from,to;
		int pro = 0;
		cout << "²¾°Ê: X#, X#, 0/1"<<endl;
		cin >> from >> to >> pro;

		Human_DoMove(chessboard,pboard,from,to,pro,true);

	}
	return 0;
}