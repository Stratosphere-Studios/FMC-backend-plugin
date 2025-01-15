/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains declarations of member functions for Airport class and other 
    arinc424-related functions/structures. These allow you to work with arinc424 format
    and load procedures for any airport in x-plane's CIFP data base.
*/

#pragma once


#include <string>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <queue>
#include <unordered_map>
#include <set>
#include <mutex>
#include "arpt_db.hpp"
#include "navaid_db.hpp"
#include "common.hpp"
#include "str_utils.hpp"


namespace libnav
{
    enum class TurnDir
    {
        LEFT,
        RIGHT,
        EITHER
    };

    enum class AltMode
    {
        AT_OR_ABOVE,
        AT_OR_BELOW,
        AT,
        WITHIN,
        SID_AT_OR_ABOVE,  // At or above altitude specified in the 2nd altitude field
        GS_AT,  // Glide slope altitude in alt2, at altitude in alt1
        GS_AT_OR_ABOVE,  // Glide slope altitude in alt2, at or above altitude in alt1
        GS_INTC_AT,
        GS_INTC_AT_OR_ABOVE,
        ALT_STEPDOWN_AT_AT_OR_ABOVE,
        ALT_STEPDOWN_AT_AT,
        ALT_STEPDOWN_AT_AT_OR_BELOW
    };

    enum class SpeedMode
    {
        AT_OR_ABOVE,
        AT_OR_BELOW,
        AT
    };

    enum class ProcType
    {
        SID,
        STAR,
        APPROACH,
        PRDAT,
        RWY,
        NONE
    };

    enum class TCHType
    {
        ILS_MLS,
        RNAV,
        VGSI,
        DEFAULT,
        NONE
    };

    enum class LSCategory  // Category of the landing system as per arinc 424, section 5.80
    {
        ILS_LOC_ONLY,
        LS_CAT_I,
        LS_CAT_II,
        LS_CAT_III,
        IGS,
        LDA_GS,
        LDA_NO_GS,
        SDF_GS,
        SDF_NO_GS,
        NONE
    };


    constexpr char ARINC_FIELD_SEP = ',';
    // Number of columns in string containing SID/STAR/APPCH
    constexpr size_t N_ARINC_FLT_PROC_COL = 38;
    // Maximum number of characters for the data designator(comes before the :)
    constexpr size_t ARINC_MAX_TP_LENGTH = 5;
    // Number of columns in the first part of a runway entry
    constexpr int N_ARINC_RWY_COL_FIRST = 8;
    constexpr int N_ARINC_RWY_COL_SECOND = 3;

    constexpr int N_FLT_LEG_CACHE_SZ = 3000;
    // Approach prefixes
    typedef std::unordered_map<char, std::string> appr_pref_db_t;

    const appr_pref_db_t APPR_PREF = {
        {'B', "LOC"},
        {'G', "IGS"},
        {'J', "GLS"},
        {'M', "MLS"},
        {'W', "MLS"},
        {'Y', "MLS"},
        {'I', "ILS"},
        {'N', "NDB"},
        {'Q', "NDME"},
        {'R', "RNAV"},
        {'S', "VOR"},
        {'V', "VOR"},
        {'T', "TACAN"},
        {'U', "SDF"},
        {'X', "LDA"}
        };

    
    // Functions for decoding some arinc data fields:

    ProcType str2proc_type(std::string s);

    TurnDir char2dir(char c);  // Ref: arinc section 5.20

    float str2rnp(std::string s);  // Ref: arinc section 5.211

    float str2outbd_crs(std::string s, bool* is_true);  // Ref: arinc 5.26

    float str2outbd_dist(std::string s, bool* as_time);  // Ref: arinc 5.27

    AltMode char2alt_mode(char c);  // Ref: arinc 5.29

    int str2alt(std::string s);  // Ref: arinc 5.30

    SpeedMode char2spd_mode(char c);  // Ref: arinc 5.261

    TCHType char2tch_type(char c);  // Ref: arinc 5.270

    LSCategory char2ls_category(char c);  // Ref: arinc 5.80


    struct arinc_rwy_data_t
    {
        float grad_deg;  // Column 2. Runway gradient in degrees * 1000. Ref: arinc424 spec, section 5.212
        float ellips_height_m;  // Column 3. Ellipsoidal height in m * 10. Ref: arinc424 spec, section 5.225
        int thresh_elev_msl_ft;  // Column 4. Ref: arinc424 spec, section 5.68

        TCHType tch_tp;  // Column 5. Ref: arinc424 spec, section 5.270

        std::string ls_ident;  // Column 6. Identifyer of the landing system(4 chars). Ref: arinc424 spec, section 5.44
        LSCategory ls_cat;  // Column 7. Landing system category. Ref: arinc424 spec, section 5.80

        int tch_ft;  // Column 8. Threshold crossing height. Ref: arinc424 spec, section 5.67

        geo::point pos;  // Column 9-10. Position of the runway. Ref: arinc424 spec, section 5.36-37. NOTE: dms format.

        int thresh_displ_ft;  // Column 11. Ref: arinc424 spec, section 5.69.
    };

    struct arinc_rwy_full_t
    {
        DbErr err;

        std::string id;

        arinc_rwy_data_t data;


        int get_pos_from_db(std::string& area_code, 
        std::shared_ptr<ArptDB> arpt_db);

        void get_rwy_coords(std::string& s, std::string& area_code, 
            std::shared_ptr<ArptDB> arpt_db);

        arinc_rwy_full_t(std::string& s, std::string& area_code, 
            std::shared_ptr<ArptDB> arpt_db);
    };

    typedef std::unordered_map<std::string, arinc_rwy_data_t> arinc_rwy_db_t;


    struct arinc_fix_entry_t
    {
        std::string fix_ident;  //Ref: arinc424 spec, section 5.13/5.23/5.144/5.271
        std::string country_code;  //Ref: arinc424 spec, section 5.14
        char db_section;  //D - navaid, E - enroute, P - airport. Ref: arinc424 spec, section 5.4
        char db_subsection;  //Ref: arinc424 spec, section 5.5


        bool to_waypoint_t(std::string& area_code, 
            std::shared_ptr<ArptDB> arpt_db, std::shared_ptr<NavaidDB> navaid_db, 
            arinc_rwy_db_t& rwy_db, waypoint_t *out);
    };

    struct arinc_leg_t
    {
        char rt_type;  // Column 2. Ref: arinc424 spec, section 5.7

        bool has_main_fix;
        waypoint_t main_fix;  // Columns 5-8
        std::string wpt_desc;  //Column 9. Ref: arinc424 spec, section 5.17
        
        TurnDir turn_dir;
        double rnp;
        std::string leg_type;  // Column 12: Ref: arinc424 spec, section 5.21
        bool is_ovfy;

        bool has_recd_navaid;
        waypoint_t recd_navaid;  // Columns 14-17. Recommended navaid.

        double arc_radius;  // Column 18. Arc radius *1000 nm. Ref: arinc424 spec, section 5.204
        double theta;  // Column 19. magnetic bearing from recd_navaid to main_fix *10. Ref: arinc424 spec, section 5.24
        double tru_theta; // theta in true degrees
        double rho;  // Column 20. geodetic distance from recd_navaid to main_fix *10. Ref: arinc424 spec, section 5.25
        bool outbd_crs_true;
        double outbd_crs_deg;  // Treated as magnetic if outbd_crs_true is false.
        bool outbd_dist_as_time;
        double outbd_dist_time;
        
        AltMode alt_desc;  // Column 23
        int alt1_ft;
        int alt2_ft;
        int trans_alt;  // Column 26. In feet MSL. Ref: arinc424 spec, section 5.53

        SpeedMode speed_desc;  // Column 27. Ref: arinc424 spec, section 5.261
        int spd_lim_kias;  // Column 28. Speed limit in KIAS. Ref: arinc424 spec, section 5.28
        double vert_angle_deg; 
        int vert_scale_ft;  // Column 30. Vertical deviation scale in feet. Ref: arinc424 spec, section 5.293

        bool has_center_fix;
        waypoint_t center_fix;  // Column 31-34. Center fix for RF/AF leg.

        char multi_cod;  // Column 35 Multi-Code / TAA Center. Ref: arinc424 spec, section 5.130/5.272

        char gnss_ind;  // Column 36. Ref: arinc424 spec, section 5.222

        char rt_qual1;  // Column 37. Ref: arinc424 spec, section 5.7
        char rt_qual2;  // Column 38. Ref: arinc424 spec, section 5.7


        // Gets mag var from the recommended navaid
        double get_mag_var_deg();

        // Use the following functions if you ever need to set any leg's fix manually.
        // NEVER set the leg fixes manually.

        void set_main_fix(waypoint_t fix);

        void set_recd_fix(waypoint_t fix);

        void set_ctr_fix(waypoint_t fix);
    };

    struct arinc_str_t
    {
        char rt_type;  // Column 2. Ref: arinc424 spec, section 5.7
        std::string proc_name;  // Column 3. Ref: arinc424 spec, section 5.9 & 5.10
        std::string trans_name;  // Column 4. Ref: arinc424 spec, section 5.11

        arinc_fix_entry_t main_fix;  // Columns 5-8
        std::string wpt_desc;  //Column 9. Ref: arinc424 spec, section 5.17
        
        char turn_dir;  // Column 10: L/R/E. Ref: arinc424 spec, section 5.20
        std::string rnp;  // Column 11: 2 digits for number, 3rd digit for negative exponent. Ref: arinc424 spec, section 5.211
        std::string leg_type;  // Column 12: Ref: arinc424 spec, section 5.21
        char tdv;  // Column 13: Y if waypoint is "overfly". Ref: arinc424 spec, section 5.22

        arinc_fix_entry_t recd_navaid;  // Columns 14-17. Recommended navaid.

        double arc_radius;  // Column 18. Arc radius *1000 nm. Ref: arinc424 spec, section 5.204
        double theta;  // Column 19. magnetic bearing from recd_navaid to main_fix *10. Ref: arinc424 spec, section 5.24
        double rho;  // Column 20. geodetic distance from recd_navaid to main_fix *10. Ref: arinc424 spec, section 5.25
        std::string outbd_mag_crs;  // Column 21. Magnetic course from main_fix *10. If true, has T at the end. 
            // Ref: arinc424 spec, section 5.26
        std::string outbd_dist_time;  // Column 22. Distance in nm/time(min) from main_fix to next wpt. *10. 
            // T at the beginning if time. Ref: arinc424 spec, section 5.27
        
        char alt_desc;  // Column 23. Ref: arinc424 spec, section 5.29
        std::string alt1;  // Column 24. In feet or flight level. Ref: arinc424 spec, section 5.30
        std::string alt2;  // Column 25. In feet or flight level. Ref: arinc424 spec, section 5.30
        int trans_alt;  // Column 26. In feet MSL. Ref: arinc424 spec, section 5.53

        char speed_desc;  // Column 27. Ref: arinc424 spec, section 5.261
        int spd_lim;  // Column 28. Speed limit in KIAS. Ref: arinc424 spec, section 5.28
        double vert_angle;  // Column 29. Vertical angle in deg*100. Ref: arinc424 spec, section 5.70
        int vert_scale;  // Column 30. Vertical deviation scale in feet. Ref: arinc424 spec, section 5.293

        arinc_fix_entry_t center_fix;  // Column 31-34. Center fix for RF/AF leg.

        char multi_cod;  // Column 35 Multi-Code / TAA Center. Ref: arinc424 spec, section 5.130/5.272

        char gnss_ind;  // Column 36. Ref: arinc424 spec, section 5.222

        char rt_qual1;  // Column 37. Ref: arinc424 spec, section 5.7
        char rt_qual2;  // Column 38. Ref: arinc424 spec, section 5.7


        arinc_str_t(std::vector<std::string>& in_split);

        arinc_leg_t get_leg(std::string& area_code, std::shared_ptr<ArptDB> arpt_db,
            std::shared_ptr<NavaidDB> navaid_db, arinc_rwy_db_t& rwy_db);
    };


    typedef std::vector<arinc_leg_t> arinc_leg_seq_t;

    /*
        Function: get_rnw_wpt
        Description:
        Checks if id is a valid runway and constructs a waypoint_t from such runway if it's valid.
        @param rwy_db: reference to a runway data base
        @param id: id of the target runway. Can have "RW" prefix, but not necessary.
        @param area_cd: area code.
        @param country_cd: country code.
        @param out: location where to write the waypoint with the position at 
        the start of the selected runway.
        @return true if waypoint was written, otherwise false.
    */
    bool get_rnw_wpt(arinc_rwy_db_t& rwy_db, std::string& id, std::string& area_cd, 
        std::string& country_cd, waypoint_t *out);

    std::vector<std::string> get_all_rwys_by_mask(std::string mask, 
        arinc_rwy_db_t& rwy_db);

    std::string get_full_appr_nm(std::string nm, appr_pref_db_t& pref_db);

	
	typedef std::set<std::string> str_set_t;
    typedef std::unordered_map<std::string, str_set_t> str_umap_t;

    class Airport
    {
        typedef std::unordered_map<std::string, std::vector<int>> trans_db_t;
        typedef std::unordered_map<std::string, trans_db_t> proc_db_t;
        typedef std::pair<std::string, ProcType> proc_typed_str_t;
        

    public:
        DbErr err_code;

        std::string icao_code;


        Airport(std::string icao, std::shared_ptr<ArptDB> arpt_db, 
            std::shared_ptr<NavaidDB> navaid_db, std::string cifp_path="", 
            std::string postfix=".dat", bool use_pr=false, appr_pref_db_t pr_db = APPR_PREF);

        Airport(Airport& copy);

        std::vector<std::string> get_rwys();

        const arinc_rwy_db_t& get_rwy_db();

        str_umap_t get_all_sids();

        str_umap_t get_all_stars();

        str_umap_t get_all_appch();

        arinc_leg_seq_t get_sid(std::string& proc_name, std::string& trans);

        arinc_leg_seq_t get_star(std::string& proc_name, std::string& trans);

        arinc_leg_seq_t get_appch(std::string& proc_name, std::string& trans);

        str_set_t get_sid_by_rwy(std::string& rwy_id);

        str_set_t get_star_by_rwy(std::string& rwy_id);

        str_set_t get_rwy_by_sid(std::string& sid);

        str_set_t get_rwy_by_star(std::string& star);

        str_set_t get_trans_by_sid(std::string& sid);

        str_set_t get_trans_by_star(std::string& star);

        str_set_t get_trans_by_appch(std::string& appch);

        ~Airport();

    private:
        bool use_appch_prefix;
        appr_pref_db_t appch_prefix_db;
        arinc_rwy_db_t rwy_db;
        arinc_leg_t* arinc_legs;
        int n_arinc_legs_used;

        //std::mutex sid_mutex;
        //std::mutex star_mutex;
        //std::mutex appch_mutex;

        /*
            Storage of procedures:
            The procedures are stored in the following way:
            [proc_name][trans][legs]
        */

        proc_db_t sid_db;
        proc_db_t star_db;
        proc_db_t appch_db;

        str_umap_t sid_per_rwy;
        str_umap_t star_per_rwy;

        std::queue<proc_typed_str_t> flt_leg_strings;


        str_umap_t get_all_proc(proc_db_t& db);

        arinc_leg_seq_t get_proc(std::string& proc_name, std::string& trans, 
            proc_db_t& db);

        str_set_t get_proc_by_rwy(std::string& rwy_id, 
            str_umap_t& umap);

        str_set_t get_trans_by_proc(std::string& proc_name, 
            proc_db_t db, bool rwy=false);
			
		/*
            Function: parse_flt_legs
            Description:
            Parses flight legs. It takes them from flt_leg_strings. The function populates
            SID/STAR/Approach and runway data bases.
            @param arpt_db: pointer to airport data base
            @param navaid_db: pointer to navaid data base
            @return: error code. Can be either of the following: 
                DbErr::SUCCESS
                DbErr::PARTIAL_LOAD
                DbErr::BAD_ALLOC
        */

        DbErr parse_flt_legs(std::shared_ptr<ArptDB> arpt_db, 
            std::shared_ptr<NavaidDB> navaid_db);

		/*
            Function: load_db
            Description:
            Loads a CIFP data base
            @param arpt_db: pointer to airport data base
            @param navaid_db: pointer to navaid data base
            @param path: path to directory of target data base file
            @param postfix: file extension of target data base e.g. .dat
            @return: error code. Can be either of the following: 
                DbErr::SUCCESS
                DbErr::DATA_BASE_ERROR
                DbErr::FILE_NOT_FOUND
                DbErr::PARTIAL_LOAD
                DbErr::BAD_ALLOC
        */

        DbErr load_db(std::shared_ptr<ArptDB> arpt_db, 
            std::shared_ptr<NavaidDB> navaid_db, std::string& path, 
            std::string& postfix);
    };
}; // namespace libnav
