#ifndef _EVALUATE_H_
#define _EVALUATE_H_
#include <array>
#include <string>
#include "Types.h"

#define EVAL_PATH "kppt//"
#define KK_FILE   "kk_binary.bin"
#define KKP_FILE  "kkp_binary.bin"
#define KPP_FILE  "kpp_binary.bin"
using std::string;

class Minishogi;

namespace Evaluate {

	typedef std::array<int32_t, 2> ValueKk;
	typedef std::array<int32_t, 2> ValueKkp;
	typedef std::array<int16_t, 2> ValueKpp;

	template <typename Tl, typename Tr>
	inline std::array<Tl, 2> operator += (std::array<Tl, 2>& lhs, const std::array<Tr, 2>& rhs)	{
		lhs[0] += rhs[0];
		lhs[1] += rhs[1];
		return lhs;
	}
	template <typename Tl, typename Tr>
	inline std::array<Tl, 2> operator -= (std::array<Tl, 2>& lhs, const std::array<Tr, 2>& rhs)	{
		lhs[0] -= rhs[0];
		lhs[1] -= rhs[1];
		return lhs;
	}

	struct Evaluater {
		ValueKk kk[BOARD_NB][BOARD_NB];
		ValueKkp kkp[BOARD_NB][BOARD_NB][BONA_PIECE_NB];
		ValueKpp kpp[BOARD_NB][BONA_PIECE_NB][BONA_PIECE_NB];

		void Init();
		void Load(string path);
		void Save(string path);
		void Clean();
	};
	
	struct EvalSum {
		std::array<std::array<int32_t, 2>, 3> pos;
		Value meterial;
		Value pin;

		Value Sum(const Color c) const;
		Value PosSum(const Color c) const;
		void Clean();
	};

	const int FV_SCALE = 32;
	extern Evaluater evaluater;

	void Initialize();

}

#endif
