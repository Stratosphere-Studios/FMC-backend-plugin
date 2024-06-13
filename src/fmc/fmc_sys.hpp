/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file contains definitions of classes, functions, etc 
	used in the fmc implementation. Author: discord/bruh4096#4512
*/

#pragma once

#include <libxp/dr_cache.hpp>
#include <libxp/databus.hpp>
#include <libtime/timer.hpp>
#include <cstring>
#include "avionics/avionics.hpp"
#include "avionics/rad_nav/navaid_selector.hpp"


namespace StratosphereAvionics
{
	enum POI_types
	{
		POI_NULL = 0,
		POI_WAYPOINT = 2,
		POI_NAVAID = 3,
		POI_AIRPORT = 5,
		POI_RWY = 7
	};

	enum class fmc_pages
	{
		PAGE_OTHER = 0,
		PAGE_RTE1 = 1,
		PAGE_REF_NAV_DATA = 2
	};

	enum class ref_nav
	{
		RAD_NAV_INHIBIT = 0,
		RAD_NAV_VOR_ONLY_INHIBIT = 1,
		RAD_NAV_NO_INHIBIT = 2
	};

	constexpr int N_CDU_OUT_LINES = 6;


	struct fmc_ref_nav_in_drs
	{
		std::string poi_id, rad_nav_inh;

		std::vector<std::string> in_navaids;
		std::vector<std::string> in_vors;
	};

	struct fmc_ref_nav_out_drs
	{
		std::string poi_id, poi_type, poi_lat, poi_lon, poi_elevation,
			poi_freq, poi_mag_var, poi_length_ft, poi_length_m;
	};

	struct fmc_rte_drs
	{
		std::string dep_icao, arr_icao, dep_rnw;
	};

	struct fmc_sel_desired_wpt_in_drs
	{
		std::string curr_page, poi_idx;
	};

	struct fmc_sel_desired_wpt_out_drs
	{
		std::string is_active, n_subpages, n_pois, poi_list;

		std::vector<std::string> poi_types;
	};

	struct scratchpad_drs
	{
		size_t not_in_db_idx;
		std::vector<std::string> dr_list;
	};

	struct fmc_in_drs
	{
		// Sim data refs:
		std::string sim_ac_lat_deg, sim_ac_lon_deg;

		// Custom data refs:
		fmc_ref_nav_in_drs ref_nav;
		fmc_rte_drs rte1;
		fmc_sel_desired_wpt_in_drs sel_desired_wpt;
		std::string scratch_pad_msg_clear, curr_page;
	};

	struct fmc_out_drs
	{
		// REF NAV DATA
		fmc_ref_nav_out_drs ref_nav;

		// SELECT DESIRED WPT
		fmc_sel_desired_wpt_out_drs sel_desired_wpt;

		// MISC
		scratchpad_drs scratch_msg;
	};
	

	class FMC
	{
	public:
		std::atomic<bool> sim_shutdown{ false };

		FMC(std::shared_ptr<AvionicsSys> av, fmc_in_drs in, fmc_out_drs out, int hz);

		geo::point get_ac_pos();

		libnav::waypoint_entry_t update_sel_des_wpt(std::string id,
							  std::vector<libnav::waypoint_entry_t> vec); // Updates SELECT DESIRED WPT page for navaids

		void reset_sel_navaid();

		int update_ref_nav_poi_data(int n_arpt_found, int n_rwys_found, int n_wpts_found, std::string icao,
									libnav::airport_data_t arpt_found, libnav::runway_entry_t rwy_found,
									std::vector<libnav::waypoint_entry_t> wpts_found);

		void reset_ref_nav_poi_data(std::vector<std::string>* nav_drs);

		void update_ref_nav_inhibit(std::vector<std::string>* nav_drs, libnav::NavaidType types,
									ref_nav threshold, bool add_vor);

		int update_ref_nav(std::string icao); // Updates REF NAV DATA page

		void ref_nav_main_loop(); // Updates REF NAV DATA page

		void reset_ref_nav();

		/*
			Updates airport icao codes on any rte page.
			If user has entered a valid icao, returns true.
			Otherwise, returns false.
		*/

		bool update_rte_apt(std::string in_dr, libnav::airport_data_t* apt_data, 
			libnav::runway_data* rnw_data);

		void update_rte1();

		void update_scratch_msg(); // Updates scratch pad messages

		void main_loop();

		void disable();

		~FMC();

	private:
		int n_refresh_hz;

		std::shared_ptr<libnav::NavaidDB> navaid_db;
		std::shared_ptr<libnav::ArptDB> apt_db;
		std::shared_ptr<AvionicsSys> avionics;
		fmc_in_drs in_drs;
		fmc_out_drs out_drs;

		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		XPDataBus::DataRefCache* dr_cache;


		int get_arrival_rwy_data(std::string rwy_id, libnav::runway_entry_t* out);
	};
}
