#ifndef _MINISHOGI_H_
#define _MINISHOGI_H_
#include <iostream>

#include "Bitboard.h"
#include "Evaluate.h"
#include "Types.h"
#include "Zobrist.h"

class Thread;

struct BonaPieceDiff {
	BonaPiece preBonaW;
	BonaPiece preBonaB;
	BonaPiece nowBonaW;
	BonaPiece nowBonaB;
};

struct StateInfo {
	// Need to copy before move
	Evaluate::EvalSum eval;
	Key key;
	Key key2;
	Bitboard checker_bb;

	// No need to copy before move
	Move move;
	Piece capture;
	// 移動與吃子的BonaPiece變化
	// 0 MoverDiff 1 CaptureDiff
	BonaPieceDiff bonaPieceDiff[2];
};

class Minishogi {
public:
	const static std::string START_SFEN;

	Minishogi(Thread *th) : thisThread(th) {};
	void Initialize();
	bool Initialize(const Minishogi &m);
	bool Initialize(std::string sfen);
	bool InitializeByBoard(std::string str);
	std::string Sfen() const;

	void DoMove(std::string m);
	void DoMove(Move m);
	void UndoMove();
	void DoNullMove();
	void UndoNullMove();
	bool PseudoLegal(Move m) const;

	ExtMove* AttackGenerator(ExtMove *moveList) const;
	ExtMove* MoveGenerator(ExtMove *moveList) const;
	ExtMove* HandGenerator(ExtMove *moveList);
	Bitboard Movable(int srcIndex, Bitboard occupied = 0) const;
	Bitboard RookMovable(int srcIndex, Bitboard occupied = 0) const;
	Bitboard BishopMovable(int srcIndex, Bitboard occupied = 0) const;
	Bitboard attackers_to(int dstIndex, Bitboard occupied = 0) const;
	bool SEE(Move m, Value threshold = VALUE_ZERO) const;

	void PrintChessBoard() const;
	bool SaveBoard(std::string filename) const;
	bool LoadBoard(std::string filename, std::streamoff &offset);
	void PrintKifu(std::ostream &os) const;

	// Slow, just for debug
	bool CheckLegal() const;
	bool IsGameOver();
	bool IsLegelAction(Move m);
	Move* GetLegalMoves(Move* moveList);

	// Fast, use in search
	bool IsChecking();
	bool IsInChecked() const;
	bool IsInCheckedAfter(Move m) const;
	bool IsInCheckedAfter(Square srcIndex, Square dstIndex) const;
	bool IsCheckAfter(const Move m);
	bool IsSennichite() const;

	Thread* GetThread() const;
	Color GetTurn() const;
	int GetPly() const;
	Value GetEvaluate();
	Key GetKey() const;
	Key GetKey(int p) const;
	Piece GetChessOn(int sq) const;
	Move GetHistMove(int i) const;
	Move GetPrevMove() const;
	Piece GetPrevCapture() const;
	int GetBoard(Square sq) const;
	Bitboard GetBitboard(Piece c) const;
	Bitboard GetOccupied(Color c) const;
	const BonaPiece* GetPieceList(Color c) const;
	uint32_t GetKifuHash() const;

	friend std::ostream& operator<<(std::ostream& os, const Minishogi& pos);

private:
	Bitboard GetChecker();
	void CalcAllPin();
	void CalcAllPos();
	void CalcDiffPos();

	void SetBonaPiece(Square sq, Piece c);
	void SetBonaPiece(BonaPieceIndex index, BonaPiece w, BonaPiece b);
	void DoBonaPiece(BonaPieceDiff &bpd, Square old_sq, int old_c, Square new_sq, int new_c);
	void UndoBonaPiece(const BonaPieceDiff &bpd);

	Thread *thisThread;
	Color turn;
	int ply;

	// 同方在盤面上棋子的bitboard
	Bitboard occupied[COLOR_NB];

	// 所有盤面上棋子的bitboard
	Bitboard bitboard[PIECE_NB];

	// 0~24 盤面上的棋 25~34 手牌的數量
	int board[SQUARE_NB];

	// Piece no. -> BonaPiece
	// 0 pieceListW 1 pieceListB
	BonaPiece pieceList[2][BONA_PIECE_INDEX_NB];

	// BonaPiece -> Piece no.
	BonaPieceIndex bonaIndexList[BONA_PIECE_NB];

	// 歷史走步的移動與盤面紀錄 起始盤面在[0] 移動move[0]變成盤面[1]
	StateInfo stateHist[MAX_PLY];
};


inline void Minishogi::DoMove(std::string m) {
	DoMove(usi2move(m, turn));
}

/// 現在是否將軍對方
inline bool Minishogi::IsChecking() {
	turn = ~turn;
	bool isCheck = GetChecker();
	turn = ~turn;
	return isCheck;
}

/// 現在是否被將軍
inline bool Minishogi::IsInChecked() const {
	return stateHist[ply].checker_bb;
}

/// 移動完有沒有被將軍
inline bool Minishogi::IsInCheckedAfter(Move m) const {
	return IsInCheckedAfter(from_sq(m), to_sq(m));
}

inline Thread* Minishogi::GetThread() const { 
	return thisThread;
}

inline Color Minishogi::GetTurn() const {
	return turn; 
}

inline int Minishogi::GetPly() const { 
	return ply;
 }

inline Value Minishogi::GetEvaluate() { 
	if (stateHist[ply].eval.IsNotCalc()) {
		CalcAllPin();
		CalcDiffPos();
	}
	// Debug : CalcDiffPos()
	/*Value value = stateHist[ply].eval.Sum(turn);
	CalcAllPos();
	if (value != stateHist[ply].eval.Sum(turn)) {
		sync_cout << "Error : DiffPos = " << value << " AllPos = " << stateHist[ply].eval.Sum(turn) << sync_endl;
		system("pause");
	}*/

	return stateHist[ply].eval.Sum(turn);
}

inline Key Minishogi::GetKey() const {
#ifdef ENEMY_ISO_TT
	return turn ? key2Hist[ply] : keyHist[ply];
#else
	return stateHist[ply].key;
#endif
}

inline Key Minishogi::GetKey(int p) const { 
	return (turn ^ (p % 2 == 0)) ? stateHist[ply].key2 : stateHist[ply].key;
}

inline Piece Minishogi::GetChessOn(int sq) const { 
	return (Piece)(sq < BOARD_NB ? board[sq] : (board[sq] ? HandToChess[sq] : NO_PIECE));
}

inline Move Minishogi::GetHistMove(int i) const {
	return stateHist[i].move;
}

inline Move Minishogi::GetPrevMove() const {
	return stateHist[ply].move;
}

inline Piece Minishogi::GetPrevCapture() const { 
	return stateHist[ply].capture;
}

inline int Minishogi::GetBoard(Square sq) const {
	return board[sq];
}

inline Bitboard Minishogi::GetBitboard(Piece c) const {
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
	//for (int i = 0; i < ply; i++)
		//seed ^= toU32(moveHist[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
}

inline void Minishogi::SetBonaPiece(Square sq, Piece pc) {
	BonaPieceIndex index; 
	index = BonaPieceIndex(sq < BOARD_NB ? ((pc & 7) - 1) * 2 : (sq % 5) * 2);
	if (pieceList[WHITE][index] != BONA_PIECE_NB)
		++index;
	pieceList[WHITE][index] = to_bonapiece(sq, pc);
	pieceList[BLACK][index] = to_inv_bonapiece(sq, pc);
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

inline void Minishogi::UndoBonaPiece(const BonaPieceDiff &bpd) {
	BonaPieceIndex index = bonaIndexList[bpd.nowBonaW];
	pieceList[WHITE][index] = bpd.preBonaW;
	pieceList[BLACK][index] = bpd.preBonaB;
	bonaIndexList[bpd.preBonaW] = index;
}

#endif