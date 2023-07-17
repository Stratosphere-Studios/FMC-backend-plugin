/*
	This library provides a bridge between X-plane and the plugin itself.
	It allows the plugin to access x-plane's data from a separate thread
	while keeping all of the calls to the XPLM api in the main thread.
*/

#pragma once

#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "common.hpp"
#include <vector>
#include <queue>
#include <future>
#include <unordered_map>
#include <mutex>


constexpr size_t CHAR_BUF_SIZE = 2048;


namespace XPDataBus
{
	struct get_req
	{
		std::string dref;
		std::promise<generic_val>* prom;
		int offset;
	};

	struct set_req
	{
		std::string dref;
		generic_val val;
	};

	struct data_ref_entry
	{
		XPLMDataRef ref;
		XPLMDataTypeID dr_type;
	};

	struct custom_data_ref_entry
	{
		std::string name;
		generic_ptr val;
	};

	class DataBus
	{
	public:
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

		DataBus(std::vector<custom_data_ref_entry>* data_refs, uint64_t max_q_refresh);

		//Ran from any thread:

		generic_val get_data(std::string dr_name, int offset=0);

		int get_datai(std::string dr_name, int offset=0);

		float get_dataf(std::string dr_name, int offset=0);

		double get_datad(std::string dr_name, int offset=0);

		std::string get_data_s(std::string dr_name, int offset=0);

		void set_data(std::string dr_name, generic_val value);

		void set_datai(std::string dr_name, int value, int offset=-1);

		void set_dataf(std::string dr_name, float value, int offset=-1);

		void set_datad(std::string dr_name, double value);

		void set_data_s(std::string dr_name, std::string in, int offset=0);

		//Ran from main thread only:

		void get_data_refs();
		
		void set_data_refs();

		XPLMFlightLoopID reg_flt_loop();

		void cleanup();

		~DataBus();
		
	private:
		XPLMFlightLoopID flt_loop_id;
		std::unordered_map<std::string, data_ref_entry> data_refs; //Datarefs not owned by this plugin
		std::unordered_map<std::string, generic_ptr> custom_data_refs; //Datarefs owned by this plugin

		std::string get_xplane_path();

		std::string get_prefs_path();

		std::string get_apt_dat_path();

		std::string get_default_data_path();

		void add_to_get_queue(std::string dr_name, std::promise<generic_val>* prom, int offset);

		XPLMDataRef add_data_ref_entry(std::string* dr_name);

		//The get_ functions below return number of data items returned

		int get_data_ref_value(std::string* dr_name, generic_val* out);

		int get_custom_data_ref_value(std::string* dr_name, generic_val* out);

		int get_data_ref(std::string* dr_name, generic_val* out);

		int get_custom_data_ref(std::string* dr_name, generic_val* out);

		void set_data_ref_value(std::string* dr_name, generic_val* in);

		void set_custom_data_ref_value(std::string* dr_name, generic_val* in);

		int set_data_ref(std::string* dr_name, generic_val* in);

		int set_custom_data_ref(std::string* dr_name, generic_val* in);
	};
}
