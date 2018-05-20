#ifndef _MOVE_
#define _MOVE_
#include <iostream>
#include <string>
/*
struct Move {
	int srcIndex;
	int dstIndex;
	int srcChess;
	int dstChess;
	bool isPro;
	enum : int {
		DO,
		UNDO,
		SURRENDER,
		SAVEBOARD,
		ILLEGAL
	}mode;

	inline bool IsOk() const {
		return mode == DO;
	}

	inline void Set(int si, int di, bool p) {
		mode = Move::DO;
		srcIndex = si;
		dstIndex = di;
		isPro = p;
	}

	inline unsigned __int32 ToU32() const {
		if (!IsOk()) {
			return 0;
		}
		return (isPro << 24) | (dstChess << 18) | (srcChess << 12) | (dstIndex << 6) | srcIndex;
	}

	inline void SetU32(unsigned __int32 u) {
		if (u) {
			mode = Move::DO;
			srcIndex =  u & 0x000003f;
			dstIndex = (u & 0x0000fc0) >> 6;
			isPro    = (u & 0x1000000) >> 24;
		}
		else {
			mode = Move::SURRENDER;
		}
	}

	inline std::string ToString() const {
		switch (mode) {
		case DO:
			return Index2Input(srcIndex) + Index2Input(dstIndex) + (isPro ? "+" : " ");
		case UNDO:
			return "UNDO";
		case SURRENDER:
			return "SURRENDER";
		case SAVEBOARD:
			return "SAVEBOARD";
		}
	}

	static inline int Input2Index(char row, char col)  {
		row = toupper(row);
		if ('A' <= row && row <= 'G' && '1' <= col && col <= '5') {
			return (row - 'A') * 5 + '5' - col;
		}
		return -1;
	}

	static inline std::string Index2Input(int index) {
		if (0 <= index && index < 35) {
			std::string str;
			str.push_back('A' + index / 5);
			str.push_back('5' - index % 5);
			return str;
		}
		return "";
	}

	inline bool operator==(const Move& ra) const {
		return mode == ra.mode && srcIndex == ra.srcIndex &&
			dstIndex == ra.dstIndex && isPro == ra.isPro;
	}

	friend std::istream& operator>>(std::istream &is, Move& action) {
		std::string str;
		is >> str;
		if (str == "SURRENDER" || str == "surrender") {
			action.mode = Move::SURRENDER;
		}
		else if (str == "UNDO" || str == "undo") {
			action.mode = Move::UNDO;
		}
		else if (str == "SAVEBOARD" || str == "saveboard") {
			action.mode = Move::SAVEBOARD;
		}
		else if (str.length() != 4 || (str.length() == 5 && str[4] != '+')) {
			action.mode = Move::ILLEGAL;
		}
		else {
			action.mode = Move::DO;
			action.srcIndex = Input2Index(str[0], str[1]);
			action.dstIndex = Input2Index(str[2], str[3]);
			action.isPro = str.length() == 5;
		}
		return is;
	}

	friend std::ostream& operator<< (std::ostream& os, const Move& action) {
		os << action.ToString();
		return os;
	}
};

struct ExtMove {
	Move move;
	int value;

	operator Move() const { return move; }
	void operator=(Move m) { move = m; }

	// Inhibit unwanted implicit conversions to Move
	// with an ambiguity that yields to a compile error.
	operator float() const = delete;
};

inline bool operator<(const ExtMove& f, const ExtMove& s) {
	return f.value < s.value;
}
*/
#endif
