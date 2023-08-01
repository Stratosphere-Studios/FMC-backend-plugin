#include "dataref_structs.hpp"
#include <vector>


enum FMS_constants
{
	N_MAX_DATABUS_QUEUE_PROC = 1024,
	N_CUSTOM_STR_DR_LENGTH = 2048,
	REF_NAV_ICAO_BUF_LENGTH = 5,
	FMC_SCREEN_LINE_LENGTH = 24,
	N_REF_NAV_MAG_VAR_BUF_LENGTH = 4,
	N_REF_NAV_NAVAID_BUF_LENGTH = 4, 
	N_RTE_ICAO_BUF_LENGTH = 4,
	N_RTE_RWY_BUF_LENGTH = 5
};

std::vector<int> fmc_l_int_dr = { -1, 1 };
std::vector<int> fmc_r_int_dr = { -1, 1 };


std::vector<DRUtil::dref_i> int_datarefs = {
	{{"Strato/777/UI/messages/creating_databases", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/REF_NAV/rad_nav_inh", DR_WRITABLE, false, nullptr}, nullptr},

	// FMC L data refs:



	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/clear_msg", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/page", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/wpt_idx", DR_WRITABLE, false, nullptr}, &fmc_r_int_dr[0]},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_type", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/subpage", DR_WRITABLE, false, nullptr}, &fmc_r_int_dr[1]},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/n_subpages", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/is_active", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/n_pois_disp", DR_READONLY, false, nullptr}, nullptr},

	{{"Strato/777/FMC/FMC_R/scratchpad/not_in_database", DR_READONLY, false, nullptr}, nullptr}
};

std::vector<DRUtil::dref_d> double_datarefs = {
	// FMC L data refs:



	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lat", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lon", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_elev", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_freq", DR_READONLY, false, nullptr}, nullptr}
};

std::vector<DRUtil::dref_ia> int_arr_datarefs = {

};

std::vector<DRUtil::dref_fa> float_arr_datarefs = {
	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi_list", DR_READONLY, false, nullptr}, nullptr, 3 * N_CDU_OUT_LINES},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi_list", DR_READONLY, false, nullptr}, nullptr, 3 * N_CDU_OUT_LINES}
};

std::vector<DRUtil::dref_s> str_datarefs = {
	{{"Strato/777/FMC/REF_NAV/navaid_1_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/navaid_2_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/vor_1_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/vor_2_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},

	{{"Strato/777/FMC/RTE1/dep_icao_out", DR_READONLY, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/RTE1/arr_icao_out", DR_READONLY, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/RTE1/dep_rnw_out", DR_READONLY, false, nullptr}, nullptr, N_RTE_RWY_BUF_LENGTH},

	// FMC L data refs:



	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/REF_NAV/input_icao", DR_WRITABLE, false, nullptr}, nullptr, REF_NAV_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/out_icao", DR_READONLY, false, nullptr}, nullptr, REF_NAV_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_mag_var", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_MAG_VAR_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/navaid_1_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/navaid_2_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/vor_1_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/vor_2_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},

	{{"Strato/777/FMC/FMC_R/RTE1/dep_icao_in", DR_WRITABLE, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/RTE1/arr_icao_in", DR_WRITABLE, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/RTE1/dep_rnw_in", DR_WRITABLE, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},

	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi1_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi2_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi3_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi4_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi5_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi6_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/scratchpad_msg", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH}
};

StratosphereAvionics::avionics_out_drs av_out = {
											"Strato/777/FMC/RTE1/dep_icao_out",
											"Strato/777/FMC/RTE1/arr_icao_out",
											"Strato/777/FMC/RTE1/dep_rnw_out",
											{"Strato/777/FMC/REF_NAV/navaid_1_out",
											 "Strato/777/FMC/REF_NAV/navaid_2_out"},
											{"Strato/777/FMC/REF_NAV/vor_1_out",
											 "Strato/777/FMC/REF_NAV/vor_2_out"}
};

StratosphereAvionics::fmc_in_drs fmc_r_in = {
											"sim/flightmodel/position/latitude",
											"sim/flightmodel/position/longitude",

										   {"Strato/777/FMC/FMC_R/REF_NAV/input_icao",
											"Strato/777/FMC/REF_NAV/rad_nav_inh",
											{"Strato/777/FMC/FMC_R/REF_NAV/navaid_1_in",
											 "Strato/777/FMC/FMC_R/REF_NAV/navaid_2_in"},
											{"Strato/777/FMC/FMC_R/REF_NAV/vor_1_in",
											 "Strato/777/FMC/FMC_R/REF_NAV/vor_2_in"}},
										   {"Strato/777/FMC/FMC_R/RTE1/dep_icao_in",
											"Strato/777/FMC/FMC_R/RTE1/arr_icao_in",
											"Strato/777/FMC/FMC_R/RTE1/dep_rnw_in"},
										   {"Strato/777/FMC/FMC_R/SEL_WPT/subpage",
											"Strato/777/FMC/FMC_R/SEL_WPT/wpt_idx"},

											"Strato/777/FMC/FMC_R/clear_msg",
											"Strato/777/FMC/FMC_R/page"
};

StratosphereAvionics::fmc_out_drs fmc_r_out = {
											 {"Strato/777/FMC/FMC_R/REF_NAV/out_icao",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_type",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_lat",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_lon",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_elev",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_freq",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_mag_var"},

											 {"Strato/777/FMC/FMC_R/SEL_WPT/is_active",
											  "Strato/777/FMC/FMC_R/SEL_WPT/n_subpages",
											  "Strato/777/FMC/FMC_R/SEL_WPT/n_pois_disp",
											  "Strato/777/FMC/FMC_R/SEL_WPT/poi_list",
											  {"Strato/777/FMC/FMC_R/SEL_WPT/poi1_type",
											   "Strato/777/FMC/FMC_R/SEL_WPT/poi2_type",
											   "Strato/777/FMC/FMC_R/SEL_WPT/poi3_type",
											   "Strato/777/FMC/FMC_R/SEL_WPT/poi4_type",
											   "Strato/777/FMC/FMC_R/SEL_WPT/poi5_type",
											   "Strato/777/FMC/FMC_R/SEL_WPT/poi6_type"}},

											 {0, {"Strato/777/FMC/FMC_L/scratchpad/not_in_database"}}
};
