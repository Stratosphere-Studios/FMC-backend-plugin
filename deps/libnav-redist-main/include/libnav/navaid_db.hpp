/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains function declarations for the NavaidDB class. This class serves
    as an interface for x-plane's earth_fix.dat and earth_nav.dat
*/


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
#include "common.hpp"
#include "str_utils.hpp"


namespace libnav
{
	constexpr int N_FIX_COL_NORML_XP12 = 7;
	constexpr int N_FIX_COL_NORML_XP11 = 6;
	constexpr int N_NAVAID_COL_NORML = 11;
	constexpr double VOR_MAX_SLANT_ANGLE_DEG = 40;
	constexpr double DME_DME_PHI_MIN_DEG = 30;
	constexpr double DME_DME_PHI_MAX_DEG = 180 - DME_DME_PHI_MIN_DEG;
	constexpr double MAX_ANG_DEV_MERGE = 0.0006;
	constexpr size_t NAVAID_ENTRY_CACHE_SZ = 300000;


	enum XPLM_navaid_types
	{
		XP_NAV_NDB = 2,
		XP_NAV_VOR = 3,
		XP_NAV_ILS_LOC = 4,
		XP_NAV_ILS_LOC_ONLY = 5,
		XP_NAV_ILS_GS = 6,
		XP_NAV_OM = 7,  // Outer marker
		XP_NAV_MM = 8,  // Middle marker
		XP_NAV_IM = 9,  // Inner marker
		XP_NAV_ILS_FULL = 10,
		XP_NAV_DME = 12,
		XP_NAV_DME_ONLY = 13,
		XP_NAV_VOR_DME = 15,
		XP_NAV_ILS_DME = 18
	};

	NavaidType xp_type_to_libnav(navaid_type_t tp);

	NavaidType make_composite(NavaidType tp1, NavaidType tp2);


	struct navaid_entry_t
	{
		uint16_t max_recv;
		double elev_ft, freq, mag_var;


		bool cmp(navaid_entry_t const& other);

		bool operator==(navaid_entry_t const& other);

		bool operator!=(navaid_entry_t const& other);
	};

	struct waypoint_entry_t
	{
		NavaidType type;
		uint32_t arinc_type = 0;  // Ref: arinc424 spec, section 5.42
		geo::point pos;
		std::string area_code;
		std::string country_code;
		navaid_entry_t* navaid = nullptr;
		

		bool cmp(waypoint_entry_t const& other);

		bool operator==(waypoint_entry_t const& other);

		bool operator!=(waypoint_entry_t const& other);
	};

	struct waypoint_t
	{
		std::string id;
		waypoint_entry_t data;


		/*
			Function: get_awy_id
			Description:
			returns id string in a format used by the air way data base.
			@return: id string
		*/

		std::string get_awy_id();

		/*
			Function: get_hold_id
			Description:
			returns id string in a format used by the hold data base.
			@return: id string
		*/

		std::string get_hold_id();


		bool operator==(waypoint_t const& other);

		bool operator!=(waypoint_t const& other);
	};

	
	typedef bool (*navaid_filter_t)(waypoint_t, void*);

	bool default_navaid_filter(waypoint_t in, void* ref);


	struct wpt_line_t
	// This is used to store the contents of 1 line of earth_nav.dat
    {
        earth_data_line_t data;

		waypoint_t wpt;
        std::string desc;


        wpt_line_t(std::string& s, int db_version);
    };

	struct navaid_line_t
	// This is used to store the contents of 1 line of earth_nav.dat
    {
        earth_data_line_t data;

		waypoint_t wpt;
		navaid_entry_t navaid;
        std::string desc;  // Spoken name of the navaid


        navaid_line_t(std::string& s);
    };


	class WaypointEntryCompare
	{
	public:
		geo::point ac_pos; // Aircraft position
		bool operator()(waypoint_entry_t w1, waypoint_entry_t w2);
	};

	class WaypointCompare
	{
	public:
		geo::point ac_pos; // Aircraft position
		bool operator()(waypoint_t w1, waypoint_t w2);
	};


	typedef std::unordered_map<std::string, 
			std::vector<libnav::waypoint_entry_t>> wpt_db_t;


	class NavaidDB
	{
	public:

		DbErr err_code;


		NavaidDB(std::string wpt_path, std::string navaid_path);

		DbErr get_wpt_err();
		 
		DbErr get_navaid_err();

		int get_wpt_cycle();

		int get_wpt_version();

		int get_navaid_cycle();

		int get_navaid_version();

		DbErr load_waypoints();

		DbErr load_navaids();

		const wpt_db_t& get_db();

		bool is_wpt(std::string id);

		bool is_navaid_of_type(std::string id, NavaidType type);

		// get_wpt_data returns 0 if waypoint is not in the database. 
		// Otherwise, returns number of items written to out.
		size_t get_wpt_data(std::string& id, std::vector<waypoint_entry_t>* out, 
			std::string area_code="", std::string country_code="", 
			NavaidType type=NavaidType::NAVAID, 
			navaid_filter_t filt_func=default_navaid_filter, void* ref=NULL);

		/*
			Function: get_wpt_by_awy_str
			Description:
			Gets all matching waypoints using an id used by airway data base.
			@param awy_str: airway data base id string
			@param out: pointer to the output vector
			@return: number of items written to out.
		*/

		size_t get_wpt_by_awy_str(std::string& awy_str, std::vector<waypoint_entry_t>* out);

		/*
			Function: get_wpt_by_hold_str
			Description:
			Gets all matching waypoints using an id used by hold data base.
			@param awy_str: hold data base id string
			@param out: pointer to the output vector
			@return: number of items written to out.
		*/

		size_t get_wpt_by_hold_str(std::string& hold_str, std::vector<waypoint_entry_t>* out);

		std::string get_fix_desc(waypoint_t& fix);

		void reset();

		~NavaidDB();

	private:
		int wpt_airac_cycle, wpt_db_version;
		int navaid_airac_cycle, navaid_db_version;

		std::string sim_wpt_db_path;
		std::string sim_navaid_db_path;

		std::future<DbErr> wpt_task;
		std::future<DbErr> navaid_task;

		std::mutex wpt_db_mutex;
		std::mutex navaid_db_mutex;

		std::mutex wpt_desc_mutex;
		std::mutex navaid_desc_mutex;

		wpt_db_t wpt_cache;
		navaid_entry_t* navaid_entries;
		size_t n_navaid_entries;

		std::unordered_map<std::string, std::string> wpt_desc_db;
		std::unordered_map<std::string, std::string> navaid_desc_db;


		navaid_entry_t* navaid_entries_add(navaid_entry_t data);

		void add_to_wpt_cache(waypoint_t wpt);

		void add_to_navaid_cache(waypoint_t wpt, navaid_entry_t data);


		static std::string get_fix_unique_ident(waypoint_t& fix);

		static void add_to_map_with_mutex(std::string& id, std::string& desc,
			std::mutex& mtx, std::unordered_map<std::string, std::string>& umap);

		static std::string get_map_val_with_mutex(std::string& id,
			std::mutex& mtx, std::unordered_map<std::string, std::string>& umap);
	};


	std::string navaid_to_str(NavaidType navaid_type);

	void sort_wpt_entry_by_dist(std::vector<waypoint_entry_t>* vec, geo::point p);

	void sort_wpts_by_dist(std::vector<waypoint_t>* vec, geo::point p);

}; // namespace libnav


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
		libnav::waypoint_entry_t data;
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
}; // namespace radnav_util
