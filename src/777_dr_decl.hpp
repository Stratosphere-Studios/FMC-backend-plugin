#include "dataref_structs.hpp"
#include <vector>


enum FMS_constants
{
	N_MAX_DATABUS_QUEUE_PROC = 64,
	N_CUSTOM_STR_DR_LENGTH = 2048,
	REF_NAV_ICAO_BUF_LENGTH = 5,
	FMC_SCREEN_LINE_LENGTH = 24,
	N_REF_NAV_MAG_VAR_BUF_LENGTH = 4,
	N_REF_NAV_NAVAID_BUF_LENGTH = 4, 
	N_RTE_ICAO_BUF_LENGTH = 4,
	N_RTE_RWY_BUF_LENGTH = 5,

	DEBUG_DR_LENGTH = 32
};

constexpr int DEFAULT_WPT_IDX = -1;
constexpr int DEFAULT_WPT_SUBPAGE = 1;


std::vector<DRUtil::dref_i> int_datarefs = {
	// Autopilot control data refs:
	{{"Strato/777/mcp/ap_on", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/fma/active_vert_mode", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/fma/active_roll_mode", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/fma/alt_acq", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/fma/at_mode", DR_WRITABLE, false, nullptr}, 0},

	{{"Strato/777/UI/messages/creating_databases", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/REF_NAV/rad_nav_inh", DR_WRITABLE, false, nullptr}, 0},

	// FMC L data refs:

	{{"Strato/777/FMC/FMC_L/clear_msg", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/page", DR_WRITABLE, false, nullptr}, 0},

	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_elev", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_length_ft", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_length_m", DR_READONLY, false, nullptr}, 0},

	{{"Strato/777/FMC/FMC_L/SEL_WPT/wpt_idx", DR_WRITABLE, false, nullptr}, DEFAULT_WPT_IDX},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_type", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/subpage", DR_WRITABLE, false, nullptr}, DEFAULT_WPT_SUBPAGE},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/n_subpages", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/is_active", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/n_pois_disp", DR_READONLY, false, nullptr}, 0},

	{{"Strato/777/FMC/FMC_L/scratchpad/not_in_database", DR_READONLY, false, nullptr}, 0},

	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/clear_msg", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/page", DR_WRITABLE, false, nullptr}, 0},

	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_elev", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_length_ft", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_length_m", DR_READONLY, false, nullptr}, 0},

	{{"Strato/777/FMC/FMC_R/SEL_WPT/wpt_idx", DR_WRITABLE, false, nullptr}, DEFAULT_WPT_IDX},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_type", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/subpage", DR_WRITABLE, false, nullptr}, DEFAULT_WPT_SUBPAGE},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/n_subpages", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/is_active", DR_WRITABLE, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/n_pois_disp", DR_READONLY, false, nullptr}, 0},

	{{"Strato/777/FMC/FMC_R/scratchpad/not_in_database", DR_READONLY, false, nullptr}, 0}
};

std::vector<DRUtil::dref_d> double_datarefs = {
	// Autopilot data refs:
	{{"Strato/777/autopilot/gain_tst", DR_WRITABLE, false, nullptr}, 0.9},
	{{"Strato/777/autopilot/yoke_cmd_roll", DR_WRITABLE, false, nullptr}, 0},

	{{"Strato/777/FMC/RAD_NAV/VOR_DME/pos_lat", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/RAD_NAV/VOR_DME/pos_lon", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/RAD_NAV/VOR_DME/pos_fom", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/RAD_NAV/DME_DME/pos_lat", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/RAD_NAV/DME_DME/pos_lon", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/RAD_NAV/DME_DME/pos_fom", DR_READONLY, false, nullptr}, 0},

	// FMC L data refs:

	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_lat", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_lon", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_freq", DR_READONLY, false, nullptr}, 0},

	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lat", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lon", DR_READONLY, false, nullptr}, 0},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_freq", DR_READONLY, false, nullptr}, 0}
};

std::vector<DRUtil::dref_ia> int_arr_datarefs = {

};

std::vector<DRUtil::dref_fa> float_arr_datarefs = {
	{{"Strato/777/autopilot/yoke_cmd", DR_WRITABLE, false, nullptr}, 
		nullptr, 2},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi_list", DR_READONLY, false, nullptr}, 
		nullptr, 3 * StratosphereAvionics::N_CDU_OUT_LINES},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi_list", DR_READONLY, false, nullptr}, 
		nullptr, 3 * StratosphereAvionics::N_CDU_OUT_LINES}
};

std::vector<DRUtil::dref_s> str_datarefs = {
	{{"Strato/777/FMC/REF_NAV/navaid_1_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/navaid_2_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/vor_1_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/vor_2_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},

	{{"Strato/777/FMC/RTE1/dep_icao_out", DR_READONLY, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/RTE1/arr_icao_out", DR_READONLY, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/RTE1/dep_rnw_out", DR_READONLY, false, nullptr}, nullptr, N_RTE_RWY_BUF_LENGTH},

	// Some debug datarefs up in here

	{{"Strato/777/FMC/RAD_NAV/VOR_DME/c1", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},
	{{"Strato/777/FMC/RAD_NAV/VOR_DME/c2", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},
	{{"Strato/777/FMC/RAD_NAV/VOR_DME/c3", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},
	{{"Strato/777/FMC/RAD_NAV/VOR_DME/c4", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},

	{{"Strato/777/FMC/RAD_NAV/DME_DME/c1", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},
	{{"Strato/777/FMC/RAD_NAV/DME_DME/c2", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},
	{{"Strato/777/FMC/RAD_NAV/DME_DME/c3", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},
	{{"Strato/777/FMC/RAD_NAV/DME_DME/c4", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},
	{{"Strato/777/FMC/RAD_NAV/DME_DME/tuned_pair", DR_READONLY, false, nullptr}, nullptr, DEBUG_DR_LENGTH},

	// FMC L data refs:

	{{"Strato/777/FMC/FMC_L/REF_NAV/input_icao", DR_WRITABLE, false, nullptr}, nullptr, REF_NAV_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/REF_NAV/out_icao", DR_READONLY, false, nullptr}, nullptr, REF_NAV_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_mag_var", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_MAG_VAR_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/REF_NAV/navaid_1_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/REF_NAV/navaid_2_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/REF_NAV/vor_1_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/REF_NAV/vor_2_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},

	{{"Strato/777/FMC/FMC_L/RTE1/dep_icao_in", DR_WRITABLE, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/RTE1/arr_icao_in", DR_WRITABLE, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_L/RTE1/dep_rnw_in", DR_WRITABLE, false, nullptr}, nullptr, N_RTE_ICAO_BUF_LENGTH},

	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi1_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi2_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi3_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi4_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi5_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/poi6_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_L/scratchpad_msg", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},

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

StratosphereAvionics::avionics_in_drs av_in = {
											"sim/cockpit2/gauges/indicators/altitude_ft_pilot",
											"sim/cockpit2/gauges/indicators/altitude_ft_copilot",
											"sim/cockpit2/gauges/indicators/altitude_ft_stby",
											"sim/flightmodel/position/latitude",
											"sim/flightmodel/position/longitude",

											{
												{
													{
														"sim/cockpit2/radios/actuators/nav_frequency_hz",
														"sim/cockpit2/radios/indicators/nav1_nav_id",
														"sim/cockpit2/radios/indicators/nav1_dme_id",
														"sim/cockpit2/radios/indicators/nav_bearing_deg_mag",
														"sim/cockpit2/radios/indicators/nav_dme_distance_nm",
														0
													},

													{
														"sim/cockpit2/radios/actuators/nav_frequency_hz",
														"sim/cockpit2/radios/indicators/nav2_nav_id",
														"sim/cockpit2/radios/indicators/nav2_dme_id",
														"sim/cockpit2/radios/indicators/nav_bearing_deg_mag",
														"sim/cockpit2/radios/indicators/nav_dme_distance_nm",
														1
													},

													{
														"sim/cockpit2/radios/actuators/nav_frequency_hz",
														"sim/cockpit2/radios/indicators/nav3_nav_id",
														"sim/cockpit2/radios/indicators/nav3_dme_id",
														"sim/cockpit2/radios/indicators/nav_bearing_deg_mag",
														"sim/cockpit2/radios/indicators/nav_dme_distance_nm",
														2
													},

													{
														"sim/cockpit2/radios/actuators/nav_frequency_hz",
														"sim/cockpit2/radios/indicators/nav4_nav_id",
														"sim/cockpit2/radios/indicators/nav4_dme_id",
														"sim/cockpit2/radios/indicators/nav_bearing_deg_mag",
														"sim/cockpit2/radios/indicators/nav_dme_distance_nm",
														3
													}
												}
											}
};

StratosphereAvionics::avionics_out_drs av_out = {
											"Strato/777/FMC/RTE1/dep_icao_out",
											"Strato/777/FMC/RTE1/arr_icao_out",
											"Strato/777/FMC/RTE1/dep_rnw_out",
											{"Strato/777/FMC/REF_NAV/navaid_1_out",
											 "Strato/777/FMC/REF_NAV/navaid_2_out"},
											{"Strato/777/FMC/REF_NAV/vor_1_out",
											 "Strato/777/FMC/REF_NAV/vor_2_out"},

											{"Strato/777/FMC/RAD_NAV/VOR_DME/pos_lat",
											 "Strato/777/FMC/RAD_NAV/VOR_DME/pos_lon",
											 "Strato/777/FMC/RAD_NAV/VOR_DME/pos_fom",
											 "Strato/777/FMC/RAD_NAV/DME_DME/pos_lat",
											 "Strato/777/FMC/RAD_NAV/DME_DME/pos_lon",
											 "Strato/777/FMC/RAD_NAV/DME_DME/pos_fom",
											 "Strato/777/FMC/RAD_NAV/DME_DME/tuned_pair"},

											{{"Strato/777/FMC/RAD_NAV/VOR_DME/c1", 
											  "Strato/777/FMC/RAD_NAV/VOR_DME/c2",
											  "Strato/777/FMC/RAD_NAV/VOR_DME/c3",
											  "Strato/777/FMC/RAD_NAV/VOR_DME/c4"},

											 {"Strato/777/FMC/RAD_NAV/DME_DME/c1",
											  "Strato/777/FMC/RAD_NAV/DME_DME/c2",
											  "Strato/777/FMC/RAD_NAV/DME_DME/c3",
											  "Strato/777/FMC/RAD_NAV/DME_DME/c4"}}
											
};

StratosphereAvionics::fmc_in_drs fmc_l_in = {
											"sim/flightmodel/position/latitude",
											"sim/flightmodel/position/longitude",

										   {"Strato/777/FMC/FMC_L/REF_NAV/input_icao",
											"Strato/777/FMC/REF_NAV/rad_nav_inh",
											{"Strato/777/FMC/FMC_L/REF_NAV/navaid_1_in",
											 "Strato/777/FMC/FMC_L/REF_NAV/navaid_2_in"},
											{"Strato/777/FMC/FMC_L/REF_NAV/vor_1_in",
											 "Strato/777/FMC/FMC_L/REF_NAV/vor_2_in"}},
										   {"Strato/777/FMC/FMC_L/RTE1/dep_icao_in",
											"Strato/777/FMC/FMC_L/RTE1/arr_icao_in",
											"Strato/777/FMC/FMC_L/RTE1/dep_rnw_in"},
										   {"Strato/777/FMC/FMC_L/SEL_WPT/subpage",
											"Strato/777/FMC/FMC_L/SEL_WPT/wpt_idx"},

											"Strato/777/FMC/FMC_L/clear_msg",
											"Strato/777/FMC/FMC_L/page"
};

StratosphereAvionics::fmc_out_drs fmc_l_out = {
											 {"Strato/777/FMC/FMC_L/REF_NAV/out_icao",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_type",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_lat",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_lon",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_elev",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_freq",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_mag_var",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_length_ft",
											  "Strato/777/FMC/FMC_L/REF_NAV/poi_length_m"},

											 {"Strato/777/FMC/FMC_L/SEL_WPT/is_active",
											  "Strato/777/FMC/FMC_L/SEL_WPT/n_subpages",
											  "Strato/777/FMC/FMC_L/SEL_WPT/n_pois_disp",
											  "Strato/777/FMC/FMC_L/SEL_WPT/poi_list",
											  {"Strato/777/FMC/FMC_L/SEL_WPT/poi1_type",
											   "Strato/777/FMC/FMC_L/SEL_WPT/poi2_type",
											   "Strato/777/FMC/FMC_L/SEL_WPT/poi3_type",
											   "Strato/777/FMC/FMC_L/SEL_WPT/poi4_type",
											   "Strato/777/FMC/FMC_L/SEL_WPT/poi5_type",
											   "Strato/777/FMC/FMC_L/SEL_WPT/poi6_type"}},

											 {0, {"Strato/777/FMC/FMC_L/scratchpad/not_in_database"}}
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
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_mag_var",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_length_ft",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_length_m"},

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
