#pragma once
#include"head.h"
void Initalize(playerboard &board, int *chessboard);
void PrintChessBoard(int *chessboard);
bool Human_DoMove(int *chessboard, playerboard &board, std::string from, std::string to, int pro, int isWhiteturn);
bool AI_DoMove(playerboard &board, int isWhiteturn);