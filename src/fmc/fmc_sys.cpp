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
		navaid_db = new libnav::NavaidDB(fix_path, navaid_path, &waypoints, &navaids);
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

	void FMC::clear_screen()
	{
		int n = N_CDU_OUT_LINES;
		for (int i = 0; i < n; i++)
		{
			//xp_databus->set_data_s(out_drs.fmc_screen.at(i), " ", -1);
		}
	}

	libnav::navaid_entry FMC::update_sel_navaid(std::string id,
							   std::vector<libnav::navaid_entry>* vec) // Updates SELECT DESIRED WPT page for navaids
	{
		std::vector<libnav::navaid> tmp;

		for (size_t i = 0; i < vec->size(); i++)
		{
			tmp.push_back({ id, vec->at(i) });
		}

		double ac_lat = xp_databus->get_datad(in_drs.sim_ac_lat_deg);
		double ac_lon = xp_databus->get_datad(in_drs.sim_ac_lon_deg);

		geo::point ac_pos = { ac_lat, ac_lon }; // Current aircraft position

		libnav::sort_navaids_by_dist(&tmp, ac_pos);

		int curr_subpage = 1;
		int n_navaids_displayed_past = 0;
		
		int user_idx = xp_databus->get_datai(in_drs.sel_desired_wpt.poi_idx);

		libnav::navaid_entry out_navaid = {0, 0, {0, 0}, 0, 0, 0};

		while (user_idx == -1 && !sim_shutdown.load(std::memory_order_relaxed))
		{
			// Wait until user has selected a valid waypoint
			int start_idx = (curr_subpage - 1) * N_CDU_OUT_LINES;

			int n_navaids_displayed = N_CDU_OUT_LINES;
			if (tmp.size() - start_idx < n_navaids_displayed)
			{
				start_idx = int(tmp.size()) - start_idx;
			}

			if (n_navaids_displayed_past != n_navaids_displayed && n_navaids_displayed > 0)
			{
				for (int i = start_idx; i < start_idx + n_navaids_displayed; i++)
				{
					double poi_freq = tmp.at(i).data.freq;
					double poi_lat = tmp.at(i).data.pos.lat_deg;
					double poi_lon = tmp.at(i).data.pos.lon_deg;

					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_freq), (i - start_idx) * 3);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_lat), (i - start_idx) * 3 + 1);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_lon), (i - start_idx) * 3 + 2);
				}
				n_navaids_displayed_past = n_navaids_displayed;
			}

			user_idx = xp_databus->get_datai(in_drs.sel_desired_wpt.poi_idx);
		}

		if (user_idx != -1)
		{
			return tmp.at(user_idx).data;
		}

		return out_navaid;
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
				int poi_type = nav_db->get_poi_type(icao);
				if (poi_type == POI_AIRPORT)
				{
					libnav::airport_data tmp = {};
					size_t n_arpts_found = nav_db->get_airport_data(icao, &tmp);

					xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
					xp_databus->set_data_s(out_drs.ref_nav.poi_id, icao);
					xp_databus->set_datad(out_drs.ref_nav.poi_lat, tmp.pos.lat_deg);
					xp_databus->set_datad(out_drs.ref_nav.poi_lon, tmp.pos.lon_deg);
					xp_databus->set_datad(out_drs.ref_nav.poi_elevation, double(tmp.elevation_ft));
				}
				else if (poi_type == POI_NAVAID)
				{
					std::vector<libnav::navaid_entry> tmp = {};
					size_t n_navaids_found = nav_db->get_navaid_info(icao, &tmp);
					libnav::navaid_entry selected_navaid{};

					xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
					xp_databus->set_data_s(out_drs.ref_nav.poi_id, icao);

					if (n_navaids_found > 1)
					{
						selected_navaid = update_sel_navaid(icao, &tmp);
						if (selected_navaid.max_recv == 0)
						{
							break;
						}
						xp_databus->set_datai(in_drs.sel_desired_wpt.poi_idx, -1);

						clear_screen();
					}

					xp_databus->set_datad(out_drs.ref_nav.poi_lat, selected_navaid.pos.lat_deg);
					xp_databus->set_datad(out_drs.ref_nav.poi_lon, selected_navaid.pos.lon_deg);
					xp_databus->set_datad(out_drs.ref_nav.poi_freq, selected_navaid.freq);
				}
				else if (poi_type == POI_WAYPOINT)
				{
					std::vector<geo::point> tmp = {};
					size_t n_waypoints_found = nav_db->get_wpt_info(icao, &tmp);

					xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
					xp_databus->set_data_s(out_drs.ref_nav.poi_id, icao);
					xp_databus->set_datad(out_drs.ref_nav.poi_lat, tmp[0].lat_deg);
					xp_databus->set_datad(out_drs.ref_nav.poi_lon, tmp[0].lon_deg);
				}
				else
				{
					xp_databus->set_data_s(out_drs.ref_nav.poi_id, " ", -1);
					xp_databus->set_data_s(out_drs.scratchpad_msg, "NOT IN DATA BASE");
					xp_databus->set_datad(out_drs.ref_nav.poi_lat, -1);
					xp_databus->set_datad(out_drs.ref_nav.poi_lon, -1);
					xp_databus->set_datad(out_drs.ref_nav.poi_elevation, -1);
				}
			}
			update_scratch_msg();
		}
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
