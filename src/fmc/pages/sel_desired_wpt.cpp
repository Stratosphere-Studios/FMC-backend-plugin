namespace StratosphereAvionics
{
	libnav::waypoint_entry FMC::update_sel_des_wpt(std::string id,
		std::vector<libnav::waypoint_entry> vec) // Updates SELECT DESIRED WPT page for navaids
	{
		int n_subpages = int(ceil(float(vec.size()) / float(N_CDU_OUT_LINES)));

		xp_databus->set_datai(out_drs.sel_desired_wpt.is_active, 1);
		xp_databus->set_datai(out_drs.sel_desired_wpt.n_subpages, n_subpages);

		geo::point ac_pos = get_ac_pos(); // Current aircraft position

		libnav::sort_wpt_entry_by_dist(&vec, ac_pos);

		int curr_subpage = 1;
		int subpage_prev = 0;
		int n_navaids_displayed = N_CDU_OUT_LINES;

		int start_idx = 0;

		int user_idx = xp_databus->get_datai(in_drs.sel_desired_wpt.poi_idx);

		libnav::waypoint_entry out_navaid{};

		while ((user_idx < 0 || user_idx >= n_navaids_displayed) && !sim_shutdown.load(std::memory_order_relaxed))
		{
			// If user decides to leave this page, reset and exit
			if (!xp_databus->get_datai(out_drs.sel_desired_wpt.is_active))
			{
				return out_navaid;
			}

			if (subpage_prev != curr_subpage)
			{
				// Wait until user has selected a valid waypoint
				start_idx = (curr_subpage - 1) * N_CDU_OUT_LINES;

				n_navaids_displayed = N_CDU_OUT_LINES;
				if (vec.size() - start_idx < n_navaids_displayed)
				{
					n_navaids_displayed = int(vec.size()) - start_idx;
				}

				xp_databus->set_datai(out_drs.sel_desired_wpt.n_pois, n_navaids_displayed);

				for (int i = start_idx; i < start_idx + n_navaids_displayed; i++)
				{
					double poi_freq = 0;
					double poi_lat = vec.at(i).pos.lat_deg;
					double poi_lon = vec.at(i).pos.lon_deg;

					// Make sure the pointer to navaid data isn't null.
					if (vec.at(i).navaid)
					{
						poi_freq = vec.at(i).navaid->freq;
					}

					std::string type_str = id + " " + libnav::navaid_to_str(vec.at(i).type);

					xp_databus->set_data_s(out_drs.sel_desired_wpt.poi_types.at(i - start_idx), type_str, i - start_idx);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_freq), (i - start_idx) * 3);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_lat), (i - start_idx) * 3 + 1);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_lon), (i - start_idx) * 3 + 2);
				}
				subpage_prev = curr_subpage;
			}

			curr_subpage = libnav::clamp(xp_databus->get_datai(in_drs.sel_desired_wpt.curr_page), n_subpages, 1);
			user_idx = xp_databus->get_datai(in_drs.sel_desired_wpt.poi_idx);
		}

		xp_databus->set_datai(out_drs.sel_desired_wpt.is_active, 0);

		if (user_idx != -1 && curr_subpage <= n_subpages)
		{

			return vec.at(user_idx + start_idx);
		}

		return out_navaid;
	}

	void FMC::reset_sel_navaid()
	{
		for (int i = 0; i < N_CDU_OUT_LINES; i++)
		{
			xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, 0, i * 3);
			xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, 0, i * 3 + 1);
			xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, 0, i * 3 + 2);
			xp_databus->set_data_s(out_drs.sel_desired_wpt.poi_types.at(i), "/0", -1);
		}
		xp_databus->set_datai(out_drs.sel_desired_wpt.n_subpages, 0);
		xp_databus->set_datai(in_drs.sel_desired_wpt.curr_page, 1);
		xp_databus->set_datai(in_drs.sel_desired_wpt.poi_idx, -1);
	}
}