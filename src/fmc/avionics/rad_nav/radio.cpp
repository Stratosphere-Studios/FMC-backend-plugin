/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains definitions of member functions of vhf_radio_t - an interface for x-plane's default radios
*/

#include "radio.hpp"


namespace StratosphereAvionics
{
	vhf_radio_t::vhf_radio_t(std::shared_ptr<XPDataBus::DataBus> databus, radio_drs_t drs)
	{
		xp_databus = databus;
		dr_list = drs;

		tuned_navaid = {};
		last_tune_time_sec = 0;
		conn_retry = false;
	}

	void vhf_radio_t::tune(radnav_util::navaid_t new_navaid, double c_time)
	{
		libnav::navaid_entry_t* navaid_data = new_navaid.data.navaid;
		if (navaid_data) // Make sure the pointer to navaid data isn't null.
		{
			tuned_navaid = new_navaid;
			xp_databus->set_datai(dr_list.freq, int(navaid_data->freq), dr_list.dr_idx);
			last_tune_time_sec = c_time;
			conn_retry = false;
		}
	}

	bool vhf_radio_t::is_sig_recv(libnav::NavaidType expected_type)
	{
		bool vor_recv = false;
		bool dme_recv = false;
		switch (expected_type)
		{
		case libnav::NavaidType::VOR:
			return xp_databus->get_data_s(dr_list.nav_id) == tuned_navaid.id;
		case libnav::NavaidType::DME:
			return xp_databus->get_data_s(dr_list.dme_id) == tuned_navaid.id;
		case libnav::NavaidType::VOR_DME:
			vor_recv = xp_databus->get_data_s(dr_list.nav_id) == tuned_navaid.id;
			dme_recv = xp_databus->get_data_s(dr_list.dme_id) == tuned_navaid.id;
			return vor_recv && dme_recv;
		default:
			return false;
		}
	}

	/*
		The following function returns the ground distance to the tuned nav aid.
		It uses the distance output from a DME and accounts for slant angle using
		the current altitude of the aircraft.
	*/

	double vhf_radio_t::get_gnd_dist(double dist, double alt_ft)
	{
		if (dist && tuned_navaid.data.navaid)
		{
			double v_dist_nm = abs(alt_ft - tuned_navaid.data.navaid->elev_ft) 
				* geo::FT_TO_NM;
			return sqrt(dist * dist - v_dist_nm * v_dist_nm);
		}
		return 0;
	}

	/*
		The following function returns quality of a tuned navaid.
		It gets aircraft position as input.
	*/

	double vhf_radio_t::get_tuned_qual(geo::point3d ac_pos, double dist)
	{
		libnav::navaid_entry_t* navaid_data = tuned_navaid.data.navaid;
		if (navaid_data && dist) // Check that pointer to a navaid structure isn't null.
		{
			double v_dist_nm = abs(ac_pos.alt_ft - tuned_navaid.data.navaid->elev_ft) 
				* geo::FT_TO_NM;
			double slant_ang_deg = asin(v_dist_nm / dist) * geo::RAD_TO_DEG;

			if (slant_ang_deg > 0 && slant_ang_deg < libnav::VOR_MAX_SLANT_ANGLE_DEG)
			{
				double lat_dist_nm = sqrt(dist * dist - v_dist_nm * v_dist_nm);
				double qual = 1 - (lat_dist_nm / navaid_data->max_recv);
				if (lat_dist_nm && qual >= 0)
				{
					return qual;
				}
			}
		}

		return -1;
	}
}
