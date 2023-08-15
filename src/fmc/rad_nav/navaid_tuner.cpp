/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains definitions of member functions for NavaidTuner and BlackList classes.
*/

#include "navaid_tuner.hpp"


namespace StratosphereAvionics
{
	// BlackList definitions:

	// Public functions:

	BlackList::BlackList()
	{

	}

	void BlackList::add_to_black_list(std::string* id, libnav::waypoint_entry* data, double bl_dur)
	{
		if (data->navaid) // Make sure the navaid pointer isn't null.
		{
			std::string key = get_black_list_key(id, data);

			std::lock_guard<std::mutex> lock(bl_mutex);

			if (bl.find(key) == bl.end())
			{
				std::pair<std::string, double> tmp = std::make_pair(key, bl_dur);
				bl.insert(tmp);
			}
			else
			{
				bl.at(key) = bl_dur;
			}
		}
	}

	void BlackList::remove_from_black_list(std::string* id, libnav::waypoint_entry* data)
	{
		if (data->navaid) // Make sure the navaid pointer isn't null.
		{
			std::string key = get_black_list_key(id, data);

			std::lock_guard<std::mutex> lock(bl_mutex);

			bl.erase(key);
		}
	}

	bool BlackList::is_black_listed(std::string* id, libnav::waypoint_entry* data, double c_time_sec)
	{
		if (data->navaid)
		{
			std::lock_guard<std::mutex> lock(bl_mutex);
			std::string key = get_black_list_key(id, data);
			if (bl.find(key) != bl.end())
			{
				if (bl.at(key) > c_time_sec ||
					bl.at(key) == NAVAID_PROHIBIT_PERMANENT)
				{
					return true;
				}
				else
				{
					bl.erase(key);
				}
			}
		}

		return false;
	}

	// Private functions

	std::string BlackList::get_black_list_key(std::string* id, libnav::waypoint_entry* data)
	{
		if (data->navaid)
		{
			return *id + std::to_string(data->navaid->freq);
		}
		return "";
	}

	// NavaidTuner definitions:

	// Public functions:

	NavaidTuner::NavaidTuner(std::shared_ptr<XPDataBus::DataBus> databus, navaid_tuner_in_drs in,
		navaid_tuner_out_drs out, int freq)
	{
		ac_pos = {};
		vor_dme_pos_update_last = 0;
		dme_dme_pos_update_last = 0;

		xp_databus = databus;
		in_drs = in;
		out_drs = out;
		n_update_freq_hz = freq;

		dme_dme_cand = new radnav_util::navaid_t[N_DME_DME_CAND];
		vor_dme_radio_modes = new int[N_VOR_DME_RADIOS];

		for (int i = 0; i < N_VOR_DME_RADIOS; i++)
		{
			vor_dme_radio_modes[i] = NAV_VHF_AUTO;
			vhf_radio_t tmp_vor_dme = vhf_radio_t(databus, in.sim_radio_drs[i]);
			vhf_radio_t tmp_dme_dme = vhf_radio_t(databus, in.sim_radio_drs[N_VOR_DME_RADIOS + i]);
			vor_dme_radios.push_back(tmp_vor_dme);
			dme_dme_radios.push_back(tmp_dme_dme);
		}

		main_timer = new libtime::Timer();
		black_list = new BlackList();

		update.store(true, UPDATE_FLG_ORDR);

		radio_thread = std::thread([](NavaidTuner* ptr) {ptr->main_loop(); }, this);
	}

	bool NavaidTuner::is_black_listed(std::string* id, libnav::waypoint_entry* data)
	{
		double c_time_sec = main_timer->get_curr_time();
		return black_list->is_black_listed(id, data, c_time_sec);
	}

	void NavaidTuner::set_vor_dme_cand(radnav_util::navaid_t cand)
	{
		std::lock_guard<std::mutex> lock(vor_dme_cand_mutex);

		vor_dme_cand = cand;
	}

	radnav_util::navaid_t NavaidTuner::get_vor_dme_cand()
	{
		std::lock_guard<std::mutex> lock(vor_dme_cand_mutex);

		return vor_dme_cand;
	}

	void NavaidTuner::set_dme_dme_cand(radnav_util::navaid_t cand1, radnav_util::navaid_t cand2, double qual)
	{
		std::lock_guard<std::mutex> lock(dme_dme_cand_mutex);

		dme_dme_cand[0] = cand1;
		dme_dme_cand[1] = cand2;

		dme_dme_cand_pair = { dme_dme_cand, dme_dme_cand + 1, qual };
	}

	radnav_util::navaid_pair_t NavaidTuner::get_dme_dme_cand()
	{
		std::lock_guard<std::mutex> lock(dme_dme_cand_mutex);

		return dme_dme_cand_pair;
	}

	void NavaidTuner::set_ac_pos(geo::point3d pos)
	{
		std::lock_guard<std::mutex> lock(ac_pos_mutex);

		ac_pos = pos;
	}

	geo::point3d NavaidTuner::get_ac_pos()
	{
		std::lock_guard<std::mutex> lock(ac_pos_mutex);

		return ac_pos;
	}

	void NavaidTuner::set_vor_dme_radios()
	{
		for (int i = 0; i < N_VOR_DME_RADIOS; i++)
		{
			int mode = vor_dme_radio_modes[i];
			double c_time_sec = main_timer->get_curr_time();
			vhf_radio_t* curr_radio = &vor_dme_radios[i];

			if (c_time_sec >= curr_radio->last_tune_time_sec + RADIO_TUNE_DELAY_SEC)
			{
				radnav_util::navaid_t cand = get_vor_dme_cand();

				radnav_util::navaid_t* tmp = &curr_radio->tuned_navaid;
				bool is_b_listed = black_list->is_black_listed(&tmp->id, &tmp->data, c_time_sec);

				if (mode == NAV_VHF_AUTO)
				{
					int check_val = memcmp(&curr_radio->tuned_navaid.data, &cand.data, sizeof(libnav::waypoint_entry));
					if (check_val) // Proceed to compare qualities if the tuned navaid doesn't match the current navaid.
					{
						if (is_b_listed)
						{
							curr_radio->tune(vor_dme_cand, c_time_sec);
						}
						else
						{
							geo::point3d tmp_ac_pos = get_ac_pos();
							double dme_dist = double(xp_databus->get_dataf(curr_radio->dr_list.dme_nm, curr_radio->dr_list.dr_idx));
							double curr_qual = curr_radio->get_tuned_qual(tmp_ac_pos, dme_dist);
							if (cand.qual - curr_qual > NAVAID_MAX_QUAL_DIFF)
							{
								curr_radio->tune(vor_dme_cand, c_time_sec);
								continue;
							}
						}
					}
				}

				if (!is_b_listed)
				{
					update_vor_dme_conn(i, c_time_sec);
				}
			}
		}
	}

	void NavaidTuner::set_dme_dme_radios()
	{
		bool tune_cand = false;
		bool is_b_listed_any = false; // True if any tuned DME is black listed.

		double c_time_sec = main_timer->get_curr_time();
		double max_tune_time_sec = dme_dme_radios[0].last_tune_time_sec;
		if (dme_dme_radios[1].last_tune_time_sec > max_tune_time_sec)
		{
			max_tune_time_sec = dme_dme_radios[1].last_tune_time_sec;
		}

		if (c_time_sec >= max_tune_time_sec + RADIO_TUNE_DELAY_SEC)
		{
			radnav_util::navaid_pair_t cand_pair = get_dme_dme_cand();
			for (int i = 0; i < N_DME_DME_RADIOS && !is_b_listed_any; i++)
			{
				radnav_util::navaid_t* tmp = &vor_dme_radios[i].tuned_navaid;
				bool is_b_listed = black_list->is_black_listed(&tmp->id, &tmp->data, c_time_sec);
				is_b_listed_any |= is_b_listed;
			}

			if (is_b_listed_any)
			{
				tune_dme_dme_cand(cand_pair, c_time_sec);
			}
			else
			{
				geo::point3d ppos = get_ac_pos();
				double dist_1 = double(xp_databus->get_dataf(dme_dme_radios[0].dr_list.dme_nm, dme_dme_radios[0].dr_list.dr_idx));
				double dist_2 = double(xp_databus->get_dataf(dme_dme_radios[1].dr_list.dme_nm, dme_dme_radios[1].dr_list.dr_idx));
				double phi = get_curr_dme_dme_phi_rad(ppos, dist_1, dist_2);
				double curr_qual = get_curr_dme_dme_qual(dist_1, dist_2, phi * RAD_TO_DEG);
				std::string dme_dme_debug = dme_dme_radios[0].tuned_navaid.id + " " +
					dme_dme_radios[1].tuned_navaid.id + " " + std::to_string(curr_qual);
				xp_databus->set_data_s(out_drs.curr_dme_pair_debug, dme_dme_debug);
				if (cand_pair.qual - curr_qual > NAVAID_MAX_QUAL_DIFF)
				{
					tune_dme_dme_cand(cand_pair, c_time_sec);
					return;
				}

				update_dme_dme_conn(dist_1, dist_2, phi, c_time_sec);
			}
		}
	}

	void NavaidTuner::main_loop()
	{
		while (update.load(UPDATE_FLG_ORDR))
		{
			set_vor_dme_radios();
			set_dme_dme_radios();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / n_update_freq_hz));
		}
	}

	void NavaidTuner::kill()
	{
		update.store(false, UPDATE_FLG_ORDR);
		radio_thread.join();
	}

	NavaidTuner::~NavaidTuner()
	{
		kill();

		delete[] dme_dme_cand;
		delete[] black_list;
		delete[] main_timer;
	}

	// Private functions:

	/*
		Function: black_list_tuned_navaid
		Description:
		The following member function first tries to delay the radio tuning
		in an attempt to restore connection. If connection couldn't be restored
		after a small period of time, the navaid gets black listed for a longer period.
		Param:
		ptr - pointer to radio
		c_time - current time. Usually obtained from main_timer.
		Return:
		nothing to see here
	*/

	void NavaidTuner::black_list_tuned_navaid(vhf_radio_t* ptr, double c_time)
	{
		if (ptr->conn_retry)
		{
			radnav_util::navaid_t* tmp = &ptr->tuned_navaid;
			black_list->add_to_black_list(&tmp->id, &tmp->data, c_time + NAVAID_BLACK_LIST_DUR_SEC);
			ptr->conn_retry = false;
		}
		else
		{
			ptr->conn_retry = true;
			ptr->last_tune_time_sec = c_time + NAVAID_RETRY_DELAY_SEC;
		}
	}

	/*
		The following member function blacklists a tuned navaid if connection is interrupted.
		Otherwise, it calculates a VOR DME position based on bearing and distance to a navaid.
		The calculated position, as well as its FOM(2*standard deviation), get output using certain datarefs.
	*/

	void NavaidTuner::update_vor_dme_conn(int radio_idx, double c_time)
	{
		vhf_radio_t* curr_radio = &vor_dme_radios[radio_idx];
		if (curr_radio->is_sig_recv(NAV_VOR_DME))
		{
			vor_dme_radios[radio_idx].conn_retry = false;
			// Calculate FOM and position

			if (c_time >= vor_dme_pos_update_last + RADIO_POS_UPDATE_DELAY_SEC)
			{
				int idx = curr_radio->dr_list.dr_idx;
				geo::point3d ppos = get_ac_pos();
				double brng = double(xp_databus->get_dataf(curr_radio->dr_list.vor_deg, idx));
				double total_dist = double(xp_databus->get_dataf(curr_radio->dr_list.dme_nm, idx));
				double dist = vor_dme_radios[radio_idx].get_gnd_dist(total_dist, ppos.alt_ft);

				geo::point pos = geo::get_pos_from_brng_dist(vor_dme_radios[radio_idx].tuned_navaid.data.pos, brng, dist);
				double pos_fom_m = radnav_util::get_vor_dme_fom(total_dist) * NM_TO_M;

				xp_databus->set_datad(out_drs.vor_dme_pos_lat, pos.lat_deg);
				xp_databus->set_datad(out_drs.vor_dme_pos_lon, pos.lon_deg);
				xp_databus->set_datad(out_drs.vor_dme_pos_fom, pos_fom_m);

				vor_dme_pos_update_last = c_time;
			}
		}
		else
		{
			xp_databus->set_datad(out_drs.vor_dme_pos_lat, 0);
			xp_databus->set_datad(out_drs.vor_dme_pos_lon, 0);
			xp_databus->set_datad(out_drs.vor_dme_pos_fom, 0);
			black_list_tuned_navaid(&vor_dme_radios[radio_idx], c_time);
		}
	}

	/*
		Function: get_curr_dme_dme_phi_rad
		Description:
		function that returns angle between 2 currently tuned DMEs.
		Param:
		ppos: present aircraft position
		dist_1: distance to first dme(tuned in dme_dme radio #1)
		dist_2: distance to second dme(tuned in dme_dme radio #2)
		Return:
		returns angle between -pi/2 and pi/2 in radians
	*/

	double NavaidTuner::get_curr_dme_dme_phi_rad(geo::point3d ppos, double dist_1, double dist_2)
	{
		if (dist_1 * dist_2)
		{
			libnav::waypoint_entry* data_1 = &dme_dme_radios[0].tuned_navaid.data;
			libnav::waypoint_entry* data_2 = &dme_dme_radios[1].tuned_navaid.data;
			if (data_1->navaid && data_2->navaid)
			{
				// Get encounter geometry angle for currently tuned DMEs.
				double gnd_dist_1 = dme_dme_radios[0].get_gnd_dist(dist_1, ppos.alt_ft);
				double gnd_dist_2 = dme_dme_radios[1].get_gnd_dist(dist_2, ppos.alt_ft);
				double gnd_dist_3 = data_1->pos.get_line_dist_nm(data_2->pos, data_1->navaid->elevation, data_2->navaid->elevation);
				// Verify that triangle exists
				if (gnd_dist_1 + gnd_dist_2 > gnd_dist_3 && gnd_dist_1 + gnd_dist_3 > gnd_dist_2 && gnd_dist_2 + gnd_dist_3 > gnd_dist_1)
				{
					// Just applying the good old law of cosines. Nothing to see here :)
					double cos_phi = (std::pow(gnd_dist_1, 2) + std::pow(gnd_dist_2, 2) - std::pow(gnd_dist_3, 2)) /
						(gnd_dist_1 * gnd_dist_2);
					return acos(cos_phi);
				}
			}			
		}
		return 0;
	}

	/*
		Function: get_curr_dme_dme_qual
		Description:
		function that returns the quality value of the currently tuned pair of DMEs. The
		quality value is based on dist_1 and dist_2, which are supposed to be the distances
		returned from DMEs themselves.
		Param:
		ppos: present aircraft position
		dist_1: distance to first dme(tuned in dme_dme radio #1)
		dist_2: distance to second dme(tuned in dme_dme radio #2)
		phi_deg: encounter geometry angle between each DME and the aircraft
		e.g. for DMEs C and B and aircraft position A the angle phi would be CAB
		Return:
		returns a quality value in range [-1, 1]
	*/

	double NavaidTuner::get_curr_dme_dme_qual(double dist_1, double dist_2, double phi_deg)
	{
		geo::point3d ppos = get_ac_pos();
		double qual_1 = dme_dme_radios[0].get_tuned_qual(ppos, dist_1);
		double qual_2 = dme_dme_radios[1].get_tuned_qual(ppos, dist_2);
		return radnav_util::get_dme_dme_qual(phi_deg, qual_1, qual_2);
	}

	/*
		Function: tune_dme_dme_cand
		Description:
		function that tunes dme_dme radio #1 and #2 to navaids provided by cand_pair
		Param:
		cand_pair: pair of candidates for DME DME position
		c_time: current time(obtained using main_timer)
		Return:
		nothing to see here
	*/

	void NavaidTuner::tune_dme_dme_cand(radnav_util::navaid_pair_t cand_pair, double c_time)
	{
		radnav_util::navaid_t* cand_navaids[N_DME_DME_CAND] = { cand_pair.n1, cand_pair.n2 };
		for (int i = 0; i < N_DME_DME_RADIOS; i++)
		{
			int check_val = memcmp(&dme_dme_radios[i].tuned_navaid.data, &cand_navaids[i]->data, sizeof(libnav::waypoint_entry));

			if (check_val)
			{
				dme_dme_radios[i].tune(*cand_navaids[i], c_time);
			}
		}
	}

	/*
		Function: update_dme_dme_pos
		Description:
		function that calculates the DME/DME position and outputs it via debug datarefs(subject to change)
		Param:
		dist_1: distance to dme tuned by dme_dme radio #1
		c_time: distance to dme tuned by dme_dme radio #2
		Return:
		nothing to see here
	*/

	void NavaidTuner::update_dme_dme_pos(double dist_1, double dist_2, double phi_rad)
	{
		if (dme_dme_radios[0].tuned_navaid.data.navaid && dme_dme_radios[1].tuned_navaid.data.navaid)
		{
			libnav::waypoint_entry* data_1 = &dme_dme_radios[0].tuned_navaid.data;
			libnav::waypoint_entry* data_2 = &dme_dme_radios[1].tuned_navaid.data;
			double elev_1 = data_1->navaid->elevation;
			double elev_2 = data_2->navaid->elevation;
			// Calculate position and FOM
			geo::point3d ppos = get_ac_pos();
			geo::point pos[2];
			int n_pos_calc = 0;
			if (data_1->pos.lon_deg < data_2->pos.lon_deg)
			{
				n_pos_calc = geo::get_dme_dme_pos(data_1->pos, data_2->pos, dist_1, dist_2, elev_1, elev_2, ppos.alt_ft, pos);
			}
			else
			{
				n_pos_calc = geo::get_dme_dme_pos(data_2->pos, data_1->pos, dist_2, dist_1, elev_1, elev_2, ppos.alt_ft, pos);
			}
			if (n_pos_calc)
			{
				double d_1 = pos[0].get_great_circle_distance_nm(ppos.p);
				double d_2 = pos[1].get_great_circle_distance_nm(ppos.p);
				geo::point* dme_dme_pos;
				double pos_fom_m = radnav_util::get_dme_dme_fom(dist_1, dist_2, phi_rad) * NM_TO_M;
				if (d_1 < d_2)
				{
					dme_dme_pos = &pos[0];
				}
				else
				{
					dme_dme_pos = &pos[1];
				}

				xp_databus->set_datad(out_drs.dme_dme_pos_lat, dme_dme_pos->lat_deg);
				xp_databus->set_datad(out_drs.dme_dme_pos_lon, dme_dme_pos->lon_deg);
				xp_databus->set_datad(out_drs.dme_dme_pos_fom, pos_fom_m);
			}
			else
			{
				xp_databus->set_datad(out_drs.dme_dme_pos_lat, 0);
				xp_databus->set_datad(out_drs.dme_dme_pos_lon, 0);
				xp_databus->set_datad(out_drs.dme_dme_pos_fom, 0);
			}
		}
	}

	void NavaidTuner::update_dme_dme_conn(double dist_1, double dist_2, double phi_rad, double c_time)
	{
		bool is_sig_recv_1 = dme_dme_radios[0].is_sig_recv(NAV_DME);
		bool is_sig_recv_2 = dme_dme_radios[1].is_sig_recv(NAV_DME);
		if (is_sig_recv_1 && is_sig_recv_2)
		{
			dme_dme_radios[0].conn_retry = false;
			dme_dme_radios[1].conn_retry = false;

			if (c_time >= dme_dme_pos_update_last + RADIO_POS_UPDATE_DELAY_SEC)
			{
				update_dme_dme_pos(dist_1, dist_2, phi_rad);
				dme_dme_pos_update_last = c_time;
			}
		}
		if (!is_sig_recv_1)
		{
			black_list_tuned_navaid(&dme_dme_radios[0], c_time);
		}
		if (!is_sig_recv_2)
		{
			black_list_tuned_navaid(&dme_dme_radios[1], c_time);
		}
	}
}
