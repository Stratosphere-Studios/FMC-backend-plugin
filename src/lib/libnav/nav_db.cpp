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

	size_t NavDB::get_wpt_data(std::string id, std::vector<waypoint_entry>* out)
	{
		return navaid_db->get_wpt_data(id, out);
	}

	bool NavDB::is_navaid_of_type(std::string id, std::vector<int> types)
	{
		return navaid_db->is_navaid_of_type(id, types);
	}
}
