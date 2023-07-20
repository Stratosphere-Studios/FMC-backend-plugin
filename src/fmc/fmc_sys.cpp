/*
	This source file contains the implementation of the fmc for 
	B77W by stratosphere studios.
*/

#include "fmc_sys.hpp"

#include "pages/sel_desired_wpt.cpp"
#include "pages/ref_nav.cpp"


namespace StratosphereAvionics
{
	// AvionicsSys definitions

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

	void AvionicsSys::excl_navaid(std::string id, int idx)
	{
		std::lock_guard<std::mutex> lock(navaid_inhibit_mutex);
		if (idx >= 0 && idx < navaid_inhibit.size())
		{
			navaid_inhibit[idx] = id;
		}
	}

	void AvionicsSys::excl_vor(std::string id, int idx)
	{
		std::lock_guard<std::mutex> lock(vor_inhibit_mutex);
		if (idx >= 0 && idx < vor_inhibit.size())
		{
			vor_inhibit[idx] = id;
		}
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

	void FMC::update_scratch_msg()
	{
		if (xp_databus->get_datai(in_drs.scratch_pad_msg_clear))
		{
			xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
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
				update_ref_nav();
			}
		}
	}

	FMC::~FMC()
	{
		delete[] dr_cache;
	}
}
