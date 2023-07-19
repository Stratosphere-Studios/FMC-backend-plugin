/*
	This source file contains the implementation of the fmc for 
	B77W by stratosphere studios.
*/

#include "fmc_sys.hpp"


namespace StratosphereAvionics
{
	//AvionicsSys definitions

	AvionicsSys::AvionicsSys(std::shared_ptr<XPDataBus::DataBus> databus)
	{
		xp_databus = databus;
		strcpy_safe(path_sep, 2, xp_databus->path_sep); // Update path separator
		xplane_path = xp_databus->xplane_path;
		prefs_path = xp_databus->prefs_path;
		xplane_version = xp_databus->xplane_version;

		sim_apt_path = xp_databus->apt_dat_path;
		std::string tgt_apt_path = prefs_path + "Strato_777_apt.dat";
		std::string tgt_rnw_path = prefs_path + "Strato_777_rnw.dat";

		std::string fix_path = xp_databus->default_data_path + "earth_fix.dat";
		std::string navaid_path = xp_databus->default_data_path + "earth_nav.dat";

		airports = {};
		runways = {};

		// Initialize data bases

		apt_db = new libnav::ArptDB(&airports, &runways, sim_apt_path, tgt_apt_path, tgt_rnw_path);
		navaid_db = new libnav::NavaidDB(fix_path, navaid_path, &waypoints);
		nav_db = new libnav::NavDB(apt_db, navaid_db);
	}

	void AvionicsSys::update_load_status()
	{
		xp_databus->set_datai("Strato/777/UI/messages/creating_databases", 1);
		if (!sim_shutdown.load(std::memory_order_seq_cst))
		{
			bool sts = nav_db->is_loaded();
			if (!sts)
			{
				xp_databus->set_datai("Strato/777/UI/messages/creating_databases", -1);
				return;
			}
		}
		xp_databus->set_datai("Strato/777/UI/messages/creating_databases", 0);
	}

	void AvionicsSys::update_sys()
	{
		
	}

	void AvionicsSys::main_loop()
	{
		update_load_status();
		while (!sim_shutdown.load(std::memory_order_seq_cst))
		{
			update_sys();
		}
	}

	AvionicsSys::~AvionicsSys()
	{
		delete[] nav_db;
		delete[] apt_db;
		delete[] navaid_db;
	}

	//FMC definitions:

	FMC::FMC(std::shared_ptr<AvionicsSys> av, fmc_in_drs* in, fmc_out_drs* out)
	{
		avionics = av;
		nav_db = avionics->nav_db;
		memcpy(&in_drs, in, sizeof(fmc_in_drs));
		memcpy(&out_drs, out, sizeof(fmc_out_drs));

		xp_databus = avionics->xp_databus;

		dr_cache = new XPDataBus::DataRefCache();
	}

	geo::point FMC::get_ac_pos()
	{
		double ac_lat = xp_databus->get_datad(in_drs.sim_ac_lat_deg);
		double ac_lon = xp_databus->get_datad(in_drs.sim_ac_lon_deg);

		return { ac_lat, ac_lon };
	}

	libnav::waypoint_entry FMC::update_sel_navaid(std::string id,
							   std::vector<libnav::waypoint_entry> vec) // Updates SELECT DESIRED WPT page for navaids
	{
		int n_subpages = int(ceil(float(vec.size()) / float(N_CDU_OUT_LINES)));

		xp_databus->set_datai(out_drs.sel_desired_wpt.is_active, 1);
		xp_databus->set_datai(out_drs.sel_desired_wpt.n_subpages, n_subpages);

		geo::point ac_pos = get_ac_pos(); // Current aircraft position

		libnav::sort_wpt_entry_by_dist(&vec, ac_pos);

		int curr_subpage = xp_databus->get_datai(in_drs.sel_desired_wpt.curr_page);
		int subpage_prev = 0;

		int start_idx = 0;
		
		int user_idx = xp_databus->get_datai(in_drs.sel_desired_wpt.poi_idx);

		libnav::waypoint_entry out_navaid{};

		while ((user_idx == -1 || user_idx >= N_CDU_OUT_LINES) && !sim_shutdown.load(std::memory_order_relaxed))
		{
			// If user decides to leave this page, reset and exit
			if (!xp_databus->get_datai(out_drs.sel_desired_wpt.is_active))
			{
				return out_navaid;
			}

			// Wait until user has selected a valid waypoint
			start_idx = (curr_subpage - 1) * N_CDU_OUT_LINES;

			int n_navaids_displayed = N_CDU_OUT_LINES;
			if (vec.size() - start_idx < n_navaids_displayed)
			{
				n_navaids_displayed = int(vec.size()) - start_idx;
			}

			if (subpage_prev != curr_subpage)
			{
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

					xp_databus->set_datai(out_drs.sel_desired_wpt.poi_types, vec.at(i).type, i - start_idx);
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
		}
		xp_databus->set_datai(out_drs.sel_desired_wpt.n_subpages, 0);
		xp_databus->set_datai(in_drs.sel_desired_wpt.curr_page, 1);
		xp_databus->set_datai(in_drs.sel_desired_wpt.poi_idx, -1);
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

			if (icao != icao_entry_last && icao != "")
			{
				// Reset poi id so that only the current id gets displayed
				xp_databus->set_data_s(out_drs.ref_nav.poi_id, " ", -1);

				dr_cache->set_val_s(in_drs.ref_nav.poi_id, icao);

				libnav::airport_data arpt_found{};
				std::vector<libnav::waypoint_entry> wpts_found = {};

				size_t n_arpts_found = nav_db->get_airport_data(icao, &arpt_found);
				size_t n_wpts_found = nav_db->get_wpt_data(icao, &wpts_found);

				if (n_arpts_found + n_wpts_found)
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
							curr_wpt = update_sel_navaid(icao, wpts_found);

							reset_sel_navaid();
						}

						if (curr_wpt.type == NAV_NONE)
						{
							break;
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

					xp_databus->set_datad(out_drs.ref_nav.poi_lat, poi_lat);
					xp_databus->set_datad(out_drs.ref_nav.poi_lon, poi_lon);
					xp_databus->set_datad(out_drs.ref_nav.poi_mag_var, mag_var);
				}
				else
				{
					xp_databus->set_data_s(out_drs.scratchpad_msg, "NOT IN DATA BASE");
					reset_ref_nav();
				}
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
		xp_databus->set_datad(out_drs.ref_nav.poi_mag_var, 0);
	}

	void FMC::update_scratch_msg()
	{
		if (avionics->xp_databus->get_datai(in_drs.scratch_pad_msg_clear))
		{
			avionics->xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
		}
	}

	void FMC::main_loop()
	{
		while (!sim_shutdown.load(std::memory_order_relaxed))
		{
			int page = xp_databus->get_datai(in_drs.curr_page);
			switch (page)
			{
			case PAGE_REF_NAV_DATA:
				update_ref_nav();
			}
		}
	}

	FMC::~FMC()
	{
		delete[] dr_cache;
	}
}
