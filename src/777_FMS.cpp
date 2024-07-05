/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This source file defines the entry point for this plugin.
	Author: discord/bruh4096#4512(Tim G.)
*/


#include "input_filter.hpp"
#include "777_dr_init.hpp"
#include "777_dr_decl.hpp"
#include "pfd.hpp"
#include <libnav/geo_utils.hpp>
#include <libnav/common.hpp>
#include <thread>

#ifdef LIN
	#include <pthread.h>
#endif

#ifndef XPLM400
	#error This is made to be compiled against the XPLM400 SDK
#endif

constexpr double POI_CACHE_TILE_SIZE_RAD = 5.0 * geo::DEG_TO_RAD;
constexpr int N_FMC_REFRESH_HZ = 20;
constexpr vect2_t CAPT_PFD_POS = {20, 1392};
constexpr vect2_t FO_PFD_POS = {20, 26};
constexpr vect2_t PFD_SZ = {1337, 1337};
constexpr int CAPT_BRT_IDX = 0;
constexpr int FO_BRT_IDX = 4;
const char *PLUGIN_SIGN = "stratosphere.systems.fmsplugin";


fmc_dr::dr_init d_init = { &int_datarefs, &double_datarefs, 
						   &int_arr_datarefs, &float_arr_datarefs, 
						   &str_datarefs };

StratosphereAvionics::PFDdrs pfd_drs = {"Strato/777/mcp/ap_on", 
	"Strato/777/pfd/flt_dir_pilot", "Strato/777/pfd/flt_dir_copilot", 
	"Strato/777/fma/at_mode", "Strato/777/fma/active_roll_mode", 
	"Strato/777/fma/active_vert_mode", "sim/cockpit2/switches/instrument_brightness_ratio",
	CAPT_BRT_IDX, FO_BRT_IDX};

cairo_utils::test_drs tmp_drs = {"Strato/777/GUI/test_x", "Strato/777/GUI/test_y",
	"Strato/777/GUI/test_w", "Strato/777/GUI/test_h", "Strato/777/GUI/test_r", 
	"Strato/777/GUI/test_thick"};

std::vector<XPDataBus::cmd_entry> cmd_entries;
std::vector<XPDataBus::custom_data_ref_entry> data_refs;


std::shared_ptr<StratosphereAvionics::InputFiltering::InputFilter> input_filter;
std::shared_ptr<XPDataBus::DataBus> sim_databus;
std::shared_ptr<StratosphereAvionics::AvionicsSys> avionics;
std::shared_ptr<StratosphereAvionics::FMC> fmc_l;
std::shared_ptr<StratosphereAvionics::FMC> fmc_r;
std::shared_ptr<StratosphereAvionics::PFDData> pfd_data;
std::shared_ptr<StratosphereAvionics::PFD> capt_pfd;
std::shared_ptr<StratosphereAvionics::PFD> fo_pfd;
std::shared_ptr<std::thread> avionics_thread;
std::shared_ptr<std::thread> fmc_l_thread;
std::shared_ptr<std::thread> fmc_r_thread;
std::shared_ptr<std::thread> pfd_thread;

int data_refs_created = 0;
bool displays_created = false;
FT_Library lib;
FT_Face font;
cairo_font_face_t* myfont_face;


float FMS_init_FLCB(float elapsedMe, float elapsedSim, int counter, void* refcon)
{
	(void)elapsedMe;
	(void)elapsedSim;
	(void)counter;
	(void)refcon;

	float dead_zone = StratosphereAvionics::InputFiltering::DEAD_ZONE_DEFAULT;
	input_filter = std::make_shared<StratosphereAvionics::InputFiltering::InputFilter>(
			dead_zone, dead_zone, dead_zone);
	sim_databus = std::make_shared<XPDataBus::DataBus>(&cmd_entries, &data_refs, N_MAX_DATABUS_QUEUE_PROC, 
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

	FT_Init_FreeType(&lib);

	bool font_loaded = false;
	bool font_exists = libnav::does_file_exist(sim_databus->plugin_data_path_sep+
		"BoeingFont.ttf");
	if(font_exists)
	{
		try
		{
			font_loaded = font_utils_try_load_font(sim_databus->plugin_data_path_no_sep.c_str(), 
				"BoeingFont.ttf", lib, &font, &myfont_face);
		}
		catch(...)
		{
			font_loaded = false;
		}
	}
	
	
	if(font_loaded)
	{
		pfd_data = std::make_shared<StratosphereAvionics::PFDData>(sim_databus, pfd_drs, &tmp_drs);
		pfd_thread = std::make_shared<std::thread>([]()
		{
			pfd_data->update();
		});

		#ifdef LIN
			pthread_setname_np(pfd_thread->native_handle(), "PFD thread");
		#endif

		displays_created = true;
		capt_pfd = std::make_shared<StratosphereAvionics::PFD>(pfd_data, myfont_face, 
			CAPT_PFD_POS, PFD_SZ, 10);

		fo_pfd = std::make_shared<StratosphereAvionics::PFD>(pfd_data, myfont_face, 
			FO_PFD_POS, PFD_SZ, 10, false);

		XPLMDebugString("777_FMS: Displays have been created\n");
	}
	else
	{
		XPLMDebugString("777_FMS: Failed to load font. Displays won't be created.\n");
		if(!font_exists)
		{
			XPLMDebugString("777_FMS: Font file not found\n");
		}
	}

	return 0;
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	strcpy(outName, "777 FMS");
	strcpy(outSig, PLUGIN_SIGN);
	strcpy(outDesc, "A plugin for simulating the FMS of the 777");

	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

	data_refs_created = fmc_dr::register_data_refs(&cmd_entries, &data_refs, 
		&custom_cmds, d_init);

	if (data_refs_created)
	{
		XPLMRegisterFlightLoopCallback(FMS_init_FLCB, -1, nullptr);
		XPLMDebugString("777_FMS: Initializing\n");
		glewInit();
    	mt_cairo_render_glob_init(true);
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

		if(displays_created)
		{
			pfd_data->is_stopped.store(true, std::memory_order_relaxed);
			pfd_thread->join();

			capt_pfd->destroy();
			fo_pfd->destroy();
			pfd_data->destroy();

			capt_pfd.reset();
			fo_pfd.reset();
			pfd_data.reset();
			displays_created = false;
		}

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
		data_refs_created = 0;
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

#if	IBM
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID resvd)
{
	UNUSED(hinst);
	UNUSED(resvd);
	lacf_glew_dllmain_hook(reason);
	return (TRUE);
}
#endif	/* IBM */

