/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains declarations of member functions for NavaidSelector class. NavaidSelector class updates
	VOR/DME as well as DME/DME candidates, which are then tuned by NavaidTuner. These navaids are used to tune
	the position provided by IRS.
*/

#pragma once

#include "navaid_tuner.hpp"
#include <thread>


namespace StratosphereAvionics
{
	struct navaid_selector_out_drs
	{
		// These are DEBUG-ONLY!
		std::vector<std::string> vor_dme_cand_data;

		std::vector<std::string> dme_dme_cand_data;
	};


	class NavaidSelector
	{
	public:
		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		NavaidTuner* navaid_tuner;


		NavaidSelector(std::shared_ptr<XPDataBus::DataBus> databus, NavaidTuner* tuner, navaid_selector_out_drs out,
			double tile_size, double navaid_thresh_nm, int dur_sec);

		void update_dme_dme_cand(geo::point ac_pos, std::vector<radnav_util::navaid_t>* navaids);

		/*
			The following member function updates VOR DME and DME DME candidates.
		*/

		void update_rad_nav_cand(geo::point3d ac_pos);

		void update(libnav::wpt_db_t* ptr, geo::point3d ac_pos, double c_time_sec);

		~NavaidSelector();

	private:
		geo::point3d ac_pos_last;

		libnav::wpt_db_t* wpt_db;

		libnav::wpt_tile_t navaid_cache;

		radnav_util::navaid_t vor_dme_cand;

		radnav_util::navaid_pair_t dme_dme_cand_pair;

		std::mutex vor_dme_cand_mutex;
		std::mutex dme_dme_cand_mutex;

		int cand_update_dur_sec;

		double cache_tile_size, min_navaid_dist_nm,
			cand_update_last_sec;

		navaid_selector_out_drs out_drs;


		void update_navaid_cache(geo::point ac_pos);
	};
}
