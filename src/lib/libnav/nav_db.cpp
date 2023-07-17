#include "nav_db.hpp"


namespace libnav
{
	NavDB::NavDB(ArptDB* arpt_ptr, NavaidDB* navaid_ptr)
	{
		navaid_db = navaid_ptr;
		arpt_db = arpt_ptr;
	}

	bool NavDB::is_loaded()
	{
		return arpt_db->is_loaded() && navaid_db->is_loaded();
	}

	int NavDB::get_poi_type(std::string id)
	{
		if (arpt_db->is_airport(id))
			return POI_AIRPORT;
		else if (navaid_db->is_navaid(id))
			return POI_NAVAID;
		else if (navaid_db->is_wpt(id))
			return POI_WAYPOINT;
		return POI_NULL;
	}

	// Wrappers around ArptDB member functions.

	size_t NavDB::get_airport_data(std::string icao_code, airport_data* out)
	{
		return arpt_db->get_airport_data(icao_code, out);
	}

	size_t NavDB::get_runway_data(std::string icao_code, runway_data* out)
	{
		return arpt_db->get_runway_data(icao_code, out);
	}

	// Wrappers around NavaidDB member functions.

	size_t NavDB::get_wpt_info(std::string id, std::vector<geo::point>* out)
	{
		return navaid_db->get_wpt_info(id, out);
	}

	size_t NavDB::get_navaid_info(std::string id, std::vector<navaid_entry>* out)
	{
		return navaid_db->get_navaid_info(id, out);
	}

	// Deprecated

	size_t NavDB::get_poi_info(std::string id, POI* out)
	{
		size_t n_airports = arpt_db->get_airport_data(id, &out->arpt.data);
		if (n_airports)
		{
			out->id = id;
			out->type = POI_AIRPORT;
			return n_airports;
		}
		else
		{
			size_t n_navaids = navaid_db->get_navaid_info(id, &out->navaid);
			if (n_navaids)
			{
				out->id = id;
				out->type = POI_NAVAID;
				return n_navaids;
			}
			else
			{
				size_t n_waypoints = navaid_db->get_wpt_info(id, &out->wpt);
				if (n_waypoints)
				{
					out->id = id;
					out->type = POI_WAYPOINT;
					return n_waypoints;
				}
			}
		}
		return 0;
	}
}
