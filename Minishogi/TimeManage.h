#pragma once

#include "usi.h"

class TimeManagement
{
public:
	void Init(LimitsType& limits, Turn turn, int ply);
	void SetTimer(Value value, double pvFactor);
	int GetRemain() const { return remain_time; }
	int GetOptimum() const { return optimum_time; }
	int GetMaximum() const { return maximum_time; }
	int Elapsed() const { return int(now() - start_time); }
	void Reset() { start_time = now(); }
	//int64_t availableNodes; // When in 'nodes as time' mode

private:
	TimePoint start_time;
	int remain_time;
	int optimum_time;
	int maximum_time;
};

extern TimeManagement Time;
