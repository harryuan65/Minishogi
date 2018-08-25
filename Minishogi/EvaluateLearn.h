#ifndef _EVALUATELEARN_H_
#define _EVALUATELEARN_H_
#include <array>
#include "Minishogi.h"

namespace EvaluateLearn {
	typedef float LearnFloatType;
	typedef std::array<LearnFloatType, 2> WeightValue;

	struct Weight {
		// w : �v�� g : �@��mini-batch�����
		WeightValue weight, grad;

		static LearnFloatType eta;
		static int skip_count;

		WeightValue grad2;

		void addGrad(LearnFloatType g1, LearnFloatType g2);
		bool update(bool skip_update);
	};

	void InitGrad();
	void AddGrad(Minishogi &m, Color turn, double delta_grad);
	void Update(uint64_t epoch);
}

#endif
