#pragma once

#include <fstream>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <string>
#include <sstream>
#include "geo_utils.hpp"


#define N_NAVAID_LINES_IGNORE 3; //Number of lines at the beginning of the .dat file to ignore


enum navaid_types {
	NAV_NONE = 0,
	NAV_WAYPOINT = 1,
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
		uint16_t max_recv;
		double elevation, freq;
	};

	struct waypoint_entry
	{
		uint16_t type;
		geo::point pos;
		navaid_entry* navaid;
	};

	struct waypoint
	{
		std::string id;
		waypoint_entry data;
	};


	class WaypointEntryCompare
	{
	public:
		geo::point ac_pos; // Aircraft position
		bool operator()(waypoint_entry w1, waypoint_entry w2);
	};

	class WaypointCompare
	{
	public:
		geo::point ac_pos; // Aircraft position
		bool operator()(waypoint w1, waypoint w2);
	};

	class NavaidDB
	{
	public:

		NavaidDB(std::string wpt_path, std::string navaid_path,
			std::unordered_map<std::string, std::vector<waypoint_entry>>* wpt_db);

		bool is_loaded();

		int load_waypoints();

		int load_navaids();

		bool is_wpt(std::string id);

		bool is_navaid_of_type(std::string id, std::vector<int> types);

		// get_wpt_data returns 0 if waypoint is not in the database. 
		// Otherwise, returns number of items written to out.
		size_t get_wpt_data(std::string id, std::vector<waypoint_entry>* out);

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

		std::unordered_map<std::string, std::vector<waypoint_entry>>* wpt_cache;

		void add_to_wpt_cache(waypoint wpt);

		void add_to_navaid_cache(waypoint wpt, navaid_entry data);
	};


	std::string navaid_to_str(int navaid_type);

	void sort_wpt_entry_by_dist(std::vector<waypoint_entry>* vec, geo::point p);

	void sort_wpts_by_dist(std::vector<waypoint>* vec, geo::point p);

};


namespace radnav_util
{
	struct navaid
	{
		std::string id;
		libnav::waypoint_entry data;
		double qual, time_blacklisted_sec;
		bool is_blacklisted;
	};
};
