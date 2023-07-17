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
	PAGE_RTE = 1,
	PAGE_REF_NAV_DATA = 2
};


namespace StratosphereAvionics
{
	struct fmc_ref_nav_in_drs
	{
		std::string poi_id;
	};

	struct fmc_ref_nav_out_drs
	{
		std::string poi_id, poi_type, poi_lat, poi_lon, poi_elevation,
			poi_freq;
	};

	struct fmc_sel_desired_wpt_in_drs
	{
		std::string poi_idx;
	};

	struct fmc_sel_desired_wpt_out_drs
	{
		std::string poi_list;
	};

	struct fmc_in_drs
	{
		// Sim data refs:
		std::string sim_ac_lat_deg, sim_ac_lon_deg;

		// Custom data refs:
		fmc_ref_nav_in_drs ref_nav;
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
		std::string scratchpad_msg;
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
		std::atomic<bool> sim_shutdown{false};

		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		std::unordered_map<std::string, std::vector<libnav::navaid_entry>> navaids;
		std::unordered_map<std::string, std::vector<geo::point>> waypoints;

		std::unordered_map<std::string, libnav::airport_data> airports;
		std::unordered_map<std::string, libnav::runway_data> runways;

		libnav::ArptDB* apt_db;
		libnav::NavaidDB* navaid_db;
		libnav::NavDB* nav_db;

		AvionicsSys(std::shared_ptr<XPDataBus::DataBus> databus);

		void update_sys();

		void main_loop();

		~AvionicsSys();

	private:
		std::string icao_entry_last;

		void update_load_status();
	};

	class FMC
	{
	public:
		std::atomic<bool> sim_shutdown{ false };

		FMC(std::shared_ptr<AvionicsSys> av, fmc_in_drs* in, fmc_out_drs* out);

		void clear_screen();

		libnav::navaid_entry update_sel_navaid(std::string id,
							  std::vector<libnav::navaid_entry>* vec); // Updates SELECT DESIRED WPT page for navaids

		void update_ref_nav(); // Updates REF NAV DATA page

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
