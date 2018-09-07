#ifndef _EVALUATELEARN_H_
#define _EVALUATELEARN_H_
#include <array>
#include <vector>

#include "Minishogi.h"

namespace EvaluateLearn {
	constexpr char CUSTOM_BOARD_FILE[] = "evallearn_board.txt";
	constexpr int LEARN_PATCH_SIZE = 200;
	constexpr int SAVE_PATCH_SIZE = 10000;
	constexpr Value EVAL_LIMIT = (Value)3000;
	constexpr double LAMBDA = 0.5;
	constexpr double GAMMA = 0.93;

	extern std::vector<std::string> rootSFEN;

	typedef float LearnFloatType;
	typedef std::array<LearnFloatType, 2> WeightValue;

	struct Weight {
		WeightValue weight, grad, grad2;

		static LearnFloatType eta;
		static int skip_count;

		void addGrad(LearnFloatType g1, LearnFloatType g2);
		bool update(bool skip_update);
	};

	struct LearnData {
		int rootPosIndex;
		std::vector<Move> moves;
		std::vector<Value> values;
		Color winner;
		int learningPly = 0;
	};

	bool LoadRootPos(std::string filename);
	void InitGrad();
	double CalcGrad(Value searchValue, Value quietValue, bool winner, double progress);
	void AddGrad(const Minishogi &m, Color turn, double delta_grad);
	void UpdateKPPT(uint64_t epoch);

	void SelfLearn();
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
