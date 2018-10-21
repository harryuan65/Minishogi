#ifndef _EVALUATELEARN_H_
#define _EVALUATELEARN_H_
#include <array>
#include <vector>
#include <sstream>

#include "Minishogi.h"

Move algebraic2move(std::string str, Minishogi &pos);

namespace EvaluateLearn {
	const std::string KIFULEARN_DIRPATH = "C:/Users/nyanpass.CJC/Documents/Minishogi/Training Kifu";

	typedef float LearnFloatType;
	typedef std::array<LearnFloatType, 2> WeightValue;

	struct Weight {
		WeightValue weight, grad, grad2;

		static LearnFloatType eta;
		static int skip_count;

		void addGrad(LearnFloatType g1, LearnFloatType g2);
		bool update(bool skip_update);
	};

	void InitGrad();
	double CalcGrad(Value searchValue, Value quietValue);
	double CalcGrad(Value searchValue, Value quietValue, bool winner, double progress);
	void AddGrad(const Minishogi &m, Color turn, double delta_grad);
	void UpdateKPPT(uint64_t epoch);

	void StartKifuLearn(std::istringstream& is);
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
