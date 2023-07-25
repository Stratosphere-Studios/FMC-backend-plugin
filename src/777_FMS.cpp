#include "777_dr_init.hpp"
#include "777_dr_decl.hpp"
#include <thread>

#ifndef XPLM400
	#error This is made to be compiled against the XPLM400 SDK
#endif


fmc_dr::dr_init d_init = { &int_datarefs, &double_datarefs, 
						   &int_arr_datarefs, &float_arr_datarefs, 
						   &str_datarefs };

std::vector<XPDataBus::custom_data_ref_entry> data_refs;


std::shared_ptr<XPDataBus::DataBus> sim_databus;
std::shared_ptr<StratosphereAvionics::AvionicsSys> avionics;
std::shared_ptr<StratosphereAvionics::FMC> fmc_r;
std::shared_ptr<std::thread> avionics_thread;
std::shared_ptr<std::thread> fmc_r_thread;

int data_refs_created = 0;


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
		avionics->sim_shutdown.store(true, std::memory_order_relaxed);
		fmc_r->sim_shutdown.store(true, std::memory_order_relaxed);
		sim_databus->cleanup();
		fmc_r_thread->join();
		avionics_thread->join();
		fmc_dr::unregister_data_refs(d_init);
		fmc_r_thread.reset();
		avionics.reset();
		sim_databus.reset();
		XPLMDebugString("777_FMS: Successfully disabled.\n");
	}
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }
