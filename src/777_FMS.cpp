#include "dataref_structs.hpp"
#include "fmc_sys.hpp"
#include <thread>


enum FMS_constants
{
	N_MAX_DATABUS_QUEUE_PROC = 1024,
	N_CUSTOM_STR_DR_LENGTH = 2048,
	REF_NAV_ICAO_BUF_LENGTH = 5,
	FMC_SCREEN_LINE_LENGTH = 24,
	N_REF_NAV_MAG_VAR_BUF_LENGTH = 4,
	N_REF_NAV_NAVAID_BUF_LENGTH = 4
};

#ifndef XPLM400
	#error This is made to be compiled against the XPLM400 SDK
#endif

std::vector<int> fmc_l_int_dr = { -1, 1 };
std::vector<int> fmc_r_int_dr = { -1, 1 };


std::vector<DRUtil::dref_i> int_datarefs = {
	{{"Strato/777/UI/messages/creating_databases", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/REF_NAV/rad_nav_inh", DR_WRITABLE, false, nullptr}, nullptr},

	// FMC L data refs:

	{{"Strato/777/FMC/FMC_L/clear_msg", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/page", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/wpt_idx", DR_WRITABLE, false, nullptr}, &fmc_l_int_dr[0]},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_type", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/subpage", DR_WRITABLE, false, nullptr}, &fmc_r_int_dr[1]},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/n_subpages", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/is_active", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/SEL_WPT/n_pois_disp", DR_READONLY, false, nullptr}, nullptr},

	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/clear_msg", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/page", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/wpt_idx", DR_WRITABLE, false, nullptr}, &fmc_r_int_dr[0]},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_type", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/subpage", DR_WRITABLE, false, nullptr}, &fmc_r_int_dr[1]},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/n_subpages", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/is_active", DR_WRITABLE, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/n_pois_disp", DR_READONLY, false, nullptr}, nullptr},
};

std::vector<DRUtil::dref_d> double_datarefs = {
	// FMC L data refs:

	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_lat", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_lon", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_elev", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_L/REF_NAV/poi_freq", DR_READONLY, false, nullptr}, nullptr},

	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lat", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lon", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_elev", DR_READONLY, false, nullptr}, nullptr},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_freq", DR_READONLY, false, nullptr}, nullptr}
};

std::vector<DRUtil::dref_ia> int_arr_datarefs = {
	
};

std::vector<DRUtil::dref_fa> float_arr_datarefs = {
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi_list", DR_READONLY, false, nullptr}, nullptr, 3 * N_CDU_OUT_LINES}
};

std::vector<DRUtil::dref_s> str_datarefs = {
	{{"Strato/777/FMC/REF_NAV/navaid_1_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/navaid_2_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/vor_1_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/REF_NAV/vor_2_out", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},

	// FMC R data refs:

	{{"Strato/777/FMC/FMC_R/REF_NAV/input_icao", DR_WRITABLE, false, nullptr}, nullptr, REF_NAV_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/out_icao", DR_READONLY, false, nullptr}, nullptr, REF_NAV_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_mag_var", DR_READONLY, false, nullptr}, nullptr, N_REF_NAV_MAG_VAR_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/navaid_1_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/navaid_2_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/vor_1_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/vor_2_in", DR_WRITABLE, false, nullptr}, nullptr, N_REF_NAV_NAVAID_BUF_LENGTH},
	

	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi1_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi2_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi3_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi4_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi5_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/SEL_WPT/poi6_type", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/scratchpad_msg", DR_READONLY, false, nullptr}, nullptr, FMC_SCREEN_LINE_LENGTH}
};

std::vector<XPDataBus::custom_data_ref_entry> data_refs;


std::shared_ptr<XPDataBus::DataBus> sim_databus;
std::shared_ptr<StratosphereAvionics::AvionicsSys> avionics;
std::shared_ptr<StratosphereAvionics::FMC> fmc_r;
std::shared_ptr<std::thread> avionics_thread;
std::shared_ptr<std::thread> fmc_r_thread;

StratosphereAvionics::avionics_out_drs av_out = {
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

											  "Strato/777/FMC/FMC_R/scratchpad_msg"
											};

int data_refs_created = 0;

int register_data_refs()
{
	for (int i = 0; i < int_datarefs.size(); i++)
	{
		int ret = int_datarefs.at(i).init();

		if (!ret)
		{
			return 0;
		}

		XPDataBus::custom_data_ref_entry e = { int_datarefs.at(i).dr.name, {(void*)int_datarefs.at(i).val, xplmType_Int} };
		data_refs.push_back(e);
	}

	for (int i = 0; i < double_datarefs.size(); i++)
	{
		int ret = double_datarefs.at(i).init();

		if (!ret)
		{
			return 0;
		}

		XPDataBus::custom_data_ref_entry e = { double_datarefs.at(i).dr.name, {(void*)double_datarefs.at(i).val, 
												xplmType_Double} };
		data_refs.push_back(e);
	}

	for (int i = 0; i < int_arr_datarefs.size(); i++)
	{
		int ret = int_arr_datarefs.at(i).init();

		if (!ret)
		{
			return 0;
		}

		XPDataBus::custom_data_ref_entry e = { int_arr_datarefs.at(i).dr.name, {(void*)int_arr_datarefs.at(i).array,
												xplmType_IntArray, size_t(int_arr_datarefs.at(i).n_length)} };
		data_refs.push_back(e);
	}

	for (int i = 0; i < float_arr_datarefs.size(); i++)
	{
		int ret = float_arr_datarefs.at(i).init();

		if (!ret)
		{
			return 0;
		}

		XPDataBus::custom_data_ref_entry e = { float_arr_datarefs.at(i).dr.name, {(void*)float_arr_datarefs.at(i).array,
												xplmType_FloatArray, size_t(float_arr_datarefs.at(i).n_length)} };
		data_refs.push_back(e);
	}

	for (int i = 0; i < str_datarefs.size(); i++)
	{
		int ret = str_datarefs.at(i).init();

		if (!ret)
		{
			return 0;
		}

		XPDataBus::custom_data_ref_entry e = { str_datarefs.at(i).dr.name, {(void*)str_datarefs.at(i).str, 
												xplmType_Data, size_t(str_datarefs.at(i).n_length)}};
		data_refs.push_back(e);
	}

	return 1;
}

void unregister_data_refs()
{
	for (int i = 0; i < int_datarefs.size(); i++)
	{
		int_datarefs.at(i).unReg();
	}

	for (int i = 0; i < double_datarefs.size(); i++)
	{
		double_datarefs.at(i).unReg();
	}

	for (int i = 0; i < int_arr_datarefs.size(); i++)
	{
		int_arr_datarefs.at(i).unReg();
	}

	for (int i = 0; i < float_arr_datarefs.size(); i++)
	{
		float_arr_datarefs.at(i).unReg();
	}

	for (int i = 0; i < str_datarefs.size(); i++)
	{
		str_datarefs.at(i).unReg();
	}
}

float FMS_init_FLCB(float elapsedMe, float elapsedSim, int counter, void* refcon)
{
	sim_databus = std::make_shared<XPDataBus::DataBus>(&data_refs, N_MAX_DATABUS_QUEUE_PROC);
	avionics = std::make_shared<StratosphereAvionics::AvionicsSys>(sim_databus, av_out);
	fmc_r = std::make_shared<StratosphereAvionics::FMC>(avionics, fmc_r_in, fmc_r_out);
	avionics_thread = std::make_shared<std::thread>([]()
		{
			avionics->main_loop();
		});
	fmc_r_thread = std::make_shared<std::thread>([]()
		{
			fmc_r->main_loop();
		});
	return 0;
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	strcpy(outName, "777 FMS");
	strcpy(outSig, "stratosphere.systems.fmsplugin");
	strcpy(outDesc, "A plugin for simulating the FMS of the 777");

	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

	data_refs_created = register_data_refs();

	if (data_refs_created)
	{
		XPLMRegisterFlightLoopCallback(FMS_init_FLCB, -1, nullptr);
		XPLMDebugString("777_FMS: Initializing\n");
	}
	else
	{
		XPLMDebugString("777_FMS: Failed to initialize(err_dr_create). Disabling\n");
	}

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	if (data_refs_created)
	{
		XPLMDebugString("777_FMS: Disabling\n");
		avionics->sim_shutdown.store(true, std::memory_order_relaxed);
		fmc_r->sim_shutdown.store(true, std::memory_order_relaxed);
		sim_databus->cleanup();
		fmc_r_thread->join();
		avionics_thread->join();
		unregister_data_refs();
		fmc_r_thread.reset();
		avionics.reset();
		sim_databus.reset();
		XPLMDebugString("777_FMS: Successfully disabled.\n");
	}
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }
