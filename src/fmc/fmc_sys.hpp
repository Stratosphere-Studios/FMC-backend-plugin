/*
	This header file contains definitions of classes, functions, etc 
	used in the fmc implementation.
*/

#pragma once

#include "dr_cache.hpp"
#include "databus.hpp"
#include "nav_db.hpp"
#include <cstring>


enum fmc_pages
{
	N_CDU_OUT_LINES = 6,

	PAGE_OTHER = 0,
	PAGE_RTE1 = 1,
	PAGE_REF_NAV_DATA = 2
};


namespace StratosphereAvionics
{
	struct avionics_out_drs
	{
		std::string dep_icao, arr_icao;
		std::string dep_rnw;

		std::vector<std::string> excl_navaids;
		std::vector<std::string> excl_vors;
	};

	struct fmc_ref_nav_in_drs
	{
		std::string poi_id, rad_nav_inh;

		std::vector<std::string> in_navaids;
		std::vector<std::string> in_vors;
	};

	struct fmc_ref_nav_out_drs
	{
		std::string poi_id, poi_type, poi_lat, poi_lon, poi_elevation,
			poi_freq, poi_mag_var;
	};

	struct fmc_rte1_drs
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
		int not_in_db_idx;
		std::vector<std::string> dr_list;
	};

	struct fmc_in_drs
	{
		// Sim data refs:
		std::string sim_ac_lat_deg, sim_ac_lon_deg;

		// Custom data refs:
		fmc_ref_nav_in_drs ref_nav;
		fmc_rte1_drs rte1;
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


	struct flightplan
	{
		libnav::airport dep_apt, arr_apt;
		libnav::runway dep_rnw, arr_rnw;
	};


	class AvionicsSys 
	{
	public:
		char path_sep[2];
		std::string xplane_path;
		std::string prefs_path;
		std::string sim_apt_path;
		std::string default_data_path;
		int xplane_version;
		std::atomic<bool> sim_shutdown{ false };

		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		std::unordered_map<std::string, std::vector<libnav::waypoint_entry>> waypoints;
		std::unordered_map<std::string, libnav::airport_data> airports;
		std::unordered_map<std::string, libnav::runway_data> runways;

		libnav::ArptDB* apt_db;
		libnav::NavaidDB* navaid_db;
		libnav::NavDB* nav_db;

		AvionicsSys(std::shared_ptr<XPDataBus::DataBus> databus, avionics_out_drs out);

		void set_fpln_dep_apt(libnav::airport apt);

		void set_fpln_arr_apt(libnav::airport apt);

		void set_fpln_dep_rnw(libnav::runway rnw);

		void set_fpln_arr_rnw(libnav::runway rnw);

		void excl_navaid(std::string id, int idx);

		void excl_vor(std::string id, int idx);

		void update_sys();

		void main_loop();

		~AvionicsSys();

	private:
		avionics_out_drs out_drs;

		std::mutex fpln_mutex;
		std::mutex navaid_inhibit_mutex;
		std::mutex vor_inhibit_mutex;

		flightplan pln;

		std::vector<std::string> navaid_inhibit = { "", "" };
		std::vector<std::string> vor_inhibit = { "", "" };

		XPDataBus::DataRefCache* dr_cache;

		void update_load_status();
	};

	class FMC
	{
	public:
		std::atomic<bool> sim_shutdown{ false };

		FMC(std::shared_ptr<AvionicsSys> av, fmc_in_drs in, fmc_out_drs out);

		geo::point get_ac_pos();

		libnav::waypoint_entry update_sel_des_wpt(std::string id,
							  std::vector<libnav::waypoint_entry> vec); // Updates SELECT DESIRED WPT page for navaids

		void reset_sel_navaid();

		int update_ref_nav_poi_data(size_t n_arpts_found, size_t n_wpts_found, std::string icao,
									libnav::airport_data arpt_found, std::vector<libnav::waypoint_entry> wpts_found);

		void reset_ref_nav_poi_data(std::vector<std::string>* nav_drs);

		void update_ref_nav_inhibit(std::vector<std::string>* nav_drs, std::vector<int> types,
									int threshold, bool add_vor);

		void update_ref_nav(); // Updates REF NAV DATA page

		void reset_ref_nav();

		/*
			Updates airport icao codes on any rte page.
			If user has entered a valid icao, returns true.
			Otherwise, returns false.
		*/

		bool update_rte_apt(std::string in_dr, libnav::airport_data* apt_data, libnav::runway_data* rnw_data); 

		void update_rte1();

		void update_scratch_msg(); // Updates scratch pad messages

		void main_loop();

		~FMC();

	private:
		libnav::NavDB* nav_db;
		std::shared_ptr<AvionicsSys> avionics;
		fmc_in_drs in_drs;
		fmc_out_drs out_drs;

		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		XPDataBus::DataRefCache* dr_cache;
	};
}
