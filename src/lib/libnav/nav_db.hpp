#pragma once

#include "arpt_db.hpp"
#include "navaid_db.hpp"


enum POI_types
{
	POI_NULL = 0,
	POI_WAYPOINT = 2,
	POI_NAVAID = 3,
	POI_AIRPORT = 5
};


namespace libnav
{
	struct POI
	{
		std::string id;
		std::vector<geo::point> wpt;
		std::vector<navaid_entry> navaid;
		airport_entry arpt;
		uint8_t type;
	};


	/*
		NavDB provides is unified interface for all nav data bases.
	*/

	class NavDB
	{
	public:
		NavDB(ArptDB* arpt_ptr, NavaidDB* navaid_ptr);

		bool is_loaded();

		//These member functions are just wrappers around ArptDB member functions.

		size_t get_airport_data(std::string icao_code, airport_data* out);

		size_t get_runway_data(std::string icao_code, runway_data* out);

		//These member functions are just wrappers around NavaidDB member functions.

		size_t get_wpt_data(std::string id, std::vector<waypoint_entry>* out);

	private:
		ArptDB* arpt_db;
		NavaidDB* navaid_db;
	};
}
