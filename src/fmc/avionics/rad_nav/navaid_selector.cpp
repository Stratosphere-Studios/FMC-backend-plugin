/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains definitions of member functions for NavaidSelector class. NavaidSelector class updates
	VOR/DME as well as DME/DME candidates, which are then tuned by NavaidTuner. These navaids are used to tune
	the position provided by IRS.
*/

#include "navaid_selector.hpp"


namespace StratosphereAvionics
{
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

		size_t n_navaids = N_DME_DME_STA;
		if (navaids->size() < n_navaids)
		{
			n_navaids = navaids->size();
		}

		for (size_t i = 0; i < n_navaids; i++)
		{
			for (size_t j = i + 1; j < n_navaids; j++)
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

		for (size_t i = 0; i < out_drs.dme_dme_cand_data.size(); i++)
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

		for (size_t i = 0; i < navaid_cache.size(); i++)
		{
			libnav::waypoint_t tmp = navaid_cache.at(i);

			if (!navaid_tuner->is_black_listed(&tmp.id, &tmp.data))
			{
				geo::point3d tmp_pos = { tmp.data.pos, tmp.data.navaid->elev_ft };
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

		size_t j = 0;
		for (size_t i = 0; i < navaids.size(); i++)
		{
			if (navaids.at(i).data.type == libnav::NavaidType::VOR_DME)
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

	void NavaidSelector::update(libnav::wpt_db_t* ptr, geo::point3d ac_pos, 
		double c_time_sec)
	{
		if (wpt_db != ptr)
		{
			wpt_db = ptr;
		}

		if (abs(ac_pos_last.p.lat_rad - ac_pos.p.lat_rad) > cache_tile_size * 0.3 ||
			abs(ac_pos_last.p.lon_rad - ac_pos.p.lon_rad) > cache_tile_size * 0.3)
		{
			update_navaid_cache(ac_pos.p);
		}

		if (c_time_sec >= cand_update_dur_sec + cand_update_last_sec)
		{
			cand_update_last_sec = c_time_sec;

			std::future<void> f = std::async(std::launch::async,
				[](NavaidSelector* ptr, geo::point3d pos)
				{ptr->update_rad_nav_cand(pos); }, this, ac_pos);
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
				for (size_t i = 0; i < it.second.size(); i++)
				{
					libnav::waypoint_entry_t tmp = it.second.at(i);
					if (tmp.navaid && 
						(tmp.type == libnav::NavaidType::DME || 
						tmp.type == libnav::NavaidType::DME_ONLY || 
						tmp.type == libnav::NavaidType::VOR_DME))
					{
						if (abs(tmp.pos.lat_rad - ac_pos.lat_rad) <= cache_tile_size &&
							abs(tmp.pos.lon_rad - ac_pos.lon_rad) <= cache_tile_size)
						{
							libnav::waypoint_t tmp_wpt = { it.first, tmp };
							navaid_cache.push_back(tmp_wpt);
						}
					}
				}
			}
		}
	}
}
