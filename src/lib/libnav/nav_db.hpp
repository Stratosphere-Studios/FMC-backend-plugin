/*
	This project is licensed under 
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512
*/

#pragma once

#include "arpt_db.hpp"
#include "navaid_db.hpp"
#include "common.hpp"


enum POI_types
{
	POI_NULL = 0,
	POI_WAYPOINT = 2,
	POI_NAVAID = 3,
	POI_AIRPORT = 5,
	POI_RWY = 7
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
		NavDB(std::shared_ptr<libnav::ArptDB> arpt_ptr, std::shared_ptr<libnav::NavaidDB> navaid_ptr);

		bool is_loaded();

		//These member functions are just wrappers around ArptDB member functions.

		int get_airport_data(std::string icao_code, airport_data* out);

		int get_apt_rwys(std::string icao_code, runway_data* out);

		int get_rnw_data(std::string apt_icao, std::string rnw_id, runway_entry* out);

		//These member functions are just wrappers around NavaidDB member functions.

		int get_wpt_data(std::string id, std::vector<waypoint_entry>* out);

		bool is_navaid_of_type(std::string id, std::vector<int> types);

	private:
		std::shared_ptr<libnav::ArptDB> arpt_db;
		std::shared_ptr<libnav::NavaidDB> navaid_db;
	};
}
