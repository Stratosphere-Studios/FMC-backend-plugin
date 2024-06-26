/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file contains declarations of classes, functions, etc 
	used in the avionics implementation. Author: discord/bruh4096#4512(Tim G.)
*/


#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <libxp/dr_cache.hpp>
#include <libxp/databus.hpp>
#include <libnav/navaid_db.hpp>
#include <libnav/arpt_db.hpp>
#include <libtime/timer.hpp>
#include "rad_nav/navaid_selector.hpp"



namespace StratosphereAvionics
{
	// Minimal distance to navaid for radio navigation.

	constexpr double min_navaid_dist_nm = 160;

	// Period in seconds after which the radio navigation 
	// candidates will be re-examined

	constexpr int rad_nav_cand_update_time_sec = 5;


	struct avionics_in_drs
	{
		std::string sim_baro_alt_ft1, sim_baro_alt_ft2, sim_baro_alt_ft3;
		std::string sim_ac_lat_deg, sim_ac_lon_deg;

		navaid_tuner_in_drs nav_tuner;
	};

	struct avionics_out_drs
	{
		std::string dep_icao, arr_icao;
		std::string dep_rnw;

		std::vector<std::string> excl_navaids;
		std::vector<std::string> excl_vors;

		navaid_tuner_out_drs nav_tuner;
		navaid_selector_out_drs nav_selector;
	};

	struct flightplan
	{
		libnav::airport_t dep_apt, arr_apt;
		libnav::runway_t dep_rnw, arr_rnw;
	};


	/*
		class: AvionicsSys
		Description:
		This class provides the mutual resources for the FMCs.
		For example, it stores the flight plan, handles automatic navaid tuning and communicates with other systems.
	*/

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
		
		std::shared_ptr<libnav::ArptDB> apt_db;
		std::shared_ptr<libnav::NavaidDB> navaid_db;


		AvionicsSys(std::shared_ptr<XPDataBus::DataBus> databus, avionics_in_drs in, avionics_out_drs out,
			double cache_tile_size, int hz);

		geo::point3d get_ac_pos();

		void set_fpln_dep_apt(libnav::airport_t apt);

		std::string get_fpln_dep_icao();

		void set_fpln_arr_apt(libnav::airport_t apt);

		libnav::airport_t get_fpln_arr_apt();

		std::string get_fpln_arr_icao();

		void set_fpln_dep_rnw(libnav::runway_t rnw);

		void set_fpln_arr_rnw(libnav::runway_t rnw);

		void excl_navaid(std::string id, size_t idx);

		void excl_vor(std::string id, size_t idx);

		void update_sys();

		void main_loop();

		void disable();

		~AvionicsSys();

	private:
		int n_refresh_hz;
		double tile_size;

		avionics_in_drs in_drs;
		avionics_out_drs out_drs;

		std::mutex fpln_mutex;
		std::mutex navaid_inhibit_mutex;
		std::mutex vor_inhibit_mutex;
		std::mutex ac_pos_mutex;

		flightplan pln;

		std::vector<std::string> navaid_inhibit = { "", "" };
		std::vector<std::string> vor_inhibit = { "", "" };

		geo::point3d ac_pos;
		geo::point3d ac_pos_last;

		libtime::Timer* clock;

		XPDataBus::DataRefCache* dr_cache;
		NavaidTuner* navaid_tuner;
		NavaidSelector* navaid_selector;

		libnav::wpt_db_t waypoints;


		void update_load_status();

		void update_ac_pos();

		/*
			Blacklists all navaids with given id forever.
		*/

		void add_to_bl(std::string id);

		/*
			Removes all navaids with given id from black list.
		*/

		void rm_from_bl(std::string id);
	};
}; // namespace StratosphereAvionics
