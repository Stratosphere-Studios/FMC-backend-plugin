/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file contains declarations of classes, functions, etc 
	used in the timer implementation. Author: discord/bruh4096#4512(Tim G.)
*/


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

