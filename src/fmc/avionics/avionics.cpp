#include "avionics.hpp"


namespace StratosphereAvionics
{
	// AvionicsSys definitions

	// Public member functions:

	AvionicsSys::AvionicsSys(std::shared_ptr<XPDataBus::DataBus> databus, avionics_in_drs in, avionics_out_drs out,
		double cache_tile_size, int hz)
	{
		n_refresh_hz = hz;
		tile_size = cache_tile_size;
		ac_pos_last = {};

		in_drs = in;
		out_drs = out;

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

		clock = new libtime::Timer();

		// Initialize data bases

		apt_db = std::make_shared<libnav::ArptDB>(&airports, &runways, sim_apt_path, tgt_apt_path, tgt_rnw_path);
		navaid_db = std::make_shared<libnav::NavaidDB>(fix_path, navaid_path, &waypoints);
		nav_db = std::make_shared<libnav::NavDB>(apt_db, navaid_db);

		dr_cache = new XPDataBus::DataRefCache();

		navaid_tuner = new NavaidTuner(databus, in_drs.nav_tuner, out_drs.nav_tuner, rad_nav_cand_update_time_sec);
		navaid_selector = new NavaidSelector(databus, navaid_tuner, out_drs.nav_selector, cache_tile_size,
			min_navaid_dist_nm, rad_nav_cand_update_time_sec);
	}

	geo::point3d AvionicsSys::get_ac_pos()
	{
		std::lock_guard<std::mutex> lock(ac_pos_mutex);

		return ac_pos;
	}

	std::string AvionicsSys::get_fpln_dep_icao()
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		return pln.dep_apt.icao;
	}

	void AvionicsSys::set_fpln_dep_apt(libnav::airport apt)
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		pln.dep_apt = apt;
		xp_databus->set_data_s(out_drs.dep_icao, apt.icao);

		xp_databus->set_data_s(out_drs.dep_rnw, " ", -1);
	}

	libnav::airport AvionicsSys::get_fpln_arr_apt()
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		return pln.arr_apt;
	}

	std::string AvionicsSys::get_fpln_arr_icao()
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		return pln.arr_apt.icao;
	}

	void AvionicsSys::set_fpln_arr_apt(libnav::airport apt)
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		pln.arr_apt = apt;
		xp_databus->set_data_s(out_drs.arr_icao, apt.icao);
	}

	void AvionicsSys::set_fpln_dep_rnw(libnav::runway rnw)
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		pln.dep_rnw = rnw;
		xp_databus->set_data_s(out_drs.dep_rnw, "RW" + rnw.id);
	}

	void AvionicsSys::set_fpln_arr_rnw(libnav::runway rnw)
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		pln.arr_rnw = rnw;
	}

	void AvionicsSys::excl_navaid(std::string id, int idx)
	{
		std::lock_guard<std::mutex> lock(navaid_inhibit_mutex);
		if (idx >= 0 && idx < int(navaid_inhibit.size()))
		{
			rm_from_bl(navaid_inhibit[idx]); // Remove all previously blacklisted navaids

			if (id != "")
			{
				add_to_bl(id);

				xp_databus->set_data_s(out_drs.excl_navaids.at(idx), id);
			}
			else
			{
				xp_databus->set_data_s(out_drs.excl_navaids.at(idx), " ", -1);
			}
			navaid_inhibit[idx] = id;
		}
	}

	void AvionicsSys::excl_vor(std::string id, int idx)
	{
		std::lock_guard<std::mutex> lock(vor_inhibit_mutex);
		if (idx >= 0 && idx < int(vor_inhibit.size()))
		{
			rm_from_bl(vor_inhibit[idx]); // Remove all previously blacklisted navaids

			if (id != "")
			{
				add_to_bl(id);

				xp_databus->set_data_s(out_drs.excl_vors.at(idx), id);
			}
			else
			{
				xp_databus->set_data_s(out_drs.excl_vors.at(idx), " ", -1);
			}
			vor_inhibit[idx] = id;
		}
	}

	void AvionicsSys::update_sys()
	{
		update_ac_pos();

		navaid_tuner->set_ac_pos(ac_pos);

		navaid_selector->update(&waypoints, ac_pos, clock->get_curr_time());
	}

	void AvionicsSys::main_loop()
	{
		update_load_status();
		while (!sim_shutdown.load(UPDATE_FLG_ORDR))
		{
			update_sys();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / n_refresh_hz));
		}
	}

	void AvionicsSys::disable()
	{
		XPLMDebugString("777_FMS: Disabling avionics\n");
		delete navaid_selector;
		delete navaid_tuner;
		delete dr_cache;
		delete clock;
	}

	AvionicsSys::~AvionicsSys()
	{
		
	}

	// Private member functions:

	void AvionicsSys::update_load_status()
	{
		xp_databus->set_datai("Strato/777/UI/messages/creating_databases", 1);
		if (!sim_shutdown.load(UPDATE_FLG_ORDR))
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

	void AvionicsSys::update_ac_pos()
	{
		double baro_ft_1 = xp_databus->get_datad(in_drs.sim_baro_alt_ft1);
		double baro_ft_2 = xp_databus->get_datad(in_drs.sim_baro_alt_ft1);
		double baro_ft_3 = xp_databus->get_datad(in_drs.sim_baro_alt_ft1);
		std::lock_guard<std::mutex> lock(ac_pos_mutex);
		ac_pos.p.lat_deg = xp_databus->get_datad(in_drs.sim_ac_lat_deg);
		ac_pos.p.lon_deg = xp_databus->get_datad(in_drs.sim_ac_lon_deg);
		ac_pos.alt_ft = (baro_ft_1 + baro_ft_2 + baro_ft_3) / 3;
	}

	/*
		Blacklists all navaids with given id forever.
	*/

	void AvionicsSys::add_to_bl(std::string id)
	{
		std::vector<libnav::waypoint_entry> entries;
		nav_db->get_wpt_data(id, &entries);

		for (int i = 0; i < int(entries.size()); i++)
		{
			navaid_tuner->black_list->add_to_black_list(&id, &entries.at(i));
		}
	}

	/*
		Removes all navaids with given id from black list.
	*/

	void AvionicsSys::rm_from_bl(std::string id)
	{
		std::vector<libnav::waypoint_entry> entries;
		nav_db->get_wpt_data(id, &entries);

		for (int i = 0; i < int(entries.size()); i++)
		{
			libnav::waypoint tmp = { id, entries.at(i) };
			navaid_tuner->black_list->remove_from_black_list(&id, &entries.at(i));
		}
	}
}; // namespace StratosphereAvionics
