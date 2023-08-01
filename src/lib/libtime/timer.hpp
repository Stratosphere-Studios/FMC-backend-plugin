#include <chrono>


namespace libtime
{
	struct Timer
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> t_start;

		Timer();

		double get_curr_time();
	};
};

