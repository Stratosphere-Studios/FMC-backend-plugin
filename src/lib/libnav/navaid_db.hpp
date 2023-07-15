#pragma once

#include <fstream>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <cstring>
#include "common.hpp"


#define N_NAVAID_LINES_IGNORE 3; //Number of lines at the beginning of the .dat file to ignore

enum navaid_types {
	NAV_NDB = 2,
	NAV_VOR = 3,
	NAV_ILS_LOC = 4,
	NAV_ILS_LOC_ONLY = 5,
	NAV_ILS_GS = 6,
	NAV_ILS_FULL = 10,
	NAV_DME = 12,
	NAV_DME_ONLY = 13,
	NAV_VOR_DME = 15,
	NAV_ILS_DME = 18
};


namespace libnav
{
	struct navaid_entry
	{
		uint16_t type, max_recv;
		geo::point wpt;
		double elevation, mag_var, freq;
	};


	class NavaidDB
	{
	public:

		NavaidDB(std::string wpt_path, std::string navaid_path,
			std::unordered_map<std::string, std::vector<geo::point>>* wpt_db,
			std::unordered_map<std::string, std::vector<navaid_entry>>* navaid_db);

		int get_load_status();

		int load_waypoints();

		int load_navaids();

		bool is_wpt(std::string id);

		// get_wpt_info returns 0 if waypoint is not in the database. 
		// Otherwise, returns number of items written to out.
		size_t get_wpt_info(std::string id, std::vector<geo::point>* out);

		bool is_navaid(std::string id);

		// get_navaid_info returns 0 if waypoint is not in the database. 
		// Otherwise, returns number of items written to out.
		size_t get_navaid_info(std::string id, std::vector<navaid_entry>* out);

		~NavaidDB();

	private:
		int comp_types[NAV_ILS_DME + 1];
		int max_comp = NAV_ILS_DME;
		std::string sim_wpt_db_path;
		std::string sim_navaid_db_path;

		std::future<int> wpt_loaded;
		std::future<int> navaid_loaded;

		std::mutex wpt_db_mutex;
		std::mutex navaid_db_mutex;

		std::unordered_map<std::string, std::vector<geo::point>>* wpt_cache;
		std::unordered_map<std::string, std::vector<navaid_entry>>* navaid_cache;
	};
}
