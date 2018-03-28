#ifndef _MINISHOGI_
#define _MINISHOGI_

#include <atlstr.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>

#include "Action.h"
#include "Bitboard.h"
#include "Chess.h"
#include "Zobrist.h"

#define BOARD_PATH        "board//"

class Minishogi {
private:
    bool m_turn;
	int m_step;
	int m_evaluate;
	Zobrist::Zobrist m_initHashcode;
	Zobrist::Zobrist m_hashcode;
	Zobrist::Zobrist m_hashcode2;
	// recordZobrist[0] 經過DoMove(recordAction[0]) 變成recordZobrist[1]
	Action recordAction[1000];
	Zobrist::Zobrist recordZobrist[1001];
	Zobrist::Zobrist recordZobrist2[1001];
	bool checkstate[1001];

public:
	static const int BOARD_SIZE = 25;
	static const int TOTAL_BOARD_SIZE = 35;
	static const int SINGLE_GENE_MAX_ACTIONS = 112;
	static const int TOTAL_GENE_MAX_ACTIONS = 162; // AtkGene 21, MoveGene 29, HandGene 112

	Bitboard occupied[2];
	Bitboard bitboard[32];
    int board[TOTAL_BOARD_SIZE];

    void Initialize();
	// 輸入格式: 前25個輸入棋子ID [後10個輸入手排個數0~2(可選)]
    void Initialize(const char *str);
	bool Initialize(const string* str);

	void DoMove(Action &action);
	void UndoMove();
	void AttackGenerator(Action *movelist, int &start) const;
	void MoveGenerator(Action *movelist, int &start) const;
	void HandGenerator(Action *movelist, int &start);
	inline Bitboard Movable(const int srcIndex) const;
	inline Bitboard RookMovable(const int srcIndex) const;
	inline Bitboard BishopMovable(const int srcIndex) const;

	void PrintChessBoard() const;
	void PrintNoncolorBoard(ostream &os) const;
	/*
	第一行 +代表白方先 -代表黑方先
	第二~六行 5*5的棋盤 旗子符號請見Chess.h的SAVE_CHESS_WORD
	第七~八行 ▼0步0銀0金0角0飛 △0步0銀0金0角0飛
	*/
	bool SaveBoard(string filename) const;
    bool LoadBoard(string filename, streamoff &offset);
	void PrintKifu(ostream &os) const;

	bool IsGameOver();
	bool IsLegelAction(Action action);
	inline bool IsChecked() const { return checkstate[m_step]; }
	bool IsCheckAfter(const int srcIndex, const int dstIndex);
	// 如果現在盤面曾經出現過 且距離為偶數(同個人) 判定為千日手 需要先DoMove後才能判斷 在此不考慮被連將
	bool IsSennichite() const;
	inline bool IsIsomorphic() const { return m_hashcode == m_hashcode2; }
    inline int GetTurn() const { return m_turn; }
	inline int GetStep() const { return m_step; }
	inline int GetEvaluate() const { return m_turn ? m_evaluate : -m_evaluate; };
	inline Zobrist::Zobrist GetZobristHash() const { return m_turn ? m_hashcode2 : m_hashcode; }
	inline Zobrist::Zobrist GetInitZobristHash() const { return m_initHashcode; }
	unsigned int GetKifuHash() const ;
};

//+Example Board
//▼飛▼角▼銀▼金▼玉
// ．  ．  ．  ． ▼步
// ．  ．  ．  ．  ． 
//△步 ．  ．  ．  ． 
//△王△金△銀△角△飛
//▼0步0銀0金0角0飛
//△0步0銀0金0角0飛

#endif