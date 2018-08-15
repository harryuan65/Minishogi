#ifndef _MINISHOGI_
#define _MINISHOGI_

#include "Bitboard.h"
#include "Types.h"
#include "Observer.h"
#include "Zobrist.h"
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

	Move moveHist[MAX_HISTORY_PLY - 1];
	Chess captureHist[MAX_HISTORY_PLY - 1];
	Value evalHist[MAX_HISTORY_PLY];
	Value pinHist[MAX_HISTORY_PLY];
	Key keyHist[MAX_HISTORY_PLY];
	Key key2Hist[MAX_HISTORY_PLY];
	Bitboard checker_bb[MAX_HISTORY_PLY];

	inline void checker_BB();
	inline void pin_score();

public:
	void Initialize();
	void Initialize(const char *str);
	bool Initialize(const string* str);
	void Set(const Minishogi &m, Thread *th);

	void DoMove(Move m);
	void UndoMove();
	void DoNullMove();
	void UndoNullMove();
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
	inline bool IsInChecked() const { return checker_bb[ply]; }
	inline bool IsInCheckedAfter(const Move m) const;
	bool IsInCheckedAfter(const Square srcIndex, const Square dstIndex) const;
	bool IsCheckAfter(const Move m);
	bool IsSennichite() const;
	//inline bool IsIsomorphic() const { return keyHist[ply] == key2Hist[ply]; }

	inline Thread* GetThread() const { return thisThread; }
	inline Color   GetTurn() const { return turn; }
	inline int     GetStep() const { return ply; }
	inline Value   GetEvaluate() const { return turn ? -evalHist[ply] - pinHist[ply] : evalHist[ply] + pinHist[ply]; };
#ifdef ENEMY_ISO_TT
	inline Key     GetKey() const { return turn ? key2Hist[ply] : keyHist[ply]; }
#else
	inline Key     GetKey() const { return keyHist[ply]; }
#endif
	inline Key     GetKey(int p) const { return (turn ^ (p % 2 == 0)) ? key2Hist[p] : keyHist[p]; }
	inline Chess   GetChessOn(int sq) const { return (Chess)(sq < BOARD_NB ? board[sq] : (board[sq] ? HandToChess[sq] : NO_PIECE)); }
	inline Chess   GetCapture() const { return captureHist[ply - 1]; }
	unsigned int   GetKifuHash() const ;
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