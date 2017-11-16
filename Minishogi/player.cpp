#include"head.h"
string invalid_move = "Invalid Move";

int ConvertInput(string position) {
	/*cout << position << " = " <<
	(int)(5 * (position[0] - 'A'))<<"+"<< (int)('5' - position[1]) <<
	" = "<< (int)(5 * (position[0] - 'A') + ('5' - position[1] )) << endl;*/
	return (int)(5 * (position[0] - 'A') + ('5' - position[1]));
}

bool Player::Move(int *chessboard, playerboard &board, std::string from, std::string to, int pro, int isWhiteturn)
{	
	if ((pro < 0 || pro>1) ||
		(from[0] < 'A'&&from[1] > 'F') && (from[1] >= '0'&&from[1] > '9') &&
		(to[0] < 'A'&&to[1] > 'F') && (to[1] < '0'&&to[1] > '9'))
	{
		cout << invalid_move + ":bad input\n";
		return false;
	}
	else
	{

		int from_pos = ConvertInput(from);
		int to_pos = ConvertInput(to);
		if (isWhiteturn)
		{
			switch (chessboard[from_pos])
			{
			case BLANK:
				cout<<" "<<endl;
				break;
			case PAWN: // white pawn
				cout<<"步"<<endl;

				//cout<<"BitScan(wPawn) = "<<BitScan(&(board.chesspiece[w_Pawn]));
				//像是BitScan會回傳15
				break;
			case SILVER: // white silver
				cout<<"銀"<<endl;
				break;
			case GOLD: // white gold
				cout<<"金"<<endl;
				break;
			case BISHOP: // white bishop
				cout<<"角"<<endl;
				break;
			case ROOK: // white rook
				cout<<"飛"<<endl;
				break;
			case KING: // white king
				 //白色字 現在改黑了
				cout<<"玉"<<endl;
				break;
			case ePAWN: // 9 white e_pawn
				cout<<"ㄈ"<<endl;
				break;
			case eSILVER: // 10 white e_silver
				cout<<"全"<<endl;
				break;
			case eBISHOP: // 12 white e_bishop
				cout<<"馬"<<endl;
				break;
			case eROOK: // 13 white e_rook
				cout<<"龍"<<endl;
				break;
			}
		}
		else
		{
			switch (chessboard[from_pos])
			{
			case bPAWN: // 17 black pawn
				cout << "步" << endl;
				break;
			case bSILVER: //18 black silver
				cout << "銀" << endl;
				break;
			case bGOLD: //19 black gold
				cout << "金" << endl;
				break;
			case bBISHOP: //20 black bishop
				cout << "角" << endl;
				break;
			case bROOK: //21 black rook
				cout << "飛" << endl;
				break;
			case bKING: //22 black king
				cout << "王" << endl;
				break;
			case bePAWN: //25 black e_pawn
				cout << "ㄈ" << endl;
				break;
			case beSILVER: //26 black e_silver
				cout << "全" << endl;
				break;
			case beBISHOP: //28 black e_bishop
				cout << "馬" << endl;
				break;
			case beROOK: //29 black e_rook
				cout << "龍" << endl;
				break;
			default:
				break;
			}
		}
	}

	

}


