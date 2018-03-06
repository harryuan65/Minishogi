#ifndef _MINISHOGI_
#define _MINISHOGI_
#define WINDOWS_10

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
#include "Observer.h"
#include "Zobrist.h"

#define BOARD_PATH   "board//"
#define KIFU_PATH    "output//"

class Minishogi {
private:
    bool m_turn;
	int m_step;
	int m_evaluate;
	Zobrist::Zobrist m_hashcode;
	Zobrist::Zobrist m_hashcode2;
	// recordZobrist[0] �g�LDoMove(recordAction[0]) �ܦ�recordZobrist[1]
	Action recordAction[120];
	Zobrist::Zobrist recordZobrist[121];
	Zobrist::Zobrist recordZobrist2[121];

public:
	static const int BOARD_SIZE = 25;
	static const int TOTAL_BOARD_SIZE = 35;
	static const int SINGLE_GENE_MAX_ACTIONS = 112;
	static const int TOTAL_GENE_MAX_ACTIONS = 162; // AtkGene 21, MoveGene 29, HandGene 112

	Bitboard occupied[2];
	Bitboard bitboard[32];
    int board[TOTAL_BOARD_SIZE];

    void Initialize();
	// ��J�榡: �e25�ӿ�J�ѤlID [��10�ӿ�J��ƭӼ�0~2(�i��)]
    void Initialize(const char *str);

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
    bool SaveBoard(string filename, string comment) const;
	/*
	�Ĥ@�� +�N��դ�� -�N��¤��
	�ĤG~���� 5*5���ѽL �X�l�Ÿ��Ш�Chess.h��SAVE_CHESS_WORD
	�ĤC~�K�� ��0�B0��0��0��0�� ��0�B0��0��0��0��
	*/
    bool LoadBoard(string filename, streamoff &offset);
	bool SaveKifu(string filename) const;

	bool IsGameOver();
	bool IsLegelAction(Action action);
	bool IsCheckAfter(const int srcIndex, const int dstIndex);
	// �p�G�{�b�L�����g�X�{�L �B�Z��������(�P�ӤH) �P�w���d��� �ݭn��DoMove��~��P�_ �b�����Ҽ{�Q�s�N
	bool IsSennichite() const;
    inline int GetTurn() const { return m_turn; }
	inline int GetStep() const { return m_step; }
	inline int GetEvaluate() const { return m_turn ? m_evaluate : -m_evaluate; };
	inline Zobrist::Zobrist GetZobristHash() const { return m_turn ? m_hashcode2 : m_hashcode; }
	unsigned int GetKifuHash() const ;
};

//+Example Board
//�����������ȡ�������
// �D  �D  �D  �D ���B
// �D  �D  �D  �D  �D 
//���B �D  �D  �D  �D 
//�����������ȡ�������
//��0�B0��0��0��0��
//��0�B0��0��0��0��

#endif