#ifndef _EVALUATELEARN_H_
#define _EVALUATELEARN_H_
#include <array>
#include "Minishogi.h"

namespace EvaluateLearn {
	constexpr char CUSTOM_BOARD_FILE[] = "evallearn_board.txt";
	constexpr uint64_t LEARN_PATCH_SIZE = 200;
	constexpr Value EVAL_LIMIT = (Value)3000;
	constexpr double LAMBDA = 0.5;
	constexpr double GAMMA = 0.93;

	typedef float LearnFloatType;
	typedef std::array<LearnFloatType, 2> WeightValue;

	struct Weight {
		// w : 權重 g : 一次mini-batch的梯度
		WeightValue weight, grad;

		static LearnFloatType eta;
		static int skip_count;

		WeightValue grad2;

		void addGrad(LearnFloatType g1, LearnFloatType g2);
		bool update(bool skip_update);
	};

	void InitGrad();
	double CalcGrad(Value searchValue, Value quietValue, bool winner, double progress);
	void AddGrad(const Minishogi &m, Color turn, double delta_grad);
	void UpdateKPPT(uint64_t epoch);
}


inline double sigmoid(double x) {
	return 1.0 / (1.0 + std::exp(-x));
}

inline double dsigmoid(double x) {
	return sigmoid(x) * (1.0 - sigmoid(x));
}

inline double winest(Value value) { 
	return sigmoid((int)value / 600.0); 
}


#endif
