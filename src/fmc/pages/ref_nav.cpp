enum ref_nav_const
{
	RAD_NAV_INHIBIT = 0,
	RAD_NAV_VOR_ONLY_INHIBIT = 1,
	RAD_NAV_NO_INHIBIT = 2
};


namespace StratosphereAvionics
{
	int FMC::update_ref_nav_poi_data(size_t n_arpts_found, size_t n_wpts_found, std::string icao, 
									 libnav::airport_data arpt_found, std::vector<libnav::waypoint_entry> wpts_found)
	{
		xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
		xp_databus->set_data_s(out_drs.ref_nav.poi_id, icao);

		double poi_lat, poi_lon;

		if (n_arpts_found)
		{
			xp_databus->set_datai(out_drs.ref_nav.poi_type, POI_AIRPORT);
			poi_lat = arpt_found.pos.lat_deg;
			poi_lon = arpt_found.pos.lon_deg;

			xp_databus->set_datad(out_drs.ref_nav.poi_elevation, double(arpt_found.elevation_ft));
		}
		else
		{
			libnav::waypoint_entry curr_wpt = wpts_found.at(0);

			if (n_wpts_found > 1)
			{
				curr_wpt = update_sel_des_wpt(icao, wpts_found);

				reset_sel_navaid();
			}

			if (curr_wpt.type == NAV_NONE)
			{
				return 0;
			}
			else if (curr_wpt.type == NAV_WAYPOINT)
			{
				xp_databus->set_datai(out_drs.ref_nav.poi_type, POI_WAYPOINT);
			}
			else
			{
				xp_databus->set_datai(out_drs.ref_nav.poi_type, POI_NAVAID);
				xp_databus->set_datad(out_drs.ref_nav.poi_freq, curr_wpt.navaid->freq);
			}

			poi_lat = curr_wpt.pos.lat_deg;
			poi_lon = curr_wpt.pos.lon_deg;
		}

		double mag_var = xp_databus->get_mag_var(poi_lat, poi_lon);
		std::string mag_var_str = strutils::mag_var_to_str(mag_var);

		xp_databus->set_datad(out_drs.ref_nav.poi_lat, poi_lat);
		xp_databus->set_datad(out_drs.ref_nav.poi_lon, poi_lon);
		xp_databus->set_data_s(out_drs.ref_nav.poi_mag_var, mag_var_str);

		return 1;
	}

	void FMC::update_ref_nav_inhibit(std::vector<std::string>* nav_drs, std::vector<int> types, 
									 int threshold, bool add_vor)
	{
		for (int i = 0; i < nav_drs->size(); i++)
		{
			std::string curr_dr_name = nav_drs->at(i);
			std::string tmp = xp_databus->get_data_s(curr_dr_name);
			std::string entry_curr;
			std::string entry_last = dr_cache->get_val_s(curr_dr_name);

			strip_str(&tmp, &entry_curr);

			if (entry_curr != entry_last)
			{
				dr_cache->set_val_s(curr_dr_name, entry_curr);

				if (entry_curr != "")
				{
					if (xp_databus->get_datai(in_drs.ref_nav.rad_nav_inh) > threshold)
					{
						if (nav_db->is_navaid_of_type(entry_curr, types))
						{
							if (add_vor)
							{
								avionics->excl_vor(entry_curr, i);
							}
							else
							{
								avionics->excl_navaid(entry_curr, i);
							}
						}
						else
						{
							xp_databus->set_data_s(out_drs.scratchpad_msg, "NOT IN DATA BASE");
						}
					}
					else
					{
						xp_databus->set_data_s(out_drs.scratchpad_msg, "INVALID ENTRY");
					}
				}
				else
				{
					if (add_vor)
					{
						avionics->excl_vor(entry_curr, i);
					}
					else
					{
						avionics->excl_navaid(entry_curr, i);
					}
				}
			}
		}
	}

	void FMC::reset_ref_nav_poi_data(std::vector<std::string>* nav_drs)
	{
		for (int i = 0; i < nav_drs->size(); i++)
		{
			std::string curr_dr = nav_drs->at(i);
			xp_databus->set_data_s(curr_dr, " ", -1);
		}
	}

	void FMC::update_ref_nav() // Updates ref nav data page
	{
		while (xp_databus->get_datai(in_drs.curr_page) == PAGE_REF_NAV_DATA &&
			!sim_shutdown.load(std::memory_order_relaxed))
		{
			std::string tmp = xp_databus->get_data_s(in_drs.ref_nav.poi_id);
			std::string icao;
			std::string icao_entry_last = dr_cache->get_val_s(in_drs.ref_nav.poi_id);

			strip_str(&tmp, &icao);

			if (icao != icao_entry_last)
			{
				dr_cache->set_val_s(in_drs.ref_nav.poi_id, icao);

				if (icao != "")
				{
					// Reset poi id so that only the current id gets displayed
					xp_databus->set_data_s(out_drs.ref_nav.poi_id, " ", -1);
					xp_databus->set_data_s(in_drs.ref_nav.poi_id, " ", -1);

					libnav::airport_data arpt_found{};
					std::vector<libnav::waypoint_entry> wpts_found = {};

					size_t n_arpts_found = nav_db->get_airport_data(icao, &arpt_found);
					size_t n_wpts_found = nav_db->get_wpt_data(icao, &wpts_found);

					if (n_arpts_found + n_wpts_found)
					{
						int ret = update_ref_nav_poi_data(n_arpts_found, n_wpts_found, icao, arpt_found, wpts_found);
						if (!ret)
						{
							break;
						}
					}
					else
					{
						xp_databus->set_data_s(out_drs.scratchpad_msg, "NOT IN DATA BASE");
						reset_ref_nav();
					}
				}
			}

			update_ref_nav_inhibit(&in_drs.ref_nav.in_navaids, { NAV_DME, NAV_DME_ONLY, NAV_VOR, NAV_VOR_DME }, RAD_NAV_INHIBIT, false);
			update_ref_nav_inhibit(&in_drs.ref_nav.in_vors, { NAV_VOR, NAV_VOR_DME }, RAD_NAV_VOR_ONLY_INHIBIT, true);

			int inh_curr = xp_databus->get_datai(in_drs.ref_nav.rad_nav_inh);
			int inh_last = dr_cache->get_val_i(in_drs.ref_nav.rad_nav_inh);

			if (inh_curr != inh_last)
			{
				dr_cache->set_val_i(in_drs.ref_nav.rad_nav_inh, inh_curr);
				reset_ref_nav_poi_data(&in_drs.ref_nav.in_navaids);
				reset_ref_nav_poi_data(&in_drs.ref_nav.in_vors);
			}

			update_scratch_msg();
		}
		reset_ref_nav();
	}

	void FMC::reset_ref_nav()
	{
		xp_databus->set_datai(out_drs.ref_nav.poi_type, 0);
		xp_databus->set_datad(out_drs.ref_nav.poi_lat, -1);
		xp_databus->set_datad(out_drs.ref_nav.poi_lon, -1);
		xp_databus->set_datad(out_drs.ref_nav.poi_elevation, -1);
		xp_databus->set_datad(out_drs.ref_nav.poi_freq, -1);
		xp_databus->set_data_s(out_drs.ref_nav.poi_mag_var, " ", -1);
	}
}