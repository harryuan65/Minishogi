#ifndef _MINISHOGI_
#define _MINISHOGI_

#include "Bitboard.h"
#include "Types.h"
using std::string;

class Thread;

class Minishogi {
private:
	Color turn;
	int ply;
	Bitboard occupied[COLOR_NB];
	Bitboard bitboard[CHESS_NB];
	int board[SQUARE_NB];
	Thread *thisThread;

	Move moveHist[MAX_HISTORY_PLY];
	Chess captureHist[MAX_HISTORY_PLY];
	Value evalHist[MAX_HISTORY_PLY + 1];
	Value pinHist[MAX_HISTORY_PLY + 1];
	Key keyHist[MAX_HISTORY_PLY + 1];
	Key key2Hist[MAX_HISTORY_PLY + 1];
	Bitboard checker_bb[MAX_HISTORY_PLY + 1];

	inline void checker_BB();
	inline void pin_score();

public:
	void Initialize();
	void Initialize(const char *str);
	bool Initialize(const string* str);
	void Set(const Minishogi &m, Thread *th);

	void DoMove(Move m);
	void UndoMove();
	bool PseudoLegal(Move m) const;
	ExtMove* AttackGenerator(ExtMove *moveList) const;
	ExtMove* MoveGenerator(ExtMove *moveList) const;
	ExtMove* HandGenerator(ExtMove *moveList);
	inline Bitboard Movable(const int srcIndex, const Bitboard occupied = 0) const;
	inline Bitboard RookMovable(const int srcIndex, Bitboard occupied = 0) const;
	inline Bitboard BishopMovable(const int srcIndex, Bitboard occupied = 0) const;
	inline Bitboard attackers_to(const int dstIndex, const Bitboard occupied = 0) const;
	bool SEE(Move m, Value threshold = VALUE_ZERO) const;
	Move* GetLegalMoves(Move* moveList);

	void PrintChessBoard() const;
	void PrintNoncolorBoard(std::ostream &os) const;
	bool SaveBoard(string filename) const;
	bool LoadBoard(string filename, std::streamoff &offset);
	void PrintKifu(std::ostream &os) const;

	bool IsGameOver();
	bool IsLegelAction(Move m);
	inline bool IsChecked() const { return checker_bb[ply]; }
	inline bool IsCheckedAfter(const Move m) const;
	bool IsCheckedAfter(const Square srcIndex, const Square dstIndex) const;
	bool IsCheckingAfter(const Move m);
	bool IsSennichite() const;
	inline bool IsIsomorphic() const { return keyHist[ply] == key2Hist[ply]; }

	Thread* GetThread() const { return thisThread; }
	inline Color   GetTurn() const { return turn; }
	inline int     GetStep() const { return ply; }
	inline Value   GetEvaluate() const {
		return turn ? -evalHist[ply] - pinHist[ply] : evalHist[ply] + pinHist[ply];
	};
	inline Key     GetKey() const { return turn ? key2Hist[ply] : keyHist[ply]; }
	inline Key     GetKey(int p) const { return turn ^ (p % 2 == 0) ? key2Hist[p] : keyHist[p]; }
	inline Chess   GetChessOn(int sq) const {
		return (Chess)(sq < BOARD_NB ? board[sq] : (board[sq] ? HandToChess[sq] : EMPTY));
	}
	inline Chess   GetCapture() const { return captureHist[ply - 1]; }
	unsigned int   GetKifuHash() const ;
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