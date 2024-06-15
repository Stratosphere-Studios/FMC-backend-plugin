/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This source file contains definitions of classes, functions, etc 
	used in the timer implementation. Author: discord/bruh4096#4512(Tim G.)
*/


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
