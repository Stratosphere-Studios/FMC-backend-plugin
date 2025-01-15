/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains declarations of member functions for ArptDB class. ArptDB is an interface which allows
	to create a custom airport data base using x-plane's apt.dat. The class also allows you to access the data
	base and perform some searches on it.
*/


#pragma once

#include <future>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <iterator>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctype.h>
#include "str_utils.hpp"
#include "geo_utils.hpp"
#include "common.hpp"


namespace libnav
{
	constexpr double DB_VERSION = 1.7; // Change this if you want to rebuild runway and airport data bases
	constexpr int N_ARPT_LINES_IGNORE = 3;
	// N_HEADER_STR_WORDS is the number of words in a string declaring the data base
	// version.
	constexpr int N_HEADER_STR_WORDS = 4;
	// Number of items to ignore at the beginning of the land runway declaration.
	constexpr int N_RNW_ITEMS_IGNORE_BEGINNING = 8;
	constexpr int N_RNW_ITEMS_IGNORE_END = 5;
	// Number of indices after the decimal in the string representation of a double number
	constexpr int N_DOUBLE_OUT_PRECISION = 9;
	// If the longest runway of the airport is less than this, the airport will not be included in the database
	constexpr double MIN_RWY_LENGTH_M = 1000;
	constexpr char DEFAULT_COMMENT_CHAR = '#';


	enum class XPLMArptRowCode 
	{
		LAND_ARPT = 1,
		MISC_DATA = 1302,
		LAND_RUNWAY = 100,
		DB_EOF = 99
	};


	struct runway_entry_t
	{
		geo::point start, end;
		int displ_threshold_m;
		double impl_length_m = -1;

		double get_impl_length_m()
		{
			if (impl_length_m <= 0)
			{
				impl_length_m = start.get_gc_dist_nm(end) * geo::NM_TO_M;
			}
			return impl_length_m;
		}
	};

	typedef std::unordered_map<std::string, runway_entry_t> runway_data;

	struct runway_t
	{
		std::string id;
		runway_entry_t data;
	};

	struct airport_data_t
	{
		geo::point pos;
		uint32_t elevation_ft, transition_alt_ft, transition_level;
	};

	struct airport_entry_t
	{
		std::unordered_map<std::string, runway_entry_t> runways;
		airport_data_t data;
	};

	struct airport_t
	{
		std::string icao;
		airport_data_t data;
	};

	struct rnw_data_t
	{
		std::string icao; //Airport icao
		std::vector<runway_t> runways;
	};


	typedef std::unordered_map<std::string, airport_data_t> airport_db_t;
	typedef std::unordered_map<std::string, 
		std::unordered_map<std::string, runway_entry_t>> rnw_db_t;


	class ArptDB
	{
		typedef std::pair<std::string, airport_data_t> str_arpt_data_t;
		typedef std::pair<std::string, std::unordered_map<std::string, runway_entry_t>> 
			str_rnw_t;

	public:
		DbErr err_code;

		ArptDB(std::string sim_arpt_path, std::string custom_arpt_path,
			std::string custom_rnw_path, double min_rwy_l_m = MIN_RWY_LENGTH_M);

		DbErr get_err();

		const airport_db_t& get_arpt_db();

		const rnw_db_t& get_rnw_db();

		//These functions need to be public because they're used in 
		//other threads when ArptDB object is constructed.

		int load_from_sim_db();

		void write_to_arpt_db();

		void write_to_rnw_db();

		void load_from_custom_arpt(); // Load data from custom airport database

		void load_from_custom_rnw(); // Load data from custom runway database

		// Normal user interface functions:

		bool is_airport(std::string icao_code);

		bool get_airport_data(std::string icao_code, airport_data_t* out);

		int get_apt_rwys(std::string icao_code, runway_data* out);

		int get_rnw_data(std::string apt_icao, std::string rnw_id, runway_entry_t* out);

	private:
		int db_version;  // May be used later
		double min_rwy_length_m;

		std::string custom_arpt_db_sign = "ARPTDB";
		std::string custom_rnw_db_sign = "RNWDB";
		bool apt_db_created = false;
		bool rnw_db_created = false;

		// Data for creating a custom airport database

		std::atomic<bool> write_arpt_db{ false };

		std::vector<airport_t> arpt_queue;
		std::vector<rnw_data_t> rnw_queue;

		std::mutex arpt_queue_mutex;
		std::mutex rnw_queue_mutex;

		std::mutex arpt_db_mutex;
		std::mutex rnw_db_mutex;

		std::string sim_arpt_db_path;
		std::string custom_arpt_db_path;
		std::string custom_rnw_db_path;

		std::future<int> sim_db_loaded;
		std::future<void> arpt_db_task;
		std::future<void> rnw_db_task;

		airport_db_t arpt_db;
		rnw_db_t rnw_db;

		static bool does_db_exist(std::string path, std::string sign);

		static int get_db_version(std::string& line);

		double parse_runway(std::string line, std::vector<runway_t>* rnw); // Returns runway length in meters

		void add_to_arpt_queue(airport_t arpt);

		void add_to_rnw_queue(rnw_data_t rnw);
	};
} // namespace libnav
