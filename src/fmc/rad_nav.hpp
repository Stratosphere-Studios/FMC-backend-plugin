/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains definitions of member functions used within the REF NAV DATA page.
*/

#include "nav_db.hpp"
#include "databus.hpp"
#include "timer.hpp"
#include <cstring>


namespace StratosphereAvionics
{
	constexpr int N_DME_DME_CAND = 2;
	constexpr int N_DME_DME_STA = 32; // Number of DMEs used to make pairs(for DME/DME position)
	constexpr double NAVAID_PROHIBIT_PERMANENT = -1;
	constexpr double NAVAID_MAX_QUAL_DIFF = 0.2; // If the difference in quality between candidate and current station is greater than this, the candidate(s) gets tuned.
	constexpr int ILS_NAVAID_ID_LENGTH = 4;
	
	constexpr int N_VOR_DME_RADIOS = 2; // Number of radios auto-tuned for VOR/DME position estimation
	constexpr int N_DME_DME_RADIOS = 2; // Number of radios auto-tuned for DME/DME position estimation
	constexpr int N_VHF_NAV_RADIOS = N_VOR_DME_RADIOS + N_DME_DME_RADIOS;
	constexpr std::memory_order UPDATE_FLG_ORDR = std::memory_order::memory_order_relaxed;
	constexpr double RADIO_TUNE_DELAY_SEC = 2;
	constexpr double NAVAID_RETRY_DELAY_SEC = 10;
	constexpr double NAVAID_BLACK_LIST_DUR_SEC = 60;


	enum nav_vhf_radio_modes
	{
		NAV_VHF_AUTO = 0,
		NAV_VHF_MAN = 1
	};


	struct radio_drs_t
	{
		std::string freq, nav_id, dme_id, vor_deg, dme_nm;
		int freq_idx;
	};

	struct vhf_radio_t
	{
		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		radio_drs_t dr_list;

		radnav_util::navaid_t tuned_navaid;

		double last_tune_time_sec;

		bool conn_retry;


		vhf_radio_t(std::shared_ptr<XPDataBus::DataBus> databus, radio_drs_t drs);

		void tune(radnav_util::navaid_t new_navaid, double c_time);

		bool is_sig_recv(int expected_type);

		/*
			The following function returns quality of a tuned navaid.
			It gets aircraft position as input.
		*/

		double get_tuned_qual(geo::point3d ac_pos);
	};

	struct navaid_tuner_in_drs
	{
		radio_drs_t sim_radio_drs[N_VHF_NAV_RADIOS];
	};

	struct navaid_tuner_out_drs
	{
		std::string vor_dme_pos_lat, vor_dme_pos_lon, vor_dme_pos_fom;
	};

	struct navaid_selector_out_drs
	{
		// These are DEBUG-ONLY!
		std::vector<std::string> vor_dme_cand_data;

		std::vector<std::string> dme_dme_cand_data;
	};


	class BlackList
	{
	public:
		BlackList();

		void add_to_black_list(libnav::waypoint* wpt, double bl_dur = NAVAID_PROHIBIT_PERMANENT);

		void remove_from_black_list(libnav::waypoint* wpt);

		bool is_black_listed(libnav::waypoint* wpt, double c_time_sec);

	private:
		std::unordered_map<std::string, double> bl;
		std::mutex bl_mutex;


		static std::string get_black_list_key(libnav::waypoint* wpt);
	};


	class NavaidTuner
	{
	public:
		std::atomic<bool> update;

		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		BlackList* black_list;

		radnav_util::navaid_t vor_dme_cand;

		radnav_util::navaid_pair_t dme_dme_cand_pair;


		NavaidTuner(std::shared_ptr<XPDataBus::DataBus> databus, navaid_tuner_in_drs in, int ut);

		bool is_black_listed(libnav::waypoint* wpt);

		void set_vor_dme_cand(radnav_util::navaid_t cand);

		radnav_util::navaid_t get_vor_dme_cand();

		void set_dme_dme_cand(radnav_util::navaid_t cand1, radnav_util::navaid_t cand2, double qual);

		radnav_util::navaid_pair_t get_dme_dme_cand();

		void set_ac_pos(geo::point3d pos);

		geo::point3d get_ac_pos();

		/*
			The following member function blacklists a tuned navaid if connection is interrupted.
			Otherwise, it calculates a VOR DME position based on bearing and distance to a navaid.
			The calculated position, as well as its standard deviation, get output using certain datarefs.
		*/

		void update_vor_dme_conn(int radio_idx, double c_time, libnav::waypoint* navaid);

		void set_vor_dme_radios();

		void main_loop();

		void kill();

		~NavaidTuner();
	private:
		int n_update_freq_hz;

		navaid_tuner_in_drs in_drs;

		std::thread radio_thread;
		std::mutex vor_dme_cand_mutex;
		std::mutex dme_dme_cand_mutex;
		std::mutex ac_pos_mutex;

		geo::point3d ac_pos;

		radnav_util::navaid_t* dme_dme_cand;

		int* vor_dme_radio_modes;
		std::vector<vhf_radio_t> vor_dme_radios;
		std::vector<vhf_radio_t> dme_dme_radios;

		libtime::Timer* main_timer;
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
};
