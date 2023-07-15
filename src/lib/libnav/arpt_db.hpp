#pragma once

#include <future>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <iterator>
#include "common.hpp"


#define N_ARPT_LINES_IGNORE 3;
#define N_RNW_ITEMS_IGNORE_BEGINNING 8; // Number of items to ignore at the beginning of the land runway declaration.
#define N_RNW_ITEMS_IGNORE_END 5;
#define N_DOUBLE_OUT_PRECISION 9; // Number of indices after the decimal in the string representation of a double number
#define MIN_RWY_LENGTH_M 2000; // If the longest runway of the airport is less than this, the airport will not be included in the database


enum xplm_arpt_row_codes {
	LAND_ARPT = 1,
	MISC_DATA = 1302,
	LAND_RUNWAY = 100,
	DB_EOF = 99
};


namespace libnav
{
	struct runway_entry
	{
		geo::point start, end;
		int displ_threshold_m;
		double impl_length_m = -1;

		double get_implied_length_meters()
		{
			if (impl_length_m <= 0)
			{
				impl_length_m = start.getGreatCircleDistanceNM(end) * NM_TO_M;
			}
			return impl_length_m;
		}
	};

	typedef std::unordered_map<std::string, runway_entry> runway_data;

	struct runway
	{
		std::string id;
		runway_entry data;
	};

	struct airport_data
	{
		geo::point pos;
		uint32_t elevation_ft, transition_alt_ft, transition_level;
	};

	struct airport_entry
	{
		std::unordered_map<std::string, runway_entry> runways;
		airport_data data;
	};

	//These structs are only used to load the database.

	struct arpt_data
	{
		std::string icao;
		airport_data data;
	};

	struct rnw_data
	{
		std::string icao;
		std::vector<runway> runways;
	};


	class ArptDB
	{
	public:
		// Aircraft latitude and longitude

		double ac_lat;
		double ac_lon;

		ArptDB(std::unordered_map<std::string, airport_data>* a_db, std::unordered_map<std::string, runway_data>* r_db,
			std::string sim_arpt_path, std::string custom_arpt_path, std::string custom_rnw_path, double lat, double lon);

		int get_load_status();

		//These functions need to be public because they're used in 
		//other threads when ArptDB object is constructed.

		int load_from_sim_db();

		void write_to_arpt_db();

		void write_to_rnw_db();

		void load_from_custom_arpt(); // Load data from custom airport database

		void load_from_custom_rnw(); // Load data from custom runway database

		// Normal user interface functions:

		bool is_airport(std::string icao_code);

		size_t get_airport_data(std::string icao_code, airport_data* out);

		size_t get_runway_data(std::string icao_code, runway_data* out);

	private:
		std::string custom_arpt_db_sign = "ARPTDB";
		std::string custom_rnw_db_sign = "RNWDB";
		bool apt_db_created = false;
		bool rnw_db_created = false;

		// Data for creating a custom airport database

		std::atomic<bool> write_arpt_db{ false };

		std::vector<arpt_data> arpt_queue;
		std::vector<rnw_data> rnw_queue;

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

		std::unordered_map<std::string, airport_data>* arpt_db;
		std::unordered_map<std::string, std::unordered_map<std::string, runway_entry>>* rnw_db;

		static bool does_db_exist(std::string path, std::string sign);

		double parse_runway(std::string line, std::vector<runway>* rnw); // Returns runway length in meters

		void add_to_arpt_queue(arpt_data arpt);

		void add_to_rnw_queue(rnw_data rnw);
	};
}
