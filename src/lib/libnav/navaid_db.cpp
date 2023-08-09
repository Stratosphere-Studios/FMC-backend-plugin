#include "navaid_db.hpp"

namespace libnav
{
	bool WaypointEntryCompare::operator()(waypoint_entry w1, waypoint_entry w2)
	{
		double d1 = w1.pos.get_great_circle_distance_nm(ac_pos);
		double d2 = w2.pos.get_great_circle_distance_nm(ac_pos);
		return d1 < d2;
	}

	bool WaypointCompare::operator()(waypoint w1, waypoint w2)
	{
		double d1 = w1.data.pos.get_great_circle_distance_nm(ac_pos);
		double d2 = w2.data.pos.get_great_circle_distance_nm(ac_pos);
		return d1 < d2;
	}

	NavaidDB::NavaidDB(std::string wpt_path, std::string navaid_path, wpt_db_t* wpt_db)
	{
		// Pre-defined stuff

		comp_types[NAV_ILS_FULL] = 1;
		comp_types[NAV_VOR_DME] = 1;
		comp_types[NAV_ILS_DME] = 1;

		// Paths

		sim_wpt_db_path = wpt_path;
		sim_navaid_db_path = navaid_path;

		wpt_cache = wpt_db;

		wpt_loaded = std::async(std::launch::async, [](NavaidDB* db) -> int {return db->load_waypoints(); }, this);
		navaid_loaded = std::async(std::launch::async, [](NavaidDB* db) -> int {return db->load_navaids(); }, this);
	}

	// Public member functions:

	bool NavaidDB::is_loaded()
	{
		return bool(wpt_loaded.get() * navaid_loaded.get());
	}

	NavaidDB::~NavaidDB()
	{

	}

	int NavaidDB::load_waypoints()
	{
		std::ifstream file(sim_wpt_db_path);
		if (file.is_open())
		{
			std::string line;
			int i = 0;
			int limit = N_NAVAID_LINES_IGNORE;
			while (getline(file, line) && line != "99")
			{
				if (i >= limit)
				{
					// Construct a waypoint entry.
					std::stringstream s(line);
					std::string junk;
					waypoint wpt = { "", { NAV_WAYPOINT, {0, 0}, nullptr } };
					s >> wpt.data.pos.lat_deg >> wpt.data.pos.lon_deg >> wpt.id >> junk;
					add_to_wpt_cache(wpt);
				}
				i++;
			}
			file.close();
			return 1;
		}
		return 0;
	}

	int NavaidDB::load_navaids()
	{
		std::ifstream file(sim_navaid_db_path);
		if (file.is_open())
		{
			std::string line;
			int i = 0;
			int limit = N_NAVAID_LINES_IGNORE;
			while (getline(file, line))
			{
				std::string check_val;
				std::stringstream s(line);
				s >> check_val;
				if (i >= limit && check_val != "99")
				{
					// Construct a navaid entry.
					std::stringstream s(line);
					uint16_t type, max_recv;
					double lat, lon, elevation, mag_var;
					uint32_t freq;
					std::string id;
					std::string junk;

					waypoint wpt;
					navaid_entry navaid;

					s >> type >> lat >> lon >> elevation >> freq >> max_recv >> mag_var >> id >> junk;

					wpt.id = id;
					wpt.data.type = type;
					navaid.max_recv = max_recv;
					wpt.data.pos.lat_deg = lat;
					wpt.data.pos.lon_deg = lon;
					navaid.elevation = elevation;
					navaid.freq = freq;
					add_to_navaid_cache(wpt, navaid);
				}
				else if (check_val == "99")
				{
					break;
				}
				i++;
			}
			file.close();
			return 1;
		}
		return 0;
	}

	bool NavaidDB::is_wpt(std::string id) 
	{
		std::lock_guard<std::mutex> lock(wpt_db_mutex);
		return wpt_cache->find(id) != wpt_cache->end();
	}

	bool NavaidDB::is_navaid_of_type(std::string id, std::vector<int> types)
	{
		if (is_wpt(id))
		{
			std::lock_guard<std::mutex> lock(wpt_db_mutex);
			uint16_t arr[NAV_ILS_DME];
			for (int i = 0; i < types.size(); i++)
			{
				arr[types.at(i)] = 1;
			}
			std::vector<waypoint_entry>* entries = &wpt_cache->at(id);
			for (int i = 0; i < entries->size(); i++)
			{
				uint16_t type = entries->at(i).type;
				if (arr[type])
				{
					return true;
				}
			}
		}
		return false;
	}

	int NavaidDB::get_wpt_data(std::string id, std::vector<waypoint_entry>* out)
	{
		if (is_wpt(id))
		{
			std::lock_guard<std::mutex> lock(wpt_db_mutex);
			std::vector<waypoint_entry>* waypoints = &wpt_cache->at(id);
			int n_waypoints = int(waypoints->size());
			for (int i = 0; i < n_waypoints; i++)
			{
				out->push_back(waypoints->at(i));
			}
			return n_waypoints;
		}
		return 0;
	}

	// Private member functions:

	void NavaidDB::add_to_wpt_cache(waypoint wpt)
	{
		// Find the navaid in the database by name.
		if (is_wpt(wpt.id))
		{
			std::lock_guard<std::mutex> lock(wpt_db_mutex);
			// If there is a waypoint with the same name in the database,
			// add new entry to the vector.
			wpt_cache->at(wpt.id).push_back(wpt.data);
		}
		else
		{
			std::lock_guard<std::mutex> lock(wpt_db_mutex);
			// If there is no waypoint with the same name in the database,
			// add a vector with tmp
			std::pair<std::string, std::vector<waypoint_entry>> p;
			p = std::make_pair(wpt.id, std::vector<waypoint_entry>{wpt.data});
			wpt_cache->insert(p);
		}
	}

	void NavaidDB::add_to_navaid_cache(waypoint wpt, navaid_entry data)
	{
		// Find the navaid in the database by name.
		if (is_wpt(wpt.id))
		{
			// If there is a navaid with the same name in the database,
			// add new entry to the vector.
			bool is_colocated = false;
			bool is_duplicate = false;
			std::vector<waypoint_entry>* entries = &wpt_cache->at(wpt.id);
			for (int i = 0; i < entries->size(); i++)
			{
				if (entries->at(i).navaid)
				{
					waypoint_entry* tmp_wpt = &entries->at(i);
					navaid_entry* tmp_navaid = tmp_wpt->navaid;

					bool is_wpt_equal = !bool(memcmp(&tmp_wpt->pos, &wpt.data.pos, sizeof(geo::point)));
					bool is_type_equal = tmp_wpt->type == wpt.data.type;
					bool is_nav_equal = !bool(memcmp(tmp_navaid, &data, sizeof(navaid_entry)));
					bool is_equal = is_wpt_equal && is_nav_equal && is_type_equal;

					if (is_equal)
					{
						is_duplicate = true;
						break;
					}

					double lat_dev = abs(wpt.data.pos.lat_deg - tmp_wpt->pos.lat_deg);
					double lon_dev = abs(wpt.data.pos.lon_deg - tmp_wpt->pos.lon_deg);
					double ang_dev = lat_dev + lon_dev;
					int type_sum = wpt.data.type + tmp_wpt->type;
					int is_composite = 0;
					if (type_sum <= max_comp)
					{
						is_composite = comp_types[type_sum];
					}
					if (ang_dev < 0.001 && is_composite && data.freq == tmp_navaid->freq)
					{
						tmp_wpt->type = type_sum;
						is_colocated = true;
						break;
					}
				}
			}
			if (!is_colocated && !is_duplicate)
			{
				navaid_entry* ptr = new navaid_entry;
				memcpy(ptr, &data, sizeof(navaid_entry));

				wpt.data.navaid = ptr;

				std::lock_guard<std::mutex> lock(wpt_db_mutex);
				entries->push_back(wpt.data);
			}
		}
		else
		{
			// If there is no navaid with the same name in the database,
			// add a vector with tmp
			navaid_entry* ptr = new navaid_entry;
			memcpy(ptr, &data, sizeof(navaid_entry));

			wpt.data.navaid = ptr;

			add_to_wpt_cache(wpt);
		}
	}


	std::string navaid_to_str(int navaid_type)
	{
		switch (navaid_type)
		{
		case NAV_WAYPOINT:
			return "WPT";
		case NAV_NDB:
			return "NDB";
		case NAV_ILS_LOC_ONLY:
			return "ILS";
		case NAV_ILS_LOC:
			return "ILS";
		case NAV_ILS_GS:
			return "ILS";
		case NAV_ILS_FULL:
			return "ILS";
		case NAV_DME_ONLY:
			return "DME";
		case NAV_VOR_DME:
			return "VORDME";
		case NAV_ILS_DME:
			return "ILSDME";
		default:
			return "";
		}
	}

	void sort_wpt_entry_by_dist(std::vector<waypoint_entry>* vec, geo::point p)
	{
		WaypointEntryCompare comp;
		comp.ac_pos = p;

		sort(vec->begin(), vec->end(), comp);
	}

	void sort_wpts_by_dist(std::vector<waypoint>* vec, geo::point p)
	{
		WaypointCompare comp;
		comp.ac_pos = p;

		sort(vec->begin(), vec->end(), comp);
	}
};


namespace radnav_util
{
	/*
		This function calculates the quality ratio for a navaid.
		Navaids are sorted by this ratio to determine the best 
		suitable candidate(s) for radio navigation.
	*/

	void navaid_t::calc_qual(geo::point3d ac_pos)
	{
		if (data.navaid)
		{
			double lat_dist_nm = ac_pos.p.get_great_circle_distance_nm(data.pos);

			if (lat_dist_nm)
			{
				double v_dist_nm = abs(ac_pos.alt_ft - data.navaid->elevation) * FT_TO_NM;
				double slant_deg = atan(v_dist_nm / lat_dist_nm) * RAD_TO_DEG;

				if (slant_deg > 0 && slant_deg < VOR_MAX_SLANT_ANGLE_DEG)
				{
					double true_dist_nm = sqrt(lat_dist_nm * lat_dist_nm + v_dist_nm * v_dist_nm);

					double tmp = 1 - (true_dist_nm / data.navaid->max_recv);
					if (tmp >= 0)
					{
						qual = tmp;
						return;
					}
				}
			}
		}
		qual = -1;
	}

	/*
		This function calculates a quality value for a pair of navaids.
		This is useful when picking candidates for DME/DME position calculation.
	*/

	void navaid_pair_t::calc_qual(geo::point ac_pos)
	{
		if (n1 != nullptr && n2 != nullptr)
		{
			double b1 = n1->data.pos.get_great_circle_bearing_deg(ac_pos);
			double b2 = n2->data.pos.get_great_circle_bearing_deg(ac_pos);
			double phi = abs(b1 - b2);
			if (phi > 180)
				phi = 360 - phi;

			if (phi > DME_DME_PHI_MIN_DEG && phi < DME_DME_PHI_MAX_DEG)
			{
				double min_qual = n1->qual;
				if (n2->qual < min_qual)
				{
					min_qual = n2->qual;
				}

				qual = (min_qual + 1 - abs(90 - phi) / 90) / 2;
				return;
			}
		}
		qual = -1;
	}
}
