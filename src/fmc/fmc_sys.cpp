/*
	This source file contains the implementation of the fmc for 
	B77W by stratosphere studios. Author: discord/bruh4096#4512
*/

#include "fmc_sys.hpp"

#include "pages/ref_nav.cpp"
#include "pages/rte1.cpp"
#include "pages/sel_desired_wpt.cpp"


namespace StratosphereAvionics
{
	// AvionicsSys definitions

	// Public member functions:

	AvionicsSys::AvionicsSys(std::shared_ptr<XPDataBus::DataBus> databus, avionics_out_drs out)
	{
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

		// Initialize data bases

		apt_db = new libnav::ArptDB(&airports, &runways, sim_apt_path, tgt_apt_path, tgt_rnw_path);
		navaid_db = new libnav::NavaidDB(fix_path, navaid_path, &waypoints);
		nav_db = new libnav::NavDB(apt_db, navaid_db);

		dr_cache = new XPDataBus::DataRefCache();
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
		xp_databus->set_data_s(out_drs.dep_rnw, "RW"+rnw.id);
	}

	void AvionicsSys::set_fpln_arr_rnw(libnav::runway rnw)
	{
		std::lock_guard<std::mutex> lock(fpln_mutex);
		pln.arr_rnw = rnw;
	}

	void AvionicsSys::excl_navaid(std::string id, int idx)
	{
		std::lock_guard<std::mutex> lock(navaid_inhibit_mutex);
		if (idx >= 0 && idx < navaid_inhibit.size())
		{
			navaid_inhibit[idx] = id;
			if (id != "")
			{
				xp_databus->set_data_s(out_drs.excl_navaids.at(idx), id);
			}
			else
			{
				xp_databus->set_data_s(out_drs.excl_navaids.at(idx), "\0", -1);
			}
		}
	}

	void AvionicsSys::excl_vor(std::string id, int idx)
	{
		std::lock_guard<std::mutex> lock(vor_inhibit_mutex);
		if (idx >= 0 && idx < vor_inhibit.size())
		{
			vor_inhibit[idx] = id;
			if (id != "")
			{
				xp_databus->set_data_s(out_drs.excl_vors.at(idx), id);
			}
			else
			{
				xp_databus->set_data_s(out_drs.excl_vors.at(idx), "\0", -1);
			}
		}
	}

	void AvionicsSys::update_sys()
	{

	}

	void AvionicsSys::main_loop()
	{
		update_load_status();
		while (!sim_shutdown.load(std::memory_order_relaxed))
		{
			update_sys();
		}
	}

	AvionicsSys::~AvionicsSys()
	{
		delete[] nav_db;
		delete[] apt_db;
		delete[] navaid_db;
		delete[] dr_cache;
	}

	// Private member functions:

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

	// FMC definitions:

	// Public member functions:

	FMC::FMC(std::shared_ptr<AvionicsSys> av, fmc_in_drs in, fmc_out_drs out)
	{
		avionics = av;
		nav_db = avionics->nav_db;
		in_drs = in;
		out_drs = out;

		xp_databus = avionics->xp_databus;

		dr_cache = new XPDataBus::DataRefCache();
	}

	geo::point FMC::get_ac_pos()
	{
		double ac_lat = xp_databus->get_datad(in_drs.sim_ac_lat_deg);
		double ac_lon = xp_databus->get_datad(in_drs.sim_ac_lon_deg);

		return { ac_lat, ac_lon };
	}

	void FMC::update_scratch_msg()
	{
		if (xp_databus->get_datai(in_drs.scratch_pad_msg_clear))
		{
			for (int i = 0; i < out_drs.scratch_msg.dr_list.size(); i++)
			{
				xp_databus->set_datai(out_drs.scratch_msg.dr_list[i], 0);
			}
			xp_databus->set_datai(in_drs.scratch_pad_msg_clear, 0);
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
				ref_nav_main_loop();
			case PAGE_RTE1:
				update_rte1();
			}
		}
	}

	FMC::~FMC()
	{
		delete[] dr_cache;
	}

	// Private member functions:

	int FMC::get_arrival_rwy_data(std::string rwy_id, libnav::runway_entry* out)
	{
		std::string arr_icao = avionics->get_fpln_arr_icao();
		return nav_db->get_rnw_data(arr_icao, rwy_id, out);
	}
}
