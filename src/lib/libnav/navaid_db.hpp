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


constexpr double VOR_MAX_SLANT_ANGLE_DEG = 40;
constexpr double DME_DME_PHI_MIN_DEG = 30;
constexpr double DME_DME_PHI_MAX_DEG = 180 - DME_DME_PHI_MIN_DEG;
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

	typedef std::vector<libnav::waypoint> wpt_tile_t;

	typedef std::unordered_map<std::string, std::vector<libnav::waypoint_entry>> wpt_db_t;


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

		NavaidDB(std::string wpt_path, std::string navaid_path, wpt_db_t* wpt_db);

		bool is_loaded();

		int load_waypoints();

		int load_navaids();

		bool is_wpt(std::string id);

		bool is_navaid_of_type(std::string id, std::vector<int> types);

		// get_wpt_data returns 0 if waypoint is not in the database. 
		// Otherwise, returns number of items written to out.
		int get_wpt_data(std::string id, std::vector<waypoint_entry>* out);

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

		wpt_db_t* wpt_cache;

		void add_to_wpt_cache(waypoint wpt);

		void add_to_navaid_cache(waypoint wpt, navaid_entry data);
	};


	std::string navaid_to_str(int navaid_type);

	void sort_wpt_entry_by_dist(std::vector<waypoint_entry>* vec, geo::point p);

	void sort_wpts_by_dist(std::vector<waypoint>* vec, geo::point p);

};


namespace radnav_util
{
	/*
		The following function returns a fom in nm for a DME using a formula
		from RTCA DO-236C appendix C-3. The only argument is the total distance to 
		the station.
	*/

	double get_dme_fom(double dist_nm);

	/*
		The following function returns a fom in nm for a VOR using a formula
		from RTCA DO-236C appendix C-2.The only argument is the total distance to 
		the station.
	*/

	double get_vor_fom(double dist_nm);

	/*
		The following function returns a fom in nm for a VOR DME station.
		It accepts the total distance to the station as its only argument.
	*/

	double get_vor_dme_fom(double dist_nm);

	/*
		This function calculates a quality value for a pair of navaids given
		the encounter geometry angle and their respective qualities.
	*/

	/*
		Function: get_dme_dme_fom
		Description:
		This function calculates a FOM value for a pair of navaids given
		the encounter geometry angle and their respective distances.
		Param:
		dist1_nm: quality value of the first DME
		dist2_nm: quality value of the second DME
		phi_rad: encounter geometry angle between 2 DMEs
		Return:
		Returns a FOM value.
	*/

	double get_dme_dme_fom(double dist1_nm, double dist2_nm, double phi_rad);

	double get_dme_dme_qual(double phi_deg, double q1, double q2);


	struct navaid_t
	{
		std::string id;
		libnav::waypoint_entry data;
		double qual;

		/*
			This function calculates the quality ratio for a navaid.
			Navaids are sorted by this ratio to determine the best 
			suitable candidate(s) for radio navigation.
		*/

		void calc_qual(geo::point3d ac_pos);
	};

	struct navaid_pair_t
	{
		navaid_t* n1;
		navaid_t* n2;
		double qual;

		/*
			This function calculates a quality value for a pair of navaids.
			This is useful when picking candidates for DME/DME position calculation.
		*/

		void calc_qual(geo::point ac_pos);
	};
};
