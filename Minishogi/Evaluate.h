#ifndef _EVALUATE_H_
#define _EVALUATE_H_
#include <array>
#include <string>
#include "Types.h"

class Minishogi;

namespace Evaluate {
	const std::string KPPT_DIRPATH = "kppt/";
	const std::string KK_FILENAME = "kk_binary.bin";
	const std::string KKP_FILENAME = "kkp_binary.bin";
	const std::string KPP_FILENAME = "kpp_binary.bin";
	constexpr int FV_SCALE = 32;
	
	typedef std::array<int32_t, 2> ValueKk;
	typedef std::array<int32_t, 2> ValueKkp;
	typedef std::array<int16_t, 2> ValueKpp;

	template <typename Tl, typename Tr>
	inline std::array<Tl, 2> operator+=(std::array<Tl, 2>& lhs, const std::array<Tr, 2>& rhs) {
		lhs[0] += rhs[0];
		lhs[1] += rhs[1];
		return lhs;
	}
	template <typename Tl, typename Tr>
	inline std::array<Tl, 2> operator-=(std::array<Tl, 2>& lhs, const std::array<Tr, 2>& rhs) {
		lhs[0] -= rhs[0];
		lhs[1] -= rhs[1];
		return lhs;
	}

	struct Evaluater {
		ValueKk kk[BOARD_NB][BOARD_NB];
		ValueKkp kkp[BOARD_NB][BOARD_NB][BONA_PIECE_NB];
		ValueKpp kpp[BOARD_NB][BONA_PIECE_NB][BONA_PIECE_NB];

		bool Load(std::string path);
		bool Save(std::string path);
		void Clean();
		void Blend(Evaluater &e, float ratio);
	};
	
	struct EvalSum {
		std::array<std::array<int32_t, 2>, 3> pos;
		Value material;
		Value pin;

		Value Sum(const Color c) const;
		void Clean();

		inline bool IsNotCalc() const;
		inline void SetNonCalc();
	};

	extern Evaluater GlobalEvaluater;
}

inline bool Evaluate::EvalSum::IsNotCalc() const {
	return pin == VALUE_NONE || pos[2][0] == VALUE_NONE;
}

inline void Evaluate::EvalSum::SetNonCalc() {
	pin = VALUE_NONE;
	pos[2][0] = VALUE_NONE;
}

#endif
