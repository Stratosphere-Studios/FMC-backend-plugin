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

	SteadyTimer::SteadyTimer()
	{
		t_start = std::chrono::steady_clock::now();
	}

	double SteadyTimer::get_curr_time()
	{
		auto t_now = std::chrono::steady_clock::now();
		std::chrono::duration<double> dur = t_now - t_start;
		return dur.count();
	}
};
