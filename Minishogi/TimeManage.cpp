#include <algorithm>

#include "TimeManage.h"

TimeManagement Time;

void TimeManagement::Init(LimitsType& limits, Turn turn, int ply) {
	if (limits.time[turn] == 0 && limits.byoyomi)
		limits.move_time = limits.byoyomi - USI::Options["NetworkDelay"];

	int remain_ply = std::max(1, (80 - ply) / (limits.inc[turn] ? 8 : 2));
	remain_time = limits.time[turn] + limits.byoyomi;
	optimum_time = limits.byoyomi ? limits.byoyomi : remain_time / remain_ply;
	int maxtime = (limits.inc[turn] || limits.byoyomi) ? optimum_time * 10 : optimum_time * 3;
	maximum_time = std::min(maxtime, remain_time);
	optimum_time -= USI::Options["NetworkDelay"];
	maximum_time -= USI::Options["NetworkDelay"];
	start_time = limits.start_time;
}

void TimeManagement::SetTimer(Value value, double pvFactor) {
	double valueFactor;
	if (value > 0)
		valueFactor = pow((double(VALUE_MATE - value) / 10000), 2);
	else
		valueFactor = pow((double(2000 - value) / 1000), 3);
	optimum_time = remain_time * (valueFactor * 0.5 + pvFactor * 2.0) * 0.005;
	maximum_time = remain_time * std::min((valueFactor + pvFactor * 8.0) * 0.005, 0.8);
}