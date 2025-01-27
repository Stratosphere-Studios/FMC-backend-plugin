/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains declarations of member functions for AwyDB class. 
    AwyDB is an interface for earth_awy.dat.
*/


#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include "str_utils.hpp"
#include "navaid_db.hpp"


namespace libnav
{
    constexpr int N_AWY_COL_NORML = 11;
    constexpr char AWY_NAME_SEP = '-';
    constexpr char AWY_RESTR_FWD = 'F';
    constexpr char AWY_RESTR_BWD = 'B';
    constexpr char AWY_RESTR_NONE = 'N';


    struct alt_restr_t
    {
        uint32_t lower, upper;
    };

    struct awy_entry_t
    {
        std::string xp_type, reg_code;  // Region code of navaid/fix
    };

    struct awy_point_t
    {
        std::string id;
        awy_entry_t data;
        alt_restr_t alt_restr;

        awy_point_t(std::string nm="", std::string tp="", 
            std::string r_c="", uint32_t lower=0, uint32_t upper=0);

        /*
            Function: get_uid
            Description:
            forms uid of a waypoint using the following principle:
            uid=wpt_id+"_"+reg_code+"_"+libnav_type
        */

        std::string get_uid();
    };

    struct awy_line_t  // This is used to store the contents of 1 line of awy.dat
    {
        earth_data_line_t data;

        awy_point_t p1, p2;
        char path_restr;

        uint32_t lower_fl, upper_fl;
        std::string awy_names;


        awy_line_t(std::string& s);
    };

    struct awy_to_awy_data_t;


    typedef std::unordered_map<std::string, std::unordered_map<std::string, alt_restr_t>> graph_t;
    typedef std::unordered_map<std::string, graph_t> awy_db_t;
    typedef bool (*awy_path_func_t)(std::string&, void*);


    bool awy_wpt_to_wpt_func(std::string& curr, void* ref);

    bool awy_awy_to_awy_func(std::string& curr, void* ref);
    

    class AwyDB
    {
    public:

        AwyDB(std::string awy_path);

        DbErr get_err();

        int get_airac();

        int get_db_version();

        const awy_db_t& get_db();

        bool is_in_awy(std::string awy, std::string point);

        /*
            Fucntion: get_ww_path
            Description:
            Gets airway path from start waypoint to end waypoint
            @param awy: airway name
            @param start: start waypoint(airway id)
            @param end: end waypoint(airway id)
            @param out: pointer to output vector
            @return size of out
        */

        size_t get_ww_path(std::string awy, std::string start, 
            std::string end, std::vector<awy_point_t>* out);

        /*
            Fucntion: get_aa_path
            Description:
            Gets airway path that starts at start waypoint and ends at the intersection
            of awy and next_awy
            @param awy: airway that start waypoint belongs to
            @param start: start waypoint(airway id)
            @param next_awy: airway to intersect with
            @param out: pointer to output vector
            @return size of out
        */

        size_t get_aa_path(std::string awy, std::string start, 
            std::string next_awy, std::vector<awy_point_t>* out);

        /*
            Fucntion: get_path
            Description:
            Traverses the airway and returns path given a termination function.
            @param awy: airway to be traversed
            @param start: start waypoint(airway id)
            @param out: pointer to output vector
            @param path_func: termination function. Must return true when traversing
            has to be stopped
            @param ref: pointer to miscellaneous data passed to path_func
            @return size of out
        */

        size_t get_path(std::string awy, std::string start, 
            std::vector<awy_point_t>* out, awy_path_func_t path_func, void* ref);

        // You aren't supposed to call this function.
        // It's public to allow for the concurrent loading
        DbErr load_airways(std::string awy_path);

        ~AwyDB();

    private:
        int airac_cycle, db_version;
        awy_db_t awy_db;
        std::future<DbErr> db_loaded;

        void add_to_awy_db(awy_point_t p1, awy_point_t p2, std::string awy_nm, char restr);
    };


    struct awy_to_awy_data_t
    {
        std::string tgt_awy;
        AwyDB *db_ptr;
    };
}; // namespace libnav
