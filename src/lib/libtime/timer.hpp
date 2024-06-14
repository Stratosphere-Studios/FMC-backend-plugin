#pragma once

#include <chrono>


namespace libtime
{
	struct Timer
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> t_start;

		Timer();

		double get_curr_time();
	};

	struct SteadyTimer
	{
		std::chrono::time_point<std::chrono::steady_clock> t_start;

		SteadyTimer();

		double get_curr_time();
	};
};

