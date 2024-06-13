/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/
 
	This source file contains the implementation of the fmc for 
	B77W by stratosphere studios. Author: discord/bruh4096#4512
*/

#include "fmc_sys.hpp"


namespace StratosphereAvionics
{
	// FMC definitions:

	// Public member functions:

	FMC::FMC(std::shared_ptr<AvionicsSys> av, fmc_in_drs in, fmc_out_drs out, int hz)
	{
		n_refresh_hz = hz;

		avionics = av;
		navaid_db = avionics->navaid_db;
		apt_db = avionics->apt_db;
		in_drs = in;
		out_drs = out;

		xp_databus = avionics->xp_databus;

		dr_cache = new XPDataBus::DataRefCache();
	}

	geo::point FMC::get_ac_pos()
	{
		double ac_lat = xp_databus->get_datad(in_drs.sim_ac_lat_deg) * geo::DEG_TO_RAD;
		double ac_lon = xp_databus->get_datad(in_drs.sim_ac_lon_deg) * geo::DEG_TO_RAD;

		return { ac_lat, ac_lon };
	}

	// SEL DES WPT page:

	libnav::waypoint_entry_t FMC::update_sel_des_wpt(std::string id,
		std::vector<libnav::waypoint_entry_t> vec) // Updates SELECT DESIRED WPT page for navaids
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

		libnav::waypoint_entry_t out_navaid{};

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
				if (int(vec.size()) - start_idx < n_navaids_displayed)
				{
					n_navaids_displayed = int(vec.size()) - start_idx;
				}

				xp_databus->set_datai(out_drs.sel_desired_wpt.n_pois, n_navaids_displayed);

				for (int i = start_idx; i < start_idx + n_navaids_displayed; i++)
				{
					size_t i_idx = size_t(i);
					double poi_freq = 0;
					double poi_lat = vec.at(i_idx).pos.lat_rad * geo::RAD_TO_DEG;
					double poi_lon = vec.at(i_idx).pos.lon_rad * geo::RAD_TO_DEG;

					// Make sure the pointer to navaid data isn't null.
					if (vec.at(i_idx).navaid)
					{
						poi_freq = vec.at(i_idx).navaid->freq;
					}

					std::string type_str = id + " " + libnav::navaid_to_str(vec.at(i_idx).type);

					xp_databus->set_data_s(out_drs.sel_desired_wpt.poi_types.at(size_t(i - start_idx)), type_str);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_freq), (i - start_idx) * 3);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_lat), (i - start_idx) * 3 + 1);
					xp_databus->set_dataf(out_drs.sel_desired_wpt.poi_list, float(poi_lon), (i - start_idx) * 3 + 2);
				}
				subpage_prev = curr_subpage;
			}

			curr_subpage = libnav::clamp(xp_databus->get_datai(in_drs.sel_desired_wpt.curr_page), n_subpages, 1);
			user_idx = xp_databus->get_datai(in_drs.sel_desired_wpt.poi_idx);

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / n_refresh_hz));
		}

		xp_databus->set_datai(out_drs.sel_desired_wpt.is_active, 0);
		reset_sel_navaid();

		if (user_idx != -1 && curr_subpage <= n_subpages)
		{
			return vec.at(size_t(user_idx + start_idx));
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
			xp_databus->set_data_s(out_drs.sel_desired_wpt.poi_types.at(size_t(i)), " ", -1);
		}
		xp_databus->set_datai(out_drs.sel_desired_wpt.n_pois, 0);
		xp_databus->set_datai(out_drs.sel_desired_wpt.n_subpages, 0);
		xp_databus->set_datai(in_drs.sel_desired_wpt.curr_page, 1);
		xp_databus->set_datai(in_drs.sel_desired_wpt.poi_idx, -1);
	}

	// REF NAV DATA page:

	int FMC::update_ref_nav_poi_data(int n_arpt_found, int n_rwys_found, int n_wpts_found, std::string icao,
		libnav::airport_data_t arpt_found, libnav::runway_entry_t rwy_found,
		std::vector<libnav::waypoint_entry_t> wpts_found)
	{
		xp_databus->set_data_s(out_drs.ref_nav.poi_id, icao);

		double poi_lat, poi_lon;

		if (n_arpt_found)
		{
			xp_databus->set_datai(out_drs.ref_nav.poi_type, POI_AIRPORT);
			poi_lat = arpt_found.pos.lat_rad * geo::RAD_TO_DEG;
			poi_lon = arpt_found.pos.lon_rad * geo::RAD_TO_DEG;

			xp_databus->set_datai(out_drs.ref_nav.poi_elevation, int(arpt_found.elevation_ft));
		}
		else if (n_rwys_found)
		{
			double rwy_length_m = rwy_found.get_impl_length_m() - rwy_found.displ_threshold_m;
			double rwy_length_ft = rwy_length_m * geo::M_TO_FT;
			xp_databus->set_datai(out_drs.ref_nav.poi_type, POI_RWY);
			xp_databus->set_datai(out_drs.ref_nav.poi_length_m, int(rwy_length_m));
			xp_databus->set_datai(out_drs.ref_nav.poi_length_ft, int(rwy_length_ft));
			xp_databus->set_datai(out_drs.ref_nav.poi_elevation, int(arpt_found.elevation_ft));
			poi_lat = rwy_found.start.lat_rad * geo::RAD_TO_DEG;
			poi_lon = rwy_found.start.lon_rad * geo::RAD_TO_DEG;
		}
		else
		{
			libnav::waypoint_entry_t curr_wpt = wpts_found.at(0);

			if (n_wpts_found > 1)
			{
				curr_wpt = update_sel_des_wpt(icao, wpts_found);

				if (curr_wpt.type == libnav::NavaidType::NONE) // If shutdown commenced, return.
				{
					return 0;
				}
			}

			if (curr_wpt.type == libnav::NavaidType::WAYPOINT)
			{
				xp_databus->set_datai(out_drs.ref_nav.poi_type, POI_WAYPOINT);
			}
			else
			{
				xp_databus->set_datai(out_drs.ref_nav.poi_type, POI_NAVAID);
				xp_databus->set_datad(out_drs.ref_nav.poi_freq, curr_wpt.navaid->freq);
			}

			poi_lat = curr_wpt.pos.lat_rad * geo::RAD_TO_DEG;
			poi_lon = curr_wpt.pos.lon_rad * geo::RAD_TO_DEG;
		}

		double mag_var = xp_databus->get_mag_var(poi_lat, poi_lon);
		std::string mag_var_str = strutils::mag_var_to_str(mag_var);

		xp_databus->set_datad(out_drs.ref_nav.poi_lat, poi_lat);
		xp_databus->set_datad(out_drs.ref_nav.poi_lon, poi_lon);
		xp_databus->set_data_s(out_drs.ref_nav.poi_mag_var, mag_var_str);

		return 1;
	}

	void FMC::update_ref_nav_inhibit(std::vector<std::string>* nav_drs, 
		libnav::NavaidType types, ref_nav threshold, bool add_vor)
	{
		for (size_t i = 0; i < nav_drs->size(); i++)
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
					if (xp_databus->get_datai(in_drs.ref_nav.rad_nav_inh) > static_cast<int>(threshold))
					{
						if (navaid_db->is_navaid_of_type(entry_curr, types))
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
							size_t msg_idx = out_drs.scratch_msg.not_in_db_idx;
							xp_databus->set_datai(out_drs.scratch_msg.dr_list[msg_idx], 1);
						}
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
		for (size_t i = 0; i < nav_drs->size(); i++)
		{
			std::string curr_dr = nav_drs->at(i);
			xp_databus->set_data_s(curr_dr, " ", -1);
		}
	}

	int FMC::update_ref_nav(std::string icao)
	{
		std::string tmp = icao;

		libnav::airport_data_t arpt_found{};
		libnav::runway_entry_t rwy_found{};
		std::vector<libnav::waypoint_entry_t> wpts_found{};

		int n_arpts_found = 0, n_wpts_found = 0, n_rwys_found = 0;

		if (std::isdigit(icao[0]))
		{
			arpt_found = avionics->get_fpln_arr_apt().data;
			n_rwys_found = get_arrival_rwy_data(icao, &rwy_found);
			tmp = "RW" + tmp;
		}
		else if (icao.length() == 5 && icao[0] == 'R' && icao[1] == 'W' && std::isdigit(icao[2]))
		{
			arpt_found = avionics->get_fpln_arr_apt().data;
			n_rwys_found = get_arrival_rwy_data(icao.substr(2, icao.length() - 1), &rwy_found);
		}
		else
		{
			n_arpts_found = int(apt_db->get_airport_data(icao, &arpt_found));
			n_wpts_found = int(navaid_db->get_wpt_data(icao, &wpts_found));
		}

		if (n_arpts_found + n_wpts_found + n_rwys_found)
		{
			int ret = update_ref_nav_poi_data(n_arpts_found, n_rwys_found, n_wpts_found,
				tmp, arpt_found, rwy_found, wpts_found);
			if (!ret)
			{
				return 0;
			}
		}
		else
		{
			// Trigger NOT IN DATA BASE scratch pad message
			size_t msg_idx = out_drs.scratch_msg.not_in_db_idx;
			xp_databus->set_datai(out_drs.scratch_msg.dr_list[msg_idx], 1);
		}
		return 1;
	}

	void FMC::ref_nav_main_loop() // Updates ref nav data page
	{
		while (xp_databus->get_datai(in_drs.curr_page) == static_cast<int>(fmc_pages::PAGE_REF_NAV_DATA) &&
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
					// Reset poi id so that the it isn't corrupted
					xp_databus->set_data_s(in_drs.ref_nav.poi_id, " ", -1);

					int ret = update_ref_nav(icao);

					if (!ret) return; // If shutdown has commenced, exit.
				}
			}

			
			update_ref_nav_inhibit(&in_drs.ref_nav.in_navaids, libnav::NavaidType::VHF_NAVAID, 
				ref_nav::RAD_NAV_INHIBIT, false);

			libnav::NavaidType vor_tp = libnav::NavaidType(
				static_cast<int>(libnav::NavaidType::VOR) + 
				static_cast<int>(libnav::NavaidType::VOR_DME));

			update_ref_nav_inhibit(&in_drs.ref_nav.in_vors, vor_tp, 
				ref_nav::RAD_NAV_VOR_ONLY_INHIBIT, true);

			int inh_curr = xp_databus->get_datai(in_drs.ref_nav.rad_nav_inh);
			int inh_last = dr_cache->get_val_i(in_drs.ref_nav.rad_nav_inh);

			if (inh_curr != inh_last)
			{
				dr_cache->set_val_i(in_drs.ref_nav.rad_nav_inh, inh_curr);
				reset_ref_nav_poi_data(&in_drs.ref_nav.in_navaids);
				reset_ref_nav_poi_data(&in_drs.ref_nav.in_vors);
			}

			update_scratch_msg();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / n_refresh_hz));
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
		xp_databus->set_data_s(out_drs.ref_nav.poi_id, " ", -1);
		xp_databus->set_datai(out_drs.ref_nav.poi_length_ft, 0);
		xp_databus->set_datai(out_drs.ref_nav.poi_length_m, 0);
	}

	// RTE1 page:

	/*
			Updates airport icao codes on any rte page.
			If user has entered a valid icao, returns true.
			Otherwise, returns false.
	*/

	bool FMC::update_rte_apt(std::string in_dr, libnav::airport_data_t* apt_data, 
		libnav::runway_data* rnw_data)
	{
		std::string tmp = xp_databus->get_data_s(in_dr);
		std::string icao_curr;
		std::string icao_last = dr_cache->get_val_s(in_dr);

		strip_str(&tmp, &icao_curr);

		if (icao_curr != icao_last)
		{
			dr_cache->set_val_s(in_dr, icao_curr);

			int n_arpts = apt_db->get_airport_data(icao_curr, apt_data);

			if (n_arpts)
			{
				apt_db->get_apt_rwys(icao_curr, rnw_data);

				return true;
			}
			else
			{
				size_t msg_idx = out_drs.scratch_msg.not_in_db_idx;
				xp_databus->set_datai(out_drs.scratch_msg.dr_list[msg_idx], 1);
			}
		}

		return false;
	}

	void FMC::update_rte1()
	{
		libnav::airport_data_t dep_data;
		libnav::runway_data dep_runways;
		libnav::airport_data_t arr_data;
		libnav::runway_data arr_runways;

		while (xp_databus->get_datai(in_drs.curr_page) == static_cast<int>(fmc_pages::PAGE_RTE1) &&
			!sim_shutdown.load(std::memory_order_relaxed))
		{
			bool ret1 = update_rte_apt(in_drs.rte1.dep_icao, &dep_data, &dep_runways);
			bool ret2 = update_rte_apt(in_drs.rte1.arr_icao, &arr_data, &arr_runways);

			if (ret1)
			{
				std::string dep_icao = xp_databus->get_data_s(in_drs.rte1.dep_icao);
				avionics->set_fpln_dep_apt({ dep_icao,  dep_data });
			}
			if (ret2)
			{
				std::string arr_icao = xp_databus->get_data_s(in_drs.rte1.arr_icao);
				avionics->set_fpln_arr_apt({ arr_icao,  arr_data });
			}

			std::string tmp = xp_databus->get_data_s(in_drs.rte1.dep_rnw);
			std::string rnw_curr;
			std::string rnw_last = dr_cache->get_val_s(in_drs.rte1.dep_rnw);

			strip_str(&tmp, &rnw_curr);

			if (rnw_curr != "")
			{
				xp_databus->set_data_s(in_drs.rte1.dep_rnw, " ", -1);
				if (dep_runways.find(rnw_curr) != dep_runways.end())
				{
					avionics->set_fpln_dep_rnw({ rnw_curr, dep_runways.at(rnw_curr) });
				}
				else
				{
					size_t msg_idx = out_drs.scratch_msg.not_in_db_idx;
					xp_databus->set_datai(out_drs.scratch_msg.dr_list[msg_idx], 1);
				}
			}

			update_scratch_msg();

			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / n_refresh_hz));
		}
	}

	// MISC:

	/*
		This function clears all of the scratch pad messages when the clear dataref has been set to 1.
	*/

	void FMC::update_scratch_msg()
	{
		if (xp_databus->get_datai(in_drs.scratch_pad_msg_clear))
		{
			for (size_t i = 0; i < out_drs.scratch_msg.dr_list.size(); i++)
			{
				xp_databus->set_datai(out_drs.scratch_msg.dr_list[i], 0);
			}
			xp_databus->set_datai(in_drs.scratch_pad_msg_clear, 0);
		}
	}

	void FMC::main_loop()
	{
		while (!sim_shutdown.load(UPDATE_FLG_ORDR))
		{
			fmc_pages page = static_cast<fmc_pages>(xp_databus->get_datai(in_drs.curr_page));
			switch (page)
			{
			case fmc_pages::PAGE_REF_NAV_DATA:
				ref_nav_main_loop();
				continue;
			case fmc_pages::PAGE_RTE1:
				update_rte1();
				continue;
			default:
				continue;
			}
		}
	}

	void FMC::disable()
	{
		XPLMDebugString("777_FMS: Disabling fmc\n");
		delete dr_cache;
	}

	FMC::~FMC()
	{
		
	}

	// Private member functions:

	int FMC::get_arrival_rwy_data(std::string rwy_id, libnav::runway_entry_t* out)
	{
		std::string arr_icao = avionics->get_fpln_arr_icao();
		return apt_db->get_rnw_data(arr_icao, rwy_id, out);
	}
}
