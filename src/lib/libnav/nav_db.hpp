#pragma once

#include "arpt_db.hpp"
#include "navaid_db.hpp"


enum POI_types
{
	POI_NULL = 0,
	POI_WAYPOINT = 1,
	POI_NAVAID = 2,
	POI_AIRPORT = 3
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

		int get_poi_type(std::string id);

		//These member functions are just wrappers around ArptDB member functions.

		size_t get_airport_data(std::string icao_code, airport_data* out);

		size_t get_runway_data(std::string icao_code, runway_data* out);

		//These member functions are just wrappers around NavaidDB member functions.

		size_t get_wpt_info(std::string id, std::vector<geo::point>* out);

		size_t get_navaid_info(std::string id, std::vector<navaid_entry>* out);

		size_t get_poi_info(std::string id, POI* out);

	private:
		ArptDB* arpt_db;
		NavaidDB* navaid_db;
	};
}
