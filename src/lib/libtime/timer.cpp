#include "timer.hpp"


namespace libtime
{
	Timer::Timer()
	{
		t_start = std::chrono::high_resolution_clock::now();
	}

	double Timer::get_curr_time()
	{
		auto t_now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> dur = t_now - t_start;
		return dur.count();
	}
};
