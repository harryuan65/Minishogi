/*
+Example Board
�����������ȡ�������
�D  �D  �D  �D ���B
�D  �D  �D  �D  �D
���B  �D  �D  �D  �D
�����������ȡ�������
��0�B0��0��0��0��
��0�B0��0��0��0��
*/
#ifndef _BOARD_
#define _BOARD_
#include "head.h"

#define BOARD_PATH   "board//"
#define KIFU_PATH    "output//"

class Board {
private:
    bool m_turn;
	int m_step;
	int m_evaluate;
	Zobrist::Zobrist m_hashcode;
	Zobrist::Zobrist m_hashcode2;

	//��i��recordAction�ʧ��᪺Zobrist�x�s�b��i+1��recordZobrist
	//recordZobrist��0�ӬO��l�L��
	Action recordAction[120];
	Zobrist::Zobrist recordZobrist[121];
	Zobrist::Zobrist recordZobrist2[121];

public:
    U32 occupied[2];
    U32 bitboard[32];
    int board[TOTAL_BOARD_SIZE];

    void Initialize();
    void Initialize(const char *str);
    void PrintChessBoard() const;
	void PrintNoncolorBoard(ostream &os) const;

	void DoMove(Action m_Action);
	void UndoMove();

    bool SaveBoard(string filename, string comment) const;
    bool LoadBoard(string filename, streamoff &offset);
	bool SaveKifu(string filename) const;

	bool IsGameOver();
	bool IsSennichite() const;
	bool IsCheckAfter(const int src, const int dst);
	inline void NextTurn() { m_turn ^= 1; }
    inline int GetTurn() const { return m_turn; }
	inline int GetStep() const { return m_step; }
	inline int GetEvaluate() const { return m_turn ? m_evaluate : -m_evaluate; };
	inline Zobrist::Zobrist GetZobristHash() const { return m_turn ? m_hashcode2 : m_hashcode; }
	unsigned int GetKifuHash() const ;
};

#endif