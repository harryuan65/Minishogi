
#ifndef _PLAYER
#define _PLAYER
#include"head.h"
class Player {
public :
	bool Move(int *chessboard,playerboard &board,std::string from, std::string to,int pro,int isWhiteturn) ;


};
int ConvertInput(std::string position);



#endif // !_PLAYER
