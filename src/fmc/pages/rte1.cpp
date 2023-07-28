namespace StratosphereAvionics
{
	/*
			Updates airport icao codes on any rte page.
			If user has entered a valid icao, returns true.
			Otherwise, returns false.
	*/

	bool FMC::update_rte_apt(std::string in_dr, libnav::airport_data* apt_data, libnav::runway_data* rnw_data)
	{
		std::string tmp = xp_databus->get_data_s(in_dr);
		std::string icao_curr;
		std::string icao_last = dr_cache->get_val_s(in_dr);

		strip_str(&tmp, &icao_curr);

		if (icao_curr != icao_last)
		{
			dr_cache->set_val_s(in_dr, icao_curr);

			size_t n_arpts = nav_db->get_airport_data(icao_curr, apt_data);

			if (n_arpts)
			{
				size_t n_rnw = nav_db->get_runway_data(icao_curr, rnw_data);

				return true;
			}
			else
			{
				int msg_idx = out_drs.scratch_msg.not_in_db_idx;
				xp_databus->set_datai(out_drs.scratch_msg.dr_list[msg_idx], 1);
			}
		}

		return false;
	}

	void FMC::update_rte1()
	{
		libnav::airport_data dep_data;
		libnav::runway_data dep_runways;
		libnav::airport_data arr_data;
		libnav::runway_data arr_runways;

		bool dep_rnw_updated = false;

		while (xp_databus->get_datai(in_drs.curr_page) == PAGE_RTE1 &&
			!sim_shutdown.load(std::memory_order_relaxed))
		{
			bool ret1 = update_rte_apt(in_drs.rte1.dep_icao, &dep_data, &dep_runways);
			bool ret2 = update_rte_apt(in_drs.rte1.arr_icao, &arr_data, &arr_runways);

			if (ret1)
			{
				std::string dep_icao = xp_databus->get_data_s(in_drs.rte1.dep_icao);
				avionics->set_fpln_dep_apt({ dep_icao,  dep_data });
			}
			else if(!dep_rnw_updated)
			{
				dep_rnw_updated = true;
				std::string dep_icao = xp_databus->get_data_s(in_drs.rte1.dep_icao);
				size_t n_rnw = nav_db->get_runway_data(dep_icao, &dep_runways);
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

			if (rnw_curr != rnw_last)
			{
				dr_cache->set_val_s(in_drs.rte1.dep_rnw, rnw_curr);
				xp_databus->set_data_s(in_drs.rte1.dep_rnw, " ", -1);
				if (dep_runways.find(rnw_curr) != dep_runways.end())
				{
					avionics->set_fpln_dep_rnw({ rnw_curr, dep_runways.at(rnw_curr) });
				}
				else
				{
					int msg_idx = out_drs.scratch_msg.not_in_db_idx;
					xp_databus->set_datai(out_drs.scratch_msg.dr_list[msg_idx], 1);
				}
			}

			update_scratch_msg();
		}
	}
}
