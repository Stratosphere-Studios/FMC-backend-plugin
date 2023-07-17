#include "navaid_db.hpp"

namespace libnav
{
	bool WaypointCompare::operator()(waypoint w1, waypoint w2)
	{
		double d1 = w1.pos.getGreatCircleDistanceNM(ac_pos);
		double d2 = w2.pos.getGreatCircleDistanceNM(ac_pos);
		return d1 < d2;
	}

	bool NavaidCompare::operator()(navaid n1, navaid n2)
	{
		double d1 = n1.data.pos.getGreatCircleDistanceNM(ac_pos);
		double d2 = n2.data.pos.getGreatCircleDistanceNM(ac_pos);
		return d1 < d2;
	}

	NavaidDB::NavaidDB(std::string wpt_path, std::string navaid_path,
		std::unordered_map<std::string, std::vector<geo::point>>* wpt_db,
		std::unordered_map<std::string, std::vector<navaid_entry>>* navaid_db)
	{
		// Pre-defined stuff

		comp_types[NAV_ILS_FULL] = 1;
		comp_types[NAV_VOR_DME] = 1;
		comp_types[NAV_ILS_DME] = 1;

		// Paths

		sim_wpt_db_path = wpt_path;
		sim_navaid_db_path = navaid_path;

		wpt_cache = wpt_db;
		navaid_cache = navaid_db;

		wpt_loaded = std::async(std::launch::async, [](NavaidDB* db) -> int {return db->load_waypoints(); }, this);
		navaid_loaded = std::async(std::launch::async, [](NavaidDB* db) -> int {return db->load_navaids(); }, this);
	}

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
					// Construct a navaid entry.
					std::stringstream s(line);
					double lat, lon;
					std::string name;
					std::string junk;
					geo::point tmp;
					s >> lat >> lon >> name >> junk;
					tmp.lat_deg = lat;
					tmp.lon_deg = lon;
					// Find the navaid in the database by name.
					if (wpt_cache->find(name) != wpt_cache->end())
					{
						// If there is a navaid with the same name in the database,
						// add new entry to the vector.
						wpt_cache->at(name).push_back(tmp);
					}
					else
					{
						// If there is no navaid with the same name in the database,
						// add a vector with tmp
						std::pair<std::string, std::vector<geo::point>> p;
						p = std::make_pair(name, std::vector<geo::point>{tmp});
						wpt_cache->insert(p);
					}
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
					std::string name;
					std::string junk;
					navaid_entry tmp;
					s >> type >> lat >> lon >> elevation >> freq >> max_recv >> mag_var >> name >> junk;
					tmp.type = type;
					tmp.max_recv = max_recv;
					tmp.pos.lat_deg = lat;
					tmp.pos.lon_deg = lon;
					tmp.elevation = elevation;
					tmp.mag_var = mag_var;
					tmp.freq = freq;
					// Find the navaid in the database by name.
					if (navaid_cache->find(name) != navaid_cache->end())
					{
						// If there is a navaid with the same name in the database,
						// add new entry to the vector.
						bool is_colocated = false;
						bool is_duplicate = false;
						std::vector<navaid_entry>* entries = &navaid_cache->at(name);
						for (int i = 0; i < entries->size(); i++)
						{
							bool is_equal = !bool(memcmp(&entries->at(i), &tmp, sizeof(navaid_entry)));
							if (is_equal)
							{
								is_duplicate = true;
								break;
							}
							navaid_entry* navaid = &entries->at(i);
							double ang_dev = abs(navaid->pos.lat_deg - tmp.pos.lat_deg) + abs(navaid->pos.lon_deg - tmp.pos.lon_deg);
							int type_sum = tmp.type + navaid->type;
							int is_composite = 0;
							if (type_sum <= max_comp)
							{
								is_composite = comp_types[type_sum];
							}
							if (ang_dev < 0.001 && is_composite && tmp.freq == navaid->freq)
							{
								navaid->type = type_sum;
								is_colocated = true;
								break;
							}
						}
						if (!is_colocated && !is_duplicate)
						{
							entries->push_back(tmp);
						}
					}
					else
					{
						// If there is no navaid with the same name in the database,
						// add a vector with tmp
						std::pair<std::string, std::vector<navaid_entry>> p;
						p = std::make_pair(name, std::vector<navaid_entry>{tmp});
						navaid_cache->insert(p);
					}
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

	size_t NavaidDB::get_wpt_info(std::string id, std::vector<geo::point>* out)
	{
		if (is_wpt(id))
		{
			std::lock_guard<std::mutex> lock(wpt_db_mutex);
			std::vector<geo::point>* waypoints = &wpt_cache->at(id);
			size_t n_waypoints = waypoints->size();
			for (int i = 0; i < n_waypoints; i++)
			{
				out->push_back(waypoints->at(i));
			}
			return n_waypoints;
		}
		return 0;
	}

	bool NavaidDB::is_navaid(std::string id)
	{
		std::lock_guard<std::mutex> lock(navaid_db_mutex);
		return navaid_cache->find(id) != navaid_cache->end();
	}

	size_t NavaidDB::get_navaid_info(std::string id, std::vector<navaid_entry>* out)
	{
		if (is_navaid(id))
		{
			std::lock_guard<std::mutex> lock(navaid_db_mutex);
			std::vector<navaid_entry>* navaids = &navaid_cache->at(id);
			size_t n_navaids = navaids->size();
			for (size_t i = 0; i < n_navaids; i++)
			{
				out->push_back(navaids->at(i));
			}
			return n_navaids;
		}
		return 0;
	}


	std::string navaid_to_str(int navaid_type)
	{
		switch (navaid_type)
		{
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

	void sort_wpts_by_dist(std::vector<waypoint>* vec, geo::point p)
	{
		WaypointCompare comp;
		comp.ac_pos = p;

		sort(vec->begin(), vec->end(), comp);
	}

	void sort_navaids_by_dist(std::vector<navaid>* vec, geo::point p)
	{
		NavaidCompare comp;
		comp.ac_pos = p;

		sort(vec->begin(), vec->end(), comp);
	}
}
