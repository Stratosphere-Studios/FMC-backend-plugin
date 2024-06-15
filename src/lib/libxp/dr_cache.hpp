/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file provides declarations of member functions of DataRefCache class
	Author: discord/bruh4096#4512(Tim G.)
*/


#pragma once

#include "common.hpp"
#include <unordered_map>
#include <mutex>


namespace XPDataBus
{
	class DataRefCache
	{
	public:
		DataRefCache();

		generic_val get_val(std::string dr_name);

		int get_val_i(std::string dr_name);

		std::string get_val_s(std::string dr_name);

		void set_val(std::string dr_name, generic_val val);

		void set_val_i(std::string dr_name, int in);

		void set_val_s(std::string dr_name, std::string in);
	private:
		std::unordered_map<std::string, generic_val> cache;

		std::mutex dr_cache_mutex;
	};
}
