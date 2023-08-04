/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains definitions of member functions used within the REF NAV DATA page.
*/

#include "nav_db.hpp"
#include "databus.hpp"

constexpr int N_DME_DME_CAND = 2;
constexpr int N_DME_DME_STA = 32; // Number of DMEs used to make pairs(for DME/DME position)


namespace StratosphereAvionics
{
	struct navaid_tuner_out_drs
	{
		// These are DEBUG-ONLY!
		std::vector<std::string> vor_dme_cand_data;

		std::vector<std::string> dme_dme_cand_data;
	};


	class NavaidTuner
	{
	public:
		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		radnav_util::navaid_t* dme_dme_cand;


		NavaidTuner(std::shared_ptr<XPDataBus::DataBus> databus, navaid_tuner_out_drs out, 
					double tile_size, double navaid_thresh_nm, double dur_sec);

		void set_vor_dme_cand(radnav_util::navaid_t cand);

		radnav_util::navaid_t get_vor_dme_cand();

		void set_dme_dme_cand(radnav_util::navaid_t cand1, radnav_util::navaid_t cand2, double qual);

		radnav_util::navaid_pair_t get_dme_dme_cand();

		void update_dme_dme_cand(geo::point ac_pos, std::vector<radnav_util::navaid_t>* navaids);

		/*
			The following member function updates VOR DME and DME DME candidates.
		*/

		void update_rad_nav_cand(geo::point3d ac_pos);

		void update(libnav::wpt_db_t* ptr, geo::point3d ac_pos, double c_time_sec);

		~NavaidTuner();

	private:
		geo::point3d ac_pos_last;

		std::unordered_map<std::string, double> black_list;

		libnav::wpt_db_t* wpt_db;

		libnav::wpt_tile_t navaid_cache;

		radnav_util::navaid_t vor_dme_cand;

		radnav_util::navaid_pair_t dme_dme_cand_pair;

		std::mutex vor_dme_cand_mutex;
		std::mutex dme_dme_cand_mutex;
		std::mutex black_list_mutex;

		double cache_tile_size, min_navaid_dist_nm, 
			cand_update_dur_sec, cand_update_last_sec;

		navaid_tuner_out_drs out_drs;


		bool is_black_listed(libnav::waypoint wpt, double c_time_sec);
		
		void update_navaid_cache(geo::point ac_pos);
	};
};
