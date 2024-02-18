/*
	Author: discord/bruh4096#4512
*/

#include "nav_db.hpp"


namespace libnav
{
	NavDB::NavDB(std::shared_ptr<libnav::ArptDB> arpt_ptr, std::shared_ptr<libnav::NavaidDB> navaid_ptr)
	{
		navaid_db = navaid_ptr;
		arpt_db = arpt_ptr;
	}

	bool NavDB::is_loaded()
	{
		return arpt_db->is_loaded() && navaid_db->is_loaded();
	}

	// Wrappers around ArptDB member functions.

	int NavDB::get_airport_data(std::string icao_code, airport_data* out)
	{
		return arpt_db->get_airport_data(icao_code, out);
	}

	int NavDB::get_apt_rwys(std::string icao_code, runway_data* out)
	{
		return arpt_db->get_apt_rwys(icao_code, out);
	}

	int NavDB::get_rnw_data(std::string apt_icao, std::string rnw_id, runway_entry* out)
	{
		return arpt_db->get_rnw_data(apt_icao, rnw_id, out);
	}

	// Wrappers around NavaidDB member functions.

	int NavDB::get_wpt_data(std::string id, std::vector<waypoint_entry>* out)
	{
		return navaid_db->get_wpt_data(id, out);
	}

	bool NavDB::is_navaid_of_type(std::string id, std::vector<int> types)
	{
		return navaid_db->is_navaid_of_type(id, types);
	}
}
