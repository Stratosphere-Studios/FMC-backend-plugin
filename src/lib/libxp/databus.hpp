/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This library provides a bridge between X-plane and the plugin itself.
	It allows the plugin to access x-plane's data from a separate thread
	while keeping all of the calls to the XPLM api in the main thread.
	
	This header file contains declarations of all structures and functions used 
	by the data bus itself
	Author: discord/bruh4096#4512(Tim G.)
*/


#pragma once

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>
#include <XPLMPlugin.h>
#include <XPLMScenery.h>
#include "common.hpp"
#include <vector>
#include <queue>
#include <future>
#include <unordered_map>
#include <mutex>


namespace XPDataBus
{
	constexpr char DEFAULT_STR_FILL_CHAR = ' ';
	constexpr int CHAR_BUF_SIZE = 2048;
	constexpr int PATH_BUF_SIZE = 256;
	constexpr std::memory_order ATOMIC_ORDR = std::memory_order_seq_cst;

	struct geo_point
	{
		double lat, lon;
	};

	struct mag_var_req
	{
		geo_point point;
		std::promise<float>* prom;
	};

	struct get_req
	{
		std::string dref;
		std::promise<generic_val>* prom;
		int offset;
	};

	struct set_req
	{
		std::string dref;
		bool set_cmd;
		generic_val val;
	};

	struct data_ref_entry
	{
		XPLMDataRef ref;
		XPLMDataTypeID dr_type;
	};

	struct cmd_entry
	{
		std::string name;
		XPLMCommandRef ref;
	};

	struct custom_data_ref_entry
	{
		std::string name;
		generic_ptr val;
	};


	class DataBus
	{
	public:
		std::atomic<bool> is_operative;
		std::queue<mag_var_req> mag_var_queue;
		std::mutex mag_var_queue_mutex;
		std::queue<get_req> get_queue;
		std::mutex get_queue_mutex;
		std::queue<set_req> set_queue;
		std::mutex set_queue_mutex;
		uint64_t max_queue_refresh;

		int xplane_version;
		int sdk_version;
		char* path_sep;

		std::string xplane_path;
		std::string prefs_path;
		std::string apt_dat_path;
		std::string default_data_path;
		std::string plugin_data_path_sep;
		std::string plugin_data_path_no_sep;  // No trailing separator
		std::string plugin_sign;


		DataBus(std::vector<XPDataBus::cmd_entry>* cmds, 
			std::vector<custom_data_ref_entry>* data_refs, uint64_t max_q_refresh, 
			std::string sign);

		// Ran from any thread:

		float get_mag_var(double lat, double lon);

		generic_val get_data(std::string dr_name, int offset=0);

		int get_datai(std::string dr_name, int offset=0);

		float get_dataf(std::string dr_name, int offset=0);

		double get_datad(std::string dr_name, int offset=0);

		std::string get_data_s(std::string dr_name, int offset=0);

		void cmd_once(std::string cmd_name);

		void set_data(std::string dr_name, generic_val value);

		void set_datai(std::string dr_name, int value, int offset=-1);

		void set_dataf(std::string dr_name, float value, int offset=-1);

		void set_datad(std::string dr_name, double value);

		void set_data_s(std::string dr_name, std::string in, int offset=0);

		// Ran from main thread only:

		void get_xplm_mag_var();

		void get_data_refs();
		
		void set_data_refs();

		XPLMFlightLoopID reg_flt_loop();

		void cleanup();

		void disable();

		~DataBus();
		
	private:
		XPLMPluginID plug_id;
		XPLMFlightLoopID flt_loop_id;

		std::unordered_map<std::string, XPLMCommandRef> all_cmds;
		std::unordered_map<std::string, data_ref_entry> data_refs; // Datarefs not owned by this plugin
		std::unordered_map<std::string, generic_ptr> custom_data_refs; // Datarefs owned by this plugin

		std::string get_xplane_path();

		std::string get_prefs_path();

		std::string get_apt_dat_path();

		std::string get_default_data_path();

		std::string get_plugin_data_path();

		void add_to_mag_var_queue(geo_point point, std::promise<float>* prom);

		void add_to_get_queue(std::string dr_name, std::promise<generic_val>* prom, int offset);

		XPLMDataRef add_data_ref_entry(std::string* dr_name);

		// The get_ functions below return number of data items returned

		int get_data_ref_value(std::string* dr_name, generic_val* out);

		int get_custom_data_ref_value(std::string* dr_name, generic_val* out);

		int get_data_ref(std::string* dr_name, generic_val* out);

		int get_custom_data_ref(std::string* dr_name, generic_val* out);

		void trigger_cmd_once(std::string* cmd_name);

		void set_data_ref_value(std::string* dr_name, generic_val* in);

		void set_custom_data_ref_value(std::string* dr_name, generic_val* in);

		int set_data_ref(std::string* dr_name, generic_val* in);

		int set_custom_data_ref(std::string* dr_name, generic_val* in);
	};
}
