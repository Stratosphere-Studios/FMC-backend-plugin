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

		apt_db = new libnav::ArptDB(&airports, &runways, sim_apt_path, tgt_apt_path, tgt_rnw_path, 0, 0);
		navaid_db = new libnav::NavaidDB(fix_path, navaid_path, &waypoints, &navaids);
		nav_db = new libnav::NavDB(apt_db, navaid_db);
	}

	void AvionicsSys::update_load_status()
	{
		xp_databus->set_datai("Strato/777/UI/messages/creating_databases", 1);
		if (!sim_shutdown.load(std::memory_order_seq_cst))
		{
			int sts = apt_db->get_load_status() * navaid_db->get_load_status();
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

	void FMC::update_scratch_msg()
	{
		if (avionics->xp_databus->get_datai(in_drs.scratch_pad_msg_clear))
		{
			avionics->xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
		}
	}

	void FMC::update_ref_nav() // Updates ref nav data page
	{
		std::string tmp = xp_databus->get_data_s(in_drs.ref_nav_in_id);
		std::string icao;
		std::string icao_entry_last = dr_cache->get_val_s(in_drs.ref_nav_in_id);

		strip_str(&tmp, &icao);

		if (icao != icao_entry_last)
		{
			dr_cache->set_val_s(in_drs.ref_nav_in_id, icao);
			int poi_type = nav_db->get_poi_type(icao);
			if (poi_type == POI_AIRPORT)
			{
				libnav::airport_data tmp = {};
				size_t n_arpts_found = nav_db->get_airport_data(icao, &tmp);

				xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
				xp_databus->set_data_s(out_drs.poi_name, icao);
				xp_databus->set_datad(out_drs.poi_lat, tmp.pos.lat_deg);
				xp_databus->set_datad(out_drs.poi_lon, tmp.pos.lon_deg);
				xp_databus->set_datad(out_drs.poi_elevation, double(tmp.elevation_ft));
			}
			else if (poi_type == POI_NAVAID)
			{
				std::vector<libnav::navaid_entry> tmp = {};
				size_t n_navaids_found = nav_db->get_navaid_info(icao, &tmp);

				xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
				xp_databus->set_data_s(out_drs.poi_name, icao);
				xp_databus->set_datad(out_drs.poi_lat, tmp[0].wpt.lat_deg);
				xp_databus->set_datad(out_drs.poi_lon, tmp[0].wpt.lon_deg);
			}
			else if (poi_type == POI_WAYPOINT)
			{
				std::vector<geo::point> tmp = {};
				size_t n_waypoints_found = nav_db->get_wpt_info(icao, &tmp);

				xp_databus->set_data_s(out_drs.scratchpad_msg, " ", -1);
				xp_databus->set_data_s(out_drs.poi_name, icao);
				xp_databus->set_datad(out_drs.poi_lat, tmp[0].lat_deg);
				xp_databus->set_datad(out_drs.poi_lon, tmp[0].lon_deg);
			}
			else
			{
				xp_databus->set_data_s(out_drs.poi_name, " ", -1);
				xp_databus->set_data_s(out_drs.scratchpad_msg, "NOT IN DATA BASE");
				xp_databus->set_datad(out_drs.poi_lat, -1);
				xp_databus->set_datad(out_drs.poi_lon, -1);
				xp_databus->set_datad(out_drs.poi_elevation, -1);
			}
		}
	}

	void FMC::main_loop()
	{
		while (!sim_shutdown.load(std::memory_order_seq_cst))
		{
			update_ref_nav();
			update_scratch_msg();
		}
	}

	FMC::~FMC()
	{
		delete[] dr_cache;
	}
}
