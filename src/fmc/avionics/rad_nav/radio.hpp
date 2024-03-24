/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains declarations of member functions of vhf_radio_t - an interface for x-plane's default radios
*/

#pragma once


#include "databus.hpp"
#include "nav_db.hpp"


namespace StratosphereAvionics
{
	enum nav_vhf_radio_modes
	{
		NAV_VHF_AUTO = 0,
		NAV_VHF_MAN = 1
	};


	struct radio_drs_t
	{
		std::string freq, nav_id, dme_id, vor_deg, dme_nm;
		int dr_idx; // This index is used to obtain navaid identifiers, bearings and distances
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
			The following function returns the ground distance to the tuned nav aid.
			It uses the distance output from a DME and accounts for slant angle using
			the current altitude of the aircraft.
		*/

		double get_gnd_dist(double dist, double alt_ft);

		/*
			The following function returns quality of a tuned navaid.
			It gets aircraft position as input.
		*/

		double get_tuned_qual(geo::point3d ac_pos, double dist);
	};
};
