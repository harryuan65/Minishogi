#ifndef _MINISHOGI_H_
#define _MINISHOGI_H_

#include "Bitboard.h"
#include "Evaluate.h"
#include "Types.h"
#include "Observer.h"
#include "Zobrist.h"
using std::string;
using Evaluate::EvalSum;

class Thread;

struct BonaPieceDiff {
	BonaPiece preBonaW;
	BonaPiece preBonaB;
	BonaPiece nowBonaW;
	BonaPiece nowBonaB;
};

class Minishogi {
public:
	void Initialize();
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
	Move* GetLegalMoves(Move* moveList);
	Bitboard Movable(const int srcIndex, const Bitboard occupied = 0) const;
	Bitboard RookMovable(const int srcIndex, Bitboard occupied = 0) const;
	Bitboard BishopMovable(const int srcIndex, Bitboard occupied = 0) const;
	Bitboard attackers_to(const int dstIndex, const Bitboard occupied = 0) const;
	bool SEE(Move m, Value threshold = VALUE_ZERO) const;

	void PrintChessBoard() const;
	void PrintNoncolorBoard(std::ostream &os) const;
	bool SaveBoard(string filename) const;
	bool LoadBoard(string filename, std::streamoff &offset);
	void PrintKifu(std::ostream &os) const;

	bool IsGameOver();
	bool IsLegelAction(Move m);
	bool IsInChecked() const;
	bool IsInCheckedAfter(const Move m) const;
	bool IsInCheckedAfter(const Square srcIndex, const Square dstIndex) const;
	bool IsCheckAfter(const Move m);
	bool IsSennichite() const;

	Thread* GetThread() const;
	Color GetTurn() const;
	int GetStep() const;
	Value GetEvaluate();
	Key GetKey() const;
	Key GetKey(int p) const;
	Chess GetChessOn(int sq) const;
	Chess GetCapture() const;
	int GetBoard(Square sq) const;
	Bitboard GetBitboard(Chess c) const;
	Bitboard GetOccupied(Color c) const;
	const BonaPiece* GetPieceList(Color c) const;
	uint32_t GetKifuHash() const;

private:
	void checker_BB();
	void CalcAllPin();
	void CalcAllPos();
	void CalcDiffPos();

	void SetBonaPiece(BonaPieceIndex index, Square sq, int c);
	void SetBonaPiece(BonaPieceIndex index, BonaPiece w, BonaPiece b);
	void DoBonaPiece(BonaPieceDiff &bpd, Square old_sq, int old_c, Square new_sq, int new_c);
	void UndoBonaPiece(BonaPieceDiff &bpd);

	Thread *thisThread;
	Color turn;
	int ply;
	Bitboard occupied[COLOR_NB];
	Bitboard bitboard[PIECE_NB];
	// 0~24 盤面上的棋 25~34 手牌的數量
	int board[SQUARE_NB];
	// Piece no. -> BonaPiece
	// 0 pieceListW 1 pieceListB
	BonaPiece pieceList[2][BONA_PIECE_INDEX_NB];
	// BonaPiece -> Piece no.
	BonaPieceIndex bonaIndexList[BONA_PIECE_NB];

	Move moveHist[MAX_HISTORY_PLY - 1];
	Chess captureHist[MAX_HISTORY_PLY - 1];
	// 移動與吃子的BonaPiece變化
	// 0 MoverDiff 1 CaptureDiff
	BonaPieceDiff bonaPieceDiffHist[MAX_HISTORY_PLY - 1][2];

	EvalSum evalHist[MAX_HISTORY_PLY];
	Key keyHist[MAX_HISTORY_PLY];
	Key key2Hist[MAX_HISTORY_PLY];
	Bitboard checker_bb[MAX_HISTORY_PLY];
};

/// 現在是否被將軍
inline bool Minishogi::IsInChecked() const {
	return checker_bb[ply];
}

/// 移動完有沒有被將軍
inline bool Minishogi::IsInCheckedAfter(const Move m) const {
	return IsInCheckedAfter(from_sq(m), to_sq(m));
}

inline Thread* Minishogi::GetThread() const { 
	return thisThread;
}

inline Color Minishogi::GetTurn() const {
	return turn; 
}

inline int Minishogi::GetStep() const { 
	return ply;
 }

inline Value Minishogi::GetEvaluate() { 
	if (evalHist[ply].pin == VALUE_NONE) {
		CalcAllPin();
		CalcDiffPos();
	}
	return evalHist[ply].Sum(turn);
}

inline Key Minishogi::GetKey() const {
#ifdef ENEMY_ISO_TT
	return turn ? key2Hist[ply] : keyHist[ply];
#else
	return keyHist[ply];
#endif
}

inline Key Minishogi::GetKey(int p) const { 
	return (turn ^ (p % 2 == 0)) ? key2Hist[p] : keyHist[p];
}

inline Chess Minishogi::GetChessOn(int sq) const { 
	return (Chess)(sq < BOARD_NB ? board[sq] : (board[sq] ? HandToChess[sq] : NO_PIECE));

}
inline Chess Minishogi::GetCapture() const { 
	return captureHist[ply - 1]; 
}

inline int Minishogi::GetBoard(Square sq) const {
	return board[sq];
}

inline Bitboard Minishogi::GetBitboard(Chess c) const {
	return bitboard[c];
}

inline Bitboard Minishogi::GetOccupied(Color c) const {
	return occupied[c];
}

inline const BonaPiece* Minishogi::GetPieceList(Color c) const {
	return pieceList[c];
}

inline uint32_t Minishogi::GetKifuHash() const {
	unsigned int seed = ply;
	for (int i = 0; i < ply; i++) {
		seed ^= toU32(moveHist[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}

inline void Minishogi::SetBonaPiece(BonaPieceIndex index, Square sq, int c) {
	pieceList[WHITE][index] = to_bonapiece(sq, c);
	pieceList[BLACK][index] = to_inv_bonapiece(sq, c);
	bonaIndexList[pieceList[WHITE][index]] = index;
}

inline void Minishogi::SetBonaPiece(BonaPieceIndex index, BonaPiece w, BonaPiece b) {
	pieceList[WHITE][index] = w;
	pieceList[BLACK][index] = b;
	bonaIndexList[w] = index;
}

inline void Minishogi::DoBonaPiece(BonaPieceDiff &bpd, Square old_sq, int old_c, Square new_sq, int new_c) {
	BonaPieceIndex index = bonaIndexList[to_bonapiece(old_sq, old_c)];
	bpd.preBonaW = pieceList[WHITE][index];
	bpd.preBonaB = pieceList[BLACK][index];
	pieceList[WHITE][index] = to_bonapiece(new_sq, new_c);
	pieceList[BLACK][index] = to_inv_bonapiece(new_sq, new_c);
	bpd.nowBonaW = pieceList[WHITE][index];
	bpd.nowBonaB = pieceList[BLACK][index];
	bonaIndexList[bpd.nowBonaW] = index;
}

inline void Minishogi::UndoBonaPiece(BonaPieceDiff &bpd) {
	BonaPieceIndex index = bonaIndexList[bpd.nowBonaW];
	pieceList[WHITE][index] = bpd.preBonaW;
	pieceList[BLACK][index] = bpd.preBonaB;
	bonaIndexList[bpd.preBonaW] = index;
}



//+Example Board
//▼飛▼角▼銀▼金▼玉
// ．  ．  ．  ． ▼步
// ．  ．  ．  ．  ． 
//△步 ．  ．  ．  ． 
//△王△金△銀△角△飛
//▼0步0銀0金0角0飛
//△0步0銀0金0角0飛

#endif