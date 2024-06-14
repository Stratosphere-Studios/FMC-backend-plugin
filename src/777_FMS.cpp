#include "input_filter.hpp"
#include "777_dr_init.hpp"
#include "777_dr_decl.hpp"
#include <libnav/geo_utils.hpp>
#include <thread>

#ifndef XPLM400
	#error This is made to be compiled against the XPLM400 SDK
#endif

constexpr double POI_CACHE_TILE_SIZE_RAD = 5.0 * geo::DEG_TO_RAD;
constexpr int N_FMC_REFRESH_HZ = 20;
const char *PLUGIN_SIGN = "stratosphere.systems.fmsplugin";


fmc_dr::dr_init d_init = { &int_datarefs, &double_datarefs, 
						   &int_arr_datarefs, &float_arr_datarefs, 
						   &str_datarefs };

std::vector<XPDataBus::custom_data_ref_entry> data_refs;


std::shared_ptr<StratosphereAvionics::InputFiltering::InputFilter> input_filter;
std::shared_ptr<XPDataBus::DataBus> sim_databus;
std::shared_ptr<StratosphereAvionics::AvionicsSys> avionics;
std::shared_ptr<StratosphereAvionics::FMC> fmc_l;
std::shared_ptr<StratosphereAvionics::FMC> fmc_r;
std::shared_ptr<std::thread> avionics_thread;
std::shared_ptr<std::thread> fmc_l_thread;
std::shared_ptr<std::thread> fmc_r_thread;

int data_refs_created = 0;


float FMS_init_FLCB(float elapsedMe, float elapsedSim, int counter, void* refcon)
{
	(void)elapsedMe;
	(void)elapsedSim;
	(void)counter;
	(void)refcon;

	float dead_zone = StratosphereAvionics::InputFiltering::DEAD_ZONE_DEFAULT;
	input_filter = std::make_shared<StratosphereAvionics::InputFiltering::InputFilter>(
			dead_zone, dead_zone, dead_zone);
	sim_databus = std::make_shared<XPDataBus::DataBus>(&data_refs, N_MAX_DATABUS_QUEUE_PROC, 
		PLUGIN_SIGN);
	avionics = std::make_shared<StratosphereAvionics::AvionicsSys>(sim_databus, av_in, 
		av_out, POI_CACHE_TILE_SIZE_RAD, N_FMC_REFRESH_HZ);

	fmc_l = std::make_shared<StratosphereAvionics::FMC>(avionics, fmc_l_in, fmc_l_out, 
		N_FMC_REFRESH_HZ);
	fmc_r = std::make_shared<StratosphereAvionics::FMC>(avionics, fmc_r_in, fmc_r_out, 
		N_FMC_REFRESH_HZ);

	avionics_thread = std::make_shared<std::thread>([]()
		{
			avionics->main_loop();
		});
	fmc_l_thread = std::make_shared<std::thread>([]()
		{
			fmc_l->main_loop();
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
	strcpy(outSig, PLUGIN_SIGN);
	strcpy(outDesc, "A plugin for simulating the FMS of the 777");

	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

	data_refs_created = fmc_dr::register_data_refs(&data_refs, d_init);

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

		fmc_l->sim_shutdown.store(true, StratosphereAvionics::UPDATE_FLG_ORDR);
		fmc_r->sim_shutdown.store(true, StratosphereAvionics::UPDATE_FLG_ORDR);

		avionics->sim_shutdown.store(true, StratosphereAvionics::UPDATE_FLG_ORDR);

		sim_databus->cleanup();
		fmc_l_thread->join();
		fmc_r_thread->join();
		avionics_thread->join();

		fmc_dr::unregister_data_refs(d_init);
		data_refs.clear();

		fmc_l->disable();
		fmc_r->disable();
		avionics->disable();
		sim_databus->disable();

		fmc_l.reset();
		fmc_r.reset();
		fmc_l_thread.reset();
		fmc_r_thread.reset();
		avionics.reset();
		sim_databus.reset();
		input_filter.reset();
		XPLMDebugString("777_FMS: Successfully disabled.\n");
	}
	else
	{
		XPLMDebugString("777_FMS: Datarefs weren't registered. Disabled.\n");
	}
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) 
{ 
	(void)inFrom;
	(void)inMsg;
	(void)inParam;
}
