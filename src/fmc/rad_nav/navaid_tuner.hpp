/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains declarations of member functions for NavaidTuner and BlackList classes.
*/

#pragma once


#include "radio.hpp"
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
	constexpr double VOR_DME_POS_UPDATE_DELAY_SEC = 2;


	struct navaid_tuner_in_drs
	{
		radio_drs_t sim_radio_drs[N_VHF_NAV_RADIOS];
	};

	struct navaid_tuner_out_drs
	{
		std::string vor_dme_pos_lat, vor_dme_pos_lon, vor_dme_pos_fom, curr_dme_pair_debug;
	};


	class BlackList
	{
	public:
		BlackList();

		void add_to_black_list(std::string* id, libnav::waypoint_entry* data, double bl_dur = NAVAID_PROHIBIT_PERMANENT);

		void remove_from_black_list(std::string* id, libnav::waypoint_entry* data);

		bool is_black_listed(std::string* id, libnav::waypoint_entry* data, double c_time_sec);

	private:
		std::unordered_map<std::string, double> bl;
		std::mutex bl_mutex;


		static std::string get_black_list_key(std::string* id, libnav::waypoint_entry* data);
	};


	class NavaidTuner
	{
	public:
		std::atomic<bool> update;

		std::shared_ptr<XPDataBus::DataBus> xp_databus;

		BlackList* black_list;

		radnav_util::navaid_t vor_dme_cand;

		radnav_util::navaid_pair_t dme_dme_cand_pair;


		NavaidTuner(std::shared_ptr<XPDataBus::DataBus> databus, navaid_tuner_in_drs in,
			navaid_tuner_out_drs out, int ut);

		bool is_black_listed(std::string* id, libnav::waypoint_entry* data);

		void set_vor_dme_cand(radnav_util::navaid_t cand);

		radnav_util::navaid_t get_vor_dme_cand();

		void set_dme_dme_cand(radnav_util::navaid_t cand1, radnav_util::navaid_t cand2, double qual);

		radnav_util::navaid_pair_t get_dme_dme_cand();

		void set_ac_pos(geo::point3d pos);

		geo::point3d get_ac_pos();

		void set_vor_dme_radios();

		void set_dme_dme_radios();

		void main_loop();

		void kill();

		~NavaidTuner();
	private:
		int n_update_freq_hz;

		navaid_tuner_in_drs in_drs;
		navaid_tuner_out_drs out_drs;

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

		double vor_dme_pos_update_last;


		/*
			Description:
			The following member function first ttries to delay the radio tuning
			in an attempt to restore connection. If connection couldn't be restored
			after a small period of time, the navaid gets black listed for a longer period.
			Param:
			ptr - pointer to radio
			c_time - current time. Usually obtained from main_timer.
		*/

		void black_list_tuned_navaid(vhf_radio_t* ptr, double c_time);

		/*
			The following member function blacklists a tuned navaid if connection is interrupted.
			Otherwise, it calculates a VOR DME position based on bearing and distance to a navaid.
			The calculated position, as well as its FOM(2*standard deviation), get output using certain datarefs.
		*/

		void update_vor_dme_conn(int radio_idx, double c_time);

		double get_curr_dme_dme_qual();

		void tune_dme_dme_cand(radnav_util::navaid_pair_t cand_pair, double c_time);

		void update_dme_dme_conn(double c_time);
	};
}
