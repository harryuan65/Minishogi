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

enum SennichiteType {
	NO_SENNICHITE, SENNICHITE_WIN, SENNICHITE_LOSE, SENNICHITE_CHECK
};

struct StateInfo {
	// Need to copy before move
	Evaluate::EvalSum eval;
	Key key;
	Key key2;
	int continueCheck[2];
	int nullMovePly;

	// No need to copy before move
	Move move;
	Piece capture;
	Bitboard checker_bb;
	// ���ʻP�Y�l��BonaPiece�ܤ�
	BonaPieceDiff bonaPieceDiff[2]; // 0 MoverDiff 1 CaptureDiff
	int sCount;
	bool sLegal;
	SennichiteType sType;
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

	ExtMove* AttackGenerator(ExtMove *moveList, Bitboard dstBB = BitboardMask) const;
	ExtMove* MoveGenerator(ExtMove *moveList) const;
	ExtMove* HandGenerator(ExtMove *moveList) const;
	bool SEE(Move m, Value threshold = VALUE_ZERO) const;

	bool SaveBoard(std::string filename) const;
	bool LoadBoard(std::string filename, std::streamoff &offset);


	/* Slow, just for debug */

	// Position���c�O�_���T
	bool CheckLegal() const;
	bool IsMate();


	/* Fast, use in search */

	// �Ӳ��ʬO�_�O�X�k�B ���ˬd�d��ⵥ�H�W�B
	bool PseudoLegal(Move m) const;
	// �{�b�O�_�N�x���
	bool IsChecking();
	// ���ʧ����S���N�x���
	bool IsCheckingAfter(Move m);
	// �{�b�O�_�Q�N�x
	bool IsInChecked() const;
	// ���ʧ����S���Q�N�x
	bool IsInCheckedAfter(Move m) const;
	// ���ʧ����S���Q�N�x
	bool IsInCheckedAfter(Square srcIndex, Square dstIndex) const;
	// �p�G�{�b�L�����g�X�{�L �B�Z��������(�P�ӤH) �P�w���d��� �ݭn��DoMove��A�P�_
	void CalcSennichiteType(int checkMaxPly);
	// �O�_�|�o�ͥ��B�� �ݭn��DoMove��A�P�_
	bool IsUchifuzume() const;

	/* Position Get Function */

	Thread* GetThread() const;
	Turn GetTurn() const;
	int GetPly() const;
	Piece GetBoard(Square sq) const;
	Piece GetPiece(Move m) const;
	Piece GetCapture(Move m) const;
	Bitboard GetOccupied(Turn t) const;
	Bitboard GetBitboard(Piece c) const;
	const BonaPiece* GetPieceList(Turn t) const;

	/* StateInfo Get Function */

	Value GetEvaluate();
	Key GetKey() const;
	Key GetKey(int p) const;
	Move GetHistMove(int i) const;
	Move GetPrevMove() const;
	Piece GetPrevCapture() const;
	Value GetSennichiteValue() const;

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
	Turn turn;
	int ply;

	// �P��b�L���W�Ѥl��bitboard
	Bitboard occupied[COLOR_NB];

	// �Ҧ��L���W�Ѥl��bitboard
	Bitboard bitboard[PIECE_NB];

	// 0~24 �L���W���� 25~34 ��P���ƶq
	int board[SQUARE_NB];

	// Piece no. -> BonaPiece
	// 0 pieceListW 1 pieceListB
	BonaPiece pieceList[2][BONA_PIECE_INDEX_NB];

	// BonaPiece -> Piece no.
	BonaPieceIndex bonaIndexList[BONA_PIECE_NB];

	// ���v���B�����ʻP�L������ �_�l�L���b[0] ����move[0]�ܦ��L��[1]
	StateInfo stateHist[MAX_PLY];
};


inline void Minishogi::DoMove(std::string m) {
	DoMove(usi2move(m, turn));
}

inline bool Minishogi::IsChecking() {
	turn = ~turn;
	bool isCheck = GetChecker();
	turn = ~turn;
	return isCheck;
}

inline bool Minishogi::IsInChecked() const {
	return stateHist[ply].checker_bb;
}

inline bool Minishogi::IsInCheckedAfter(Move m) const {
	return IsInCheckedAfter(from_sq(m), to_sq(m));
}

/* Position Get Function */

inline Thread* Minishogi::GetThread() const { 
	return thisThread;
}

inline Turn Minishogi::GetTurn() const {
	return turn; 
}

inline int Minishogi::GetPly() const { 
	return ply;
 }

inline Piece Minishogi::GetBoard(Square sq) const {
	return (Piece)board[sq];
}

inline Piece Minishogi::GetPiece(Move m) const {
	return (Piece)(from_sq(m) < BOARD_NB ? board[from_sq(m)] : HandToPiece[from_sq(m)]);
}

inline Piece Minishogi::GetCapture(Move m) const {
	return (Piece)board[to_sq(m)];
}

inline Bitboard Minishogi::GetOccupied(Turn t) const {
	return occupied[t];
}

inline Bitboard Minishogi::GetBitboard(Piece c) const {
	return bitboard[c];
}

inline const BonaPiece* Minishogi::GetPieceList(Turn t) const {
	return pieceList[t];
}

/* StateInfo Get Function */

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
	return turn ? stateHist[ply].key2 : stateHist[ply].key;
#else
	return stateHist[ply].key;
#endif
}

inline Key Minishogi::GetKey(int p) const { 
	return (turn ^ (p % 2 == 0)) ? stateHist[p].key2 : stateHist[p].key;
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

inline Value Minishogi::GetSennichiteValue() const {
	if (!stateHist[ply].sLegal)
		return VALUE_NONE;
	switch (stateHist[ply].sType) {
	case SENNICHITE_WIN:
		return VALUE_MATE - (3 - stateHist[ply].sCount) * VALUE_SENNICHITE - 1;
	case SENNICHITE_LOSE:
	case SENNICHITE_CHECK:
		return -(VALUE_MATE - (3 - stateHist[ply].sCount) * VALUE_SENNICHITE) + 1;
	case NO_SENNICHITE:
		return VALUE_NONE;
	}
}

/* Private Set Function */

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