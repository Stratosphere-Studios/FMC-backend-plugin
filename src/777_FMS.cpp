#include "dataref_structs.hpp"
#include "fmc_sys.hpp"
#include <thread>


enum FMS_constants
{
	N_MAX_DATABUS_QUEUE_PROC = 1024,
	N_CUSTOM_STR_DR_LENGTH = 2048,
	NAV_REF_ICAO_BUF_LENGTH = 5,
	FMC_SCREEN_LINE_LENGTH = 24
};

#ifndef XPLM400
	#error This is made to be compiled against the XPLM400 SDK
#endif

std::vector<int> int_dr_values = { 0, 0 };
std::vector<double> double_dr_values = { 0, 0, 0, 0 };
char nav_ref_in_icao[NAV_REF_ICAO_BUF_LENGTH];
char nav_ref_out_icao[NAV_REF_ICAO_BUF_LENGTH];
char fmc_line_1_big[FMC_SCREEN_LINE_LENGTH];
char fmc_line_2_big[FMC_SCREEN_LINE_LENGTH];
char fmc_line_3_big[FMC_SCREEN_LINE_LENGTH];
char fmc_line_4_big[FMC_SCREEN_LINE_LENGTH];
char fmc_line_5_big[FMC_SCREEN_LINE_LENGTH];
char fmc_line_6_big[FMC_SCREEN_LINE_LENGTH];
char scratchpad_msg[FMC_SCREEN_LINE_LENGTH];

std::vector<DRUtil::dref_i> int_datarefs = {
	{{"Strato/777/UI/messages/creating_databases", false, nullptr}, &int_dr_values[0]},
	{{"Strato/777/FMC/FMC_R/clear_msg", true, nullptr}, &int_dr_values[1]}
};

std::vector<DRUtil::dref_d> double_datarefs = {
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lat", false, nullptr}, &double_dr_values[0]},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_lon", false, nullptr}, &double_dr_values[1]},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_elev", false, nullptr}, &double_dr_values[2]},
	{{"Strato/777/FMC/FMC_R/REF_NAV/poi_freq", false, nullptr}, &double_dr_values[3]}
};

std::vector<DRUtil::dref_s> str_datarefs = {
	{{"Strato/777/FMC/FMC_R/REF_NAV/input_icao", true, nullptr}, nav_ref_in_icao, NAV_REF_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/FMC_R/REF_NAV/out_icao", false, nullptr}, nav_ref_out_icao, NAV_REF_ICAO_BUF_LENGTH},
	{{"Strato/777/FMC/line_1_big", false, nullptr}, fmc_line_1_big, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/line_2_big", false, nullptr}, fmc_line_2_big, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/line_3_big", false, nullptr}, fmc_line_3_big, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/line_4_big", false, nullptr}, fmc_line_4_big, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/line_5_big", false, nullptr}, fmc_line_5_big, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/line_6_big", false, nullptr}, fmc_line_6_big, FMC_SCREEN_LINE_LENGTH},
	{{"Strato/777/FMC/FMC_R/scratchpad_msg", false, nullptr}, scratchpad_msg, FMC_SCREEN_LINE_LENGTH}
};

std::vector<XPDataBus::custom_data_ref_entry> data_refs;


std::shared_ptr<XPDataBus::DataBus> sim_databus;
std::shared_ptr<StratosphereAvionics::AvionicsSys> avionics;
std::shared_ptr<StratosphereAvionics::FMC> fmc_l;
std::shared_ptr<std::thread> avionics_thread;
std::shared_ptr<std::thread> fmc_thread;

StratosphereAvionics::fmc_in_drs fmc_in = { 
											"Strato/777/FMC/FMC_R/REF_NAV/input_icao",
											"Strato/777/FMC/FMC_R/clear_msg"
										  };
StratosphereAvionics::fmc_out_drs fmc_out = { 
											  "Strato/777/FMC/FMC_R/REF_NAV/out_icao",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_lat", 
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_lon", 
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_elev",
											  "Strato/777/FMC/FMC_R/REF_NAV/poi_freq",
											  "Strato/777/FMC/FMC_R/scratchpad_msg",
											  {
												"Strato/777/FMC/line_1_big",
												"Strato/777/FMC/line_2_big",
												"Strato/777/FMC/line_3_big",
												"Strato/777/FMC/line_4_big",
												"Strato/777/FMC/line_5_big",
												"Strato/777/FMC/line_6_big"
											  }
											};

int data_refs_created = 0;

int register_data_refs()
{
	int status = 1;

	for (int i = 0; i < int_datarefs.size(); i++)
	{
		int_datarefs.at(i).init();
		XPDataBus::custom_data_ref_entry e = { int_datarefs.at(i).dr.name, {(void*)int_datarefs.at(i).val, xplmType_Int} };
		data_refs.push_back(e);
	}

	for (int i = 0; i < double_datarefs.size(); i++)
	{
		double_datarefs.at(i).init();
		XPDataBus::custom_data_ref_entry e = { double_datarefs.at(i).dr.name, {(void*)double_datarefs.at(i).val, xplmType_Double} };
		data_refs.push_back(e);
	}

	for (int i = 0; i < str_datarefs.size(); i++)
	{
		str_datarefs.at(i).init();
		XPDataBus::custom_data_ref_entry e = { str_datarefs.at(i).dr.name, {(void*)str_datarefs.at(i).str, xplmType_Data, size_t(str_datarefs[i].n_length)}};
		data_refs.push_back(e);
	}

	return status;
}

void unregister_data_refs()
{
	for (int i = 0; i < int_datarefs.size(); i++)
	{
		int_datarefs.at(i).dr.unReg();
	}

	for (int i = 0; i < double_datarefs.size(); i++)
	{
		double_datarefs.at(i).dr.unReg();
	}

	for (int i = 0; i < str_datarefs.size(); i++)
	{
		str_datarefs.at(i).dr.unReg();
	}
}

float FMS_init_FLCB(float elapsedMe, float elapsedSim, int counter, void* refcon)
{
	sim_databus = std::make_shared<XPDataBus::DataBus>(&data_refs, N_MAX_DATABUS_QUEUE_PROC);
	avionics = std::make_shared<StratosphereAvionics::AvionicsSys>(sim_databus);
	fmc_l = std::make_shared<StratosphereAvionics::FMC>(avionics, &fmc_in, &fmc_out);
	avionics_thread = std::make_shared<std::thread>([]()
		{
			avionics->main_loop();
		});
	fmc_thread = std::make_shared<std::thread>([]()
		{
			fmc_l->main_loop();
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
	}

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	if (data_refs_created)
	{
		XPLMDebugString("777_FMS: Disabling\n");
		avionics->sim_shutdown.store(true, std::memory_order_seq_cst);
		fmc_l->sim_shutdown.store(true, std::memory_order_seq_cst);
		sim_databus->cleanup();
		fmc_thread->join();
		avionics_thread->join();
		unregister_data_refs();
		fmc_thread.reset();
		avionics.reset();
		sim_databus.reset();
		XPLMDebugString("777_FMS: Successfully disabled.\n");
	}
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }
