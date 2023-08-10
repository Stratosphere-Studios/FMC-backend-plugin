/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains definitions of member functions used for radio navigation.
*/


#include "rad_nav.hpp"


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
		libnav::navaid_entry* navaid_data = new_navaid.data.navaid;
		if (navaid_data) // Make sure the pointer to navaid data isn't null.
		{
			tuned_navaid = new_navaid;
			xp_databus->set_datai(dr_list.freq, int(navaid_data->freq), dr_list.freq_idx);
			last_tune_time_sec = c_time;
			conn_retry = false;
		}
	}

	bool vhf_radio_t::is_sig_recv(int expected_type)
	{
		bool vor_recv = false;
		bool dme_recv = false;
		switch (expected_type)
		{
		case NAV_VOR:
			return xp_databus->get_data_s(dr_list.nav_id) == tuned_navaid.id;
		case NAV_DME:
			return xp_databus->get_data_s(dr_list.dme_id) == tuned_navaid.id;
		case NAV_VOR_DME:
			vor_recv = xp_databus->get_data_s(dr_list.nav_id) == tuned_navaid.id;
			dme_recv = xp_databus->get_data_s(dr_list.dme_id) == tuned_navaid.id;
			return vor_recv && dme_recv;
		default:
			return false;
		}
	}

	double vhf_radio_t::get_tuned_qual(geo::point3d ac_pos)
	{
		libnav::navaid_entry* navaid_data = tuned_navaid.data.navaid;
		if (navaid_data) // Check that pointer to a navaid structure isn't null.
		{
			double tru_dist_nm = xp_databus->get_datad(dr_list.dme_nm);
			if (tru_dist_nm)
			{
				double v_dist_nm = abs(ac_pos.alt_ft - tuned_navaid.data.navaid->elevation) * FT_TO_NM;
				double slant_ang_deg = asin(v_dist_nm / tru_dist_nm) * RAD_TO_DEG;

				if (slant_ang_deg > 0 && slant_ang_deg < VOR_MAX_SLANT_ANGLE_DEG)
				{
					double lat_dist_nm = sqrt(tru_dist_nm * tru_dist_nm - v_dist_nm * v_dist_nm);
					double qual = 1 - (lat_dist_nm / navaid_data->max_recv);
					if (lat_dist_nm && qual >= 0)
					{
						return qual;
					}
				}
			}
		}
		
		return -1;
	}

	// BlackList definitions:

	// Public functions:

	BlackList::BlackList()
	{

	}

	void BlackList::add_to_black_list(libnav::waypoint* wpt, double bl_dur)
	{
		std::string key = get_black_list_key(wpt);

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

	void BlackList::remove_from_black_list(libnav::waypoint* wpt)
	{
		std::string key = get_black_list_key(wpt);

		std::lock_guard<std::mutex> lock(bl_mutex);

		bl.erase(key);
	}

	bool BlackList::is_black_listed(libnav::waypoint* wpt, double c_time_sec)
	{
		if (wpt->data.navaid)
		{
			std::lock_guard<std::mutex> lock(bl_mutex);
			std::string key = get_black_list_key(wpt);
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

	std::string BlackList::get_black_list_key(libnav::waypoint* wpt)
	{
		return wpt->id + std::to_string(wpt->data.navaid->freq);
	}

	// NavaidTuner definitions:

	// Public functions:

	NavaidTuner::NavaidTuner(std::shared_ptr<XPDataBus::DataBus> databus, navaid_tuner_in_drs in, int freq)
	{
		ac_pos = {};

		xp_databus = databus;
		in_drs = in;
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

	bool NavaidTuner::is_black_listed(libnav::waypoint* wpt)
	{
		double c_time_sec = main_timer->get_curr_time();
		return black_list->is_black_listed(wpt, c_time_sec);
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

	/*
		The following member function blacklists a tuned navaid if connection is interrupted.
		Otherwise, it calculates a VOR DME position based on bearing and distance to a navaid.
		The calculated position, as well as its standard deviation, get output using certain datarefs.
	*/

	void NavaidTuner::update_vor_dme_conn(int radio_idx, double c_time, libnav::waypoint* navaid)
	{
		if (vor_dme_radios[radio_idx].is_sig_recv(NAV_VOR_DME))
		{
			// Calculate FOM and position
		}
		else
		{
			if (vor_dme_radios[radio_idx].conn_retry)
			{
				black_list->add_to_black_list(navaid, c_time + NAVAID_BLACK_LIST_DUR_SEC);
				vor_dme_radios[radio_idx].conn_retry = false;
			}
			else
			{
				vor_dme_radios[radio_idx].conn_retry = true;
				vor_dme_radios[radio_idx].last_tune_time_sec = c_time + NAVAID_RETRY_DELAY_SEC;
			}
		}
	}

	void NavaidTuner::set_vor_dme_radios()
	{
		for (int i = 0; i < N_VOR_DME_RADIOS; i++)
		{
			int mode = vor_dme_radio_modes[i];
			double c_time_sec = main_timer->get_curr_time();

			if (c_time_sec >= vor_dme_radios[i].last_tune_time_sec + RADIO_TUNE_DELAY_SEC)
			{
				libnav::waypoint tmp = { vor_dme_radios[i].tuned_navaid.id, vor_dme_radios[i].tuned_navaid.data };
				bool is_b_listed = black_list->is_black_listed(&tmp, c_time_sec);

				if (mode == NAV_VHF_AUTO)
				{
					int check_val = memcmp(&vor_dme_radios[i].tuned_navaid, &vor_dme_cand, sizeof(radnav_util::navaid_t));
					if (check_val) // Proceed to compare qualities if the tuned navaid doesn't match the current navaid.
					{
						if (is_b_listed)
						{
							vor_dme_radios[i].tune(vor_dme_cand, c_time_sec);
						}
						else
						{
							geo::point3d tmp_ac_pos = get_ac_pos();
							double curr_qual = vor_dme_radios[i].get_tuned_qual(tmp_ac_pos);
							if (vor_dme_cand.qual - curr_qual > NAVAID_MAX_QUAL_DIFF)
							{
								vor_dme_radios[i].tune(vor_dme_cand, c_time_sec);
							}
						}
					}
				}

				if (!is_b_listed)
				{
					update_vor_dme_conn(i, c_time_sec, &tmp);
				}
			}
		}
	}

	void NavaidTuner::main_loop()
	{
		while (update.load(UPDATE_FLG_ORDR))
		{
			set_vor_dme_radios();

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

	

	// NavaidSelector definitions:

	// Public functions:

	NavaidSelector::NavaidSelector(std::shared_ptr<XPDataBus::DataBus> databus, NavaidTuner* tuner, 
								   navaid_selector_out_drs out, double tile_size, double navaid_thresh_nm, int dur_sec)
	{
		ac_pos_last = { 0, 0, 0 };

		xp_databus = databus;
		navaid_tuner = tuner;

		wpt_db = nullptr;

		cache_tile_size = tile_size;
		min_navaid_dist_nm = navaid_thresh_nm;
		cand_update_dur_sec = dur_sec;
		cand_update_last_sec = -dur_sec;

		out_drs = out;

		navaid_cache = {};
	}

	void NavaidSelector::update_dme_dme_cand(geo::point ac_pos, std::vector<radnav_util::navaid_t>* navaids)
	{
		std::vector<radnav_util::navaid_pair_t> navaid_pairs;

		int n_navaids = N_DME_DME_STA;
		if (navaids->size() < n_navaids)
		{
			n_navaids = int(navaids->size());
		}

		for (int i = 0; i < n_navaids; i++)
		{
			for (int j = i + 1; j < n_navaids; j++)
			{
				radnav_util::navaid_t* p1 = &navaids->at(i);
				radnav_util::navaid_t* p2 = &navaids->at(j);
				if (p1->qual != -1 && p2->qual != -1)
				{
					radnav_util::navaid_pair_t tmp = { p1, p2, 0 };
					tmp.calc_qual(ac_pos);

					navaid_pairs.push_back(tmp);
				}
			}
		}

		// Sort navaid pairs by quality in descending order

		std::sort(navaid_pairs.begin(), navaid_pairs.end(),
				  [](radnav_util::navaid_pair_t n1, radnav_util::navaid_pair_t n2) -> bool { return n1.qual > n2.qual; });

		radnav_util::navaid_pair_t top_pair = navaid_pairs.at(0);
		navaid_tuner->set_dme_dme_cand(*top_pair.n1, *top_pair.n2, top_pair.qual);

		// Set some DEBUG-ONLY datarefs

		for (int i = 0; i < out_drs.dme_dme_cand_data.size(); i++)
		{
			std::string id1 = navaid_pairs.at(i).n1->id;
			std::string id2 = navaid_pairs.at(i).n2->id;
			std::string qual = std::to_string(navaid_pairs.at(i).qual);
			xp_databus->set_data_s(out_drs.dme_dme_cand_data.at(i), id1 + " " + id2 + " " + qual);
		}
	}

	/*
		The following member function updates VOR DME and DME DME candidates.
	*/

	void NavaidSelector::update_rad_nav_cand(geo::point3d ac_pos)
	{
		radnav_util::navaid_t vor_dme_cand;
		std::vector<radnav_util::navaid_t> navaids;
		
		// Add all navaids within min_navaid_dist_nm miles from the 
		// aircraft to navaids.

		for (int i = 0; i < navaid_cache.size(); i++)
		{
			libnav::waypoint tmp = navaid_cache.at(i);

			if (!navaid_tuner->is_black_listed(&tmp))
			{
				geo::point3d tmp_pos = { tmp.data.pos, tmp.data.navaid->elevation };
				double dist = tmp_pos.get_true_dist_nm(ac_pos);

				if (dist <= min_navaid_dist_nm)
				{
					radnav_util::navaid_t new_navaid = { tmp.id, tmp.data, 0 };
					new_navaid.calc_qual(ac_pos);
					navaids.push_back(new_navaid);
				}
			}
		}

		// Sort navaids by quality in descending order

		std::sort(navaids.begin(), navaids.end(), 
				  [](radnav_util::navaid_t n1, radnav_util::navaid_t n2) -> bool { return n1.qual > n2.qual; });

		int j = 0;
		for (int i = 0; i < navaids.size(); i++)
		{
			if (navaids.at(i).data.type == NAV_VOR_DME)
			{
				if (j == 0)
				{
					navaid_tuner->set_vor_dme_cand(navaids.at(i));
				}
				xp_databus->set_data_s(out_drs.vor_dme_cand_data.at(j), navaids.at(i).id + " " + std::to_string(navaids.at(i).qual));
				j++;
			}
			if (j == out_drs.vor_dme_cand_data.size())
			{
				break;
			}
		}
		navaid_tuner->set_vor_dme_cand(navaids.at(0));

		update_dme_dme_cand(ac_pos.p, &navaids);
	}

	void NavaidSelector::update(libnav::wpt_db_t* ptr, geo::point3d ac_pos, double c_time_sec)
	{
		if (wpt_db != ptr)
		{
			wpt_db = ptr;
		}

		if (abs(ac_pos_last.p.lat_deg - ac_pos.p.lat_deg) > cache_tile_size * 0.3 ||
			abs(ac_pos_last.p.lon_deg - ac_pos.p.lon_deg) > cache_tile_size * 0.3)
		{
			update_navaid_cache(ac_pos.p);
		}

		if (c_time_sec >= cand_update_dur_sec + cand_update_last_sec)
		{
			cand_update_last_sec = c_time_sec;

			std::future<void> f = std::async(std::launch::async, 
											 [](NavaidSelector* ptr, geo::point3d pos, double c_time_sec)
											 {ptr->update_rad_nav_cand(pos); }, this, ac_pos, c_time_sec);
		}
	}

	NavaidSelector::~NavaidSelector()
	{
	}

	// Private functions:

	void NavaidSelector::update_navaid_cache(geo::point ac_pos)
	{
		navaid_cache = {};

		for (auto& it : *wpt_db)
		{
			if (it.first.length() < ILS_NAVAID_ID_LENGTH) // ILS components aren't welcome
			{
				for (int i = 0; i < it.second.size(); i++)
				{
					libnav::waypoint_entry tmp = it.second.at(i);
					if (tmp.navaid && (tmp.type == NAV_DME || tmp.type == NAV_DME_ONLY || tmp.type == NAV_VOR_DME))
					{
						if (abs(tmp.pos.lat_deg - ac_pos.lat_deg) <= cache_tile_size &&
							abs(tmp.pos.lon_deg - ac_pos.lon_deg) <= cache_tile_size)
						{
							libnav::waypoint tmp_wpt = { it.first, tmp };
							navaid_cache.push_back(tmp_wpt);
						}
					}
				}
			}
		}
	}
};
