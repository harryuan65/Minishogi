#ifndef _ACTION_
#define _ACTION_
#include <iostream>
#include <string>
using namespace std;

struct Action {
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

	inline void Set(int si, int di, bool p) {
		mode = Action::DO;
		srcIndex = si;
		dstIndex = di;
		isPro = p;
	}

	inline unsigned __int32 ToU32() const {
		return (isPro << 24) | (dstChess << 18) | (srcChess << 12) | (dstIndex << 6) | srcIndex;
	}

	inline void SetU32(unsigned __int32 u) {
		srcIndex = u & 0x000003f;
		dstIndex = u & 0x0000fc0;
		isPro    = u & 0x1000000;
	}

	static inline int Input2Index(char row, char col)  {
		row = toupper(row);
		if ('A' <= row && row <= 'G' && '1' <= col && col <= '5') {
			return (row - 'A') * 5 + '5' - col;
		}
		return -1;
	}

	static inline string Index2Input(int index) {
		if (0 <= index && index < 35) {
			string str;
			str.push_back('A' + index / 5);
			str.push_back('5' - index % 5);
			return str;
		}
		return "";
	}

	inline bool operator==(const Action& ra) const {
		return mode == ra.mode && srcIndex == ra.srcIndex &&
			dstIndex == ra.dstIndex && isPro == ra.isPro;
	}

	friend istream& operator>>(istream &is, Action& action) {
		string str;
		is >> str;
		if (str == "SURRENDER" || str == "surrender") {
			action.mode = Action::SURRENDER;
		}
		else if (str == "UNDO" || str == "undo") {
			action.mode = Action::UNDO;
		}
		else if (str == "SAVEBOARD" || str == "saveboard") {
			action.mode = Action::SAVEBOARD;
		}
		else if (str.length() != 4 || (str.length() == 5 && str[4] != '+')) {
			action.mode = Action::ILLEGAL;
		}
		else {
			action.mode = Action::DO;
			action.srcIndex = Input2Index(str[0], str[1]);
			action.dstIndex = Input2Index(str[2], str[3]);
			action.isPro = str.length() == 5;
		}
		return is;
	}

	friend ostream& operator<< (ostream& os, const Action& action) {
		switch (action.mode) {
		case DO:
			os << Index2Input(action.srcIndex) << Index2Input(action.dstIndex) <<
				(action.isPro ? "+" : " ");
			break;
		case UNDO:
			os << "UNDO";
			break;
		case SURRENDER:
			os << "SURRENDER";
			break;
		case SAVEBOARD:
			os << "SAVEBOARD";
			break;
		}
		return os;
	}
};

#endif
