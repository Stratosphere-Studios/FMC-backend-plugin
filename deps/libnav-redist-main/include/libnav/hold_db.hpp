/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains function declarations for the HoldDB class. This class serves
    as an interface for x-plane's earth_hold.dat
*/


#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <future>
#include "str_utils.hpp"
#include "common.hpp"


namespace libnav
{
    enum class HoldTurnDir
    {
        LEFT,
        RIGHT
    };

    constexpr int N_HOLD_COL_NORML = 11;


    struct hold_data_t
    {
        float inbd_crs_mag, leg_time_min, dme_leg_dist_nm;

        HoldTurnDir turn_dir;
        
        int min_alt_ft, max_alt_ft, spd_kts;
    };

    struct hold_line_t
    {
        earth_data_line_t data;

        std::string uid;
        hold_data_t hold_data;


        hold_line_t(std::string& s);
    };


    typedef std::unordered_map<std::string, std::vector<hold_data_t>> hold_db_t;


    class HoldDB
    {
    public:
        HoldDB(std::string db_path);

        DbErr get_err();

        int get_airac();

        int get_db_version();

        const hold_db_t& get_db();

        bool has_hold(std::string& wpt_id);

        std::vector<hold_data_t> get_hold_data(std::string& wpt_id);

        // You don't need to call this one.
        // It's called by the corresponding thread that is created in the constructor.
        DbErr load_holds(std::string& db_path);

    private:
        int airac_cycle, db_version;
        hold_db_t hold_db;

        std::future<DbErr> hold_load_task;
    };
};
