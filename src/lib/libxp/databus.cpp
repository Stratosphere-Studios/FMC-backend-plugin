/*
	This source file contains definitions of all methods found in databus.h
*/

#include "databus.hpp"

namespace XPDataBus
{
	DataBus::DataBus(std::vector<custom_data_ref_entry>* data_refs, uint64_t max_q_refresh)
	{
		//Get x-plane/sdk versions and path.

		path_sep = new char[2];

		XPLMHostApplicationID hostId;
		XPLMGetVersions(&xplane_version, &sdk_version, &hostId);

		strcpy_safe(path_sep, 2, XPLMGetDirectorySeparator());

		xplane_path = get_xplane_path();
		prefs_path = get_prefs_path().append(path_sep);
		apt_dat_path = get_apt_dat_path();
		default_data_path = get_default_data_path();

		for (int i = 0; i < data_refs->size(); i++)
		{
			std::pair<std::string, generic_ptr> tmp = std::make_pair(data_refs->at(i).name, data_refs->at(i).val);
			custom_data_refs.insert(tmp);
		}
		max_queue_refresh = max_q_refresh;
		flt_loop_id = reg_flt_loop();
		XPLMScheduleFlightLoop(flt_loop_id, 1, true);
	}

	std::string DataBus::get_xplane_path()
	{
		char buf[CHAR_BUF_SIZE];
		XPLMGetSystemPath(buf);
		return buf;
	}

	std::string DataBus::get_prefs_path()
	{
		char buf[CHAR_BUF_SIZE];
		XPLMGetPrefsPath(buf);
		char* file_name = XPLMExtractFileAndPath(buf);
		return std::string(buf, 0, file_name - buf);
	}

	std::string DataBus::get_apt_dat_path()
	{
		std::string path = xplane_path;
		if (xplane_version < 12000)
		{
			path.append("Resources");
			path.append(path_sep);
			path.append("default scenery");
			path.append(path_sep);
			path.append("default apt dat");
			path.append(path_sep);
			path.append("Earth nav data");
			path.append(path_sep);
			path.append("apt.dat");
		}
		else if (xplane_version >= 12000 && xplane_version < 42000)
		{
			path.append("Global Scenery");
			path.append(path_sep);
			path.append("Global Airports");
			path.append(path_sep);
			path.append("Earth nav data");
			path.append(path_sep);
			path.append("apt.dat");
		}
		else
		{
			path.append("Deez NUTS! HAH! GOTEEEM!");
			path.append(path_sep);
			path.append("apt.dat");
		}
		return path;
	}

	std::string DataBus::get_default_data_path()
	{
		std::string path = xplane_path;
		path.append("Resources");
		path.append(path_sep);
		path.append("default data");
		path.append(path_sep);
		return path;
	}

	void DataBus::add_to_mag_var_queue(geo_point point, std::promise<float>* prom)
	{
		std::lock_guard<std::mutex> lock(mag_var_queue_mutex);
		mag_var_queue.push(mag_var_req{ point, prom });
	}

	void DataBus::add_to_get_queue(std::string dr_name, std::promise<generic_val>* prom, int offset)
	{
		std::lock_guard<std::mutex> lock(get_queue_mutex);
		get_queue.push(get_req{ dr_name, prom, offset });
	}

	XPLMDataRef DataBus::add_data_ref_entry(std::string* dr_name)
	{
		size_t length = dr_name->length() + 1;
		char* cstr = new char[length];
		strcpy_safe(cstr, length, dr_name->c_str());
		XPLMDataRef ref_ptr = XPLMFindDataRef(cstr);
		delete[] cstr;
		if (ref_ptr != nullptr)
		{
			data_refs[*dr_name] = data_ref_entry{ ref_ptr, XPLMGetDataRefTypes(ref_ptr) };
		}
		return ref_ptr;
	}

	float DataBus::get_mag_var(double lat, double lon)
	{
		std::promise<float> prom;
		std::future<float> fut_val = prom.get_future();
		add_to_mag_var_queue(geo_point{ lat, lon }, &prom);
		return fut_val.get();
	}

	generic_val DataBus::get_data(std::string dr_name, int offset)
	{
		std::promise<generic_val> prom;
		std::future<generic_val> fut_val = prom.get_future();
		add_to_get_queue(dr_name, &prom, offset);
		return fut_val.get();
	}

	int DataBus::get_datai(std::string dr_name, int offset)
	{
		generic_val val = get_data(dr_name, offset);
		int ret_val = 0;
		if (val.val_type == xplmType_Int)
		{
			ret_val = val.int_val;
		}
		else if (val.val_type == xplmType_Float)
		{
			ret_val = int(val.float_val);
		}
		else if (val.val_type == xplmType_Double)
		{
			ret_val = int(val.double_val);
		}
		return ret_val;
	}

	float DataBus::get_dataf(std::string dr_name, int offset)
	{
		generic_val val = get_data(dr_name, offset);
		float ret_val = 0;
		if (val.val_type == xplmType_Int)
		{
			ret_val = float(val.int_val);
		}
		else if (val.val_type == xplmType_Float)
		{
			ret_val = val.float_val;
		}
		else if (val.val_type == xplmType_Double)
		{
			ret_val = float(val.double_val);
		}
		return ret_val;
	}

	double DataBus::get_datad(std::string dr_name, int offset)
	{
		generic_val val = get_data(dr_name, offset);
		double ret_val = 0;
		if (val.val_type == xplmType_Int)
		{
			ret_val = double(val.int_val);
		}
		else if (val.val_type == xplmType_Float)
		{
			ret_val = double(val.float_val);
		}
		else if (val.val_type == xplmType_Double)
		{
			ret_val = val.double_val;
		}
		return ret_val;
	}

	std::string DataBus::get_data_s(std::string dr_name, int offset)
	{
		generic_val val = get_data(dr_name, offset);
		return val.str;
	}

	void DataBus::set_data(std::string dr_name, generic_val value)
	{
		std::lock_guard<std::mutex> lock(set_queue_mutex);
		set_queue.push(set_req{ dr_name, value });
	}

	void DataBus::set_datai(std::string dr_name, int value, int offset)
	{
		int val_type = xplmType_Int;
		if (offset >= 0)
		{
			val_type = xplmType_IntArray;
		}
		generic_val tmp = { {0}, "", val_type, offset};
		tmp.int_val = value;
		set_data(dr_name, tmp);
	}

	void DataBus::set_dataf(std::string dr_name, float value, int offset)
	{
		int val_type = xplmType_Float;
		if (offset >= 0)
		{
			val_type = xplmType_FloatArray;
		}
		generic_val tmp = { {0}, "", val_type, offset };
		tmp.float_val = value;
		set_data(dr_name, tmp);
	}

	void DataBus::set_datad(std::string dr_name, double value)
	{
		generic_val tmp = { {0}, "", xplmType_Double, 0 };
		tmp.double_val = value;
		set_data(dr_name, tmp);
	}

	void DataBus::set_data_s(std::string dr_name, std::string in, int offset)
	{
		/*
		* This function is for custom datarefs only.
		*/
		generic_val tmp = { {0}, in, xplmType_Data, offset };
		set_data(dr_name, tmp);
	}

	int DataBus::get_data_ref_value(std::string* dr_name, generic_val* out)
	{
		/*
		* This function gets a value of a dataref that isn't owned by this plugin
		*/
		data_ref_entry ref = data_refs[*dr_name];
		if (ref.dr_type == xplmType_Int)
		{
			out->val_type = xplmType_Int;
			out->int_val = XPLMGetDatai(ref.ref);
			return 1;
		}
		else if (ref.dr_type == xplmType_Float)
		{
			out->val_type = xplmType_Float;
			out->float_val = XPLMGetDataf(ref.ref);
			return 1;
		}
		else if (ref.dr_type == xplmType_Double)
		{
			out->val_type = xplmType_Double;
			out->double_val = XPLMGetDatad(ref.ref);
			return 1;
		}
		else if (xplmType_IntArray & ref.dr_type)
		{
			out->val_type = xplmType_Int;
			return XPLMGetDatavi(ref.ref, &out->int_val, out->offset, 1);
		}
		else if (xplmType_FloatArray & ref.dr_type)
		{
			out->val_type = xplmType_Float;
			return XPLMGetDatavf(ref.ref, &out->float_val, out->offset, 1);
		}
		return 0;
	}

	int DataBus::get_custom_data_ref_value(std::string* dr_name, generic_val* out)
	{
		/*
		* This function gets a value of a dataref that is owned by this plugin
		*/
		int offset = out->offset;
		generic_ptr ptr = custom_data_refs[*dr_name];
		if (ptr.ptr_type == xplmType_Int)
		{
			out->val_type = xplmType_Int;
			out->int_val = *reinterpret_cast<int*>(ptr.ptr);
			return 1;
		}
		else if (ptr.ptr_type == xplmType_Float)
		{
			out->val_type = xplmType_Float;
			out->float_val = *reinterpret_cast<float*>(ptr.ptr);
			return 1;
		}
		else if (ptr.ptr_type == xplmType_Double)
		{
			out->val_type = xplmType_Double;
			out->double_val = *reinterpret_cast<double*>(ptr.ptr);
			return 1;
		}
		else if (xplmType_IntArray & ptr.ptr_type && offset < ptr.n_length)
		{
			out->val_type = xplmType_Int;
			out->int_val = reinterpret_cast<int*>(ptr.ptr)[offset];
			return 1;
		}
		else if (xplmType_FloatArray & ptr.ptr_type && offset < ptr.n_length)
		{
			out->val_type = xplmType_Float;
			out->float_val = reinterpret_cast<float*>(ptr.ptr)[offset];
			return 1;
		}
		else if (xplmType_Data & ptr.ptr_type && offset < ptr.n_length)
		{
			out->val_type = xplmType_Data;
			char* data = reinterpret_cast<char*>(ptr.ptr);
			for (int i = offset; i < ptr.n_length && data[i]; i++)
			{
				out->str.push_back(data[i]);
			}
			return 1;
		}
		return 0;
	}

	void DataBus::set_data_ref_value(std::string* dr_name, generic_val* in)
	{
		/*
		* This function sets a value of a dataref that isn't owned by this plugin
		*/
		data_ref_entry ref = data_refs[*dr_name];
		if (ref.dr_type == xplmType_Int)
		{
			XPLMSetDatai(ref.ref, in->int_val);
		}
		else if (ref.dr_type == xplmType_Float)
		{
			XPLMSetDataf(ref.ref, in->float_val);
		}
		else if (ref.dr_type == xplmType_Double)
		{
			XPLMSetDatad(ref.ref, in->double_val);
		}
		else if (xplmType_IntArray & ref.dr_type)
		{
			XPLMSetDatavi(data_refs[*dr_name].ref, &in->int_val, in->offset, 1);
		}
		else if (xplmType_FloatArray & data_refs[*dr_name].dr_type)
		{
			XPLMSetDatavf(data_refs[*dr_name].ref, &in->float_val, in->offset, 1);
		}
	}

	void DataBus::set_custom_data_ref_value(std::string* dr_name, generic_val* in)
	{
		/*
		* This function sets a value of a dataref that is owned by this plugin.
		*/
		generic_ptr ptr = custom_data_refs[*dr_name];
		if (ptr.ptr_type == xplmType_Int)
		{
			*reinterpret_cast<int*>(ptr.ptr) = in->int_val;
		}
		else if (ptr.ptr_type == xplmType_Float)
		{
			*reinterpret_cast<float*>(ptr.ptr) = in->float_val;
		}
		else if (ptr.ptr_type == xplmType_Double)
		{
			*reinterpret_cast<double*>(ptr.ptr) = in->double_val;
		}
		else if (xplmType_IntArray & ptr.ptr_type)
		{
			reinterpret_cast<int*>(ptr.ptr)[in->offset] = in->int_val;
		}
		else if (xplmType_FloatArray & ptr.ptr_type)
		{
			reinterpret_cast<float*>(ptr.ptr)[in->offset] = in->float_val;
		}
		else if (xplmType_Data & ptr.ptr_type)
		{
			char* data = reinterpret_cast<char*>(ptr.ptr);
			size_t data_length = ptr.n_length - in->offset;
			size_t str_length = in->str.length();
			if (str_length < data_length)
			{
				data_length = str_length;
			}
			if (str_length == 1 && in->offset == -1) // Set all elements of output string to 1 character
			{
				for(int i = 0; i < ptr.n_length; i++)
				{
					data[i] = in->str.at(0);
				}
			}
			else
			{
				for (int i = 0; i < data_length; i++)
				{
					data[i + in->offset] = in->str.at(i);
				}
			}
		}
	}

	int DataBus::get_data_ref(std::string* dr_name, generic_val* out)
	{
		XPLMDataRef ref_ptr = nullptr;
		if (data_refs.find(*dr_name) != data_refs.end())
		{
			ref_ptr = data_refs[*dr_name].ref;
		}
		else
		{
			ref_ptr = add_data_ref_entry(dr_name);
		}
		if (ref_ptr != nullptr)
		{
			return get_data_ref_value(dr_name, out);
		}
		return 0;
	}

	int DataBus::get_custom_data_ref(std::string* dr_name, generic_val* out)
	{
		if (custom_data_refs.find(*dr_name) != custom_data_refs.end())
		{
			return get_custom_data_ref_value(dr_name, out);
		}
		return 0;
	}

	int DataBus::set_data_ref(std::string* dr_name, generic_val* in)
	{
		XPLMDataRef ref_ptr = nullptr;
		if (data_refs.find(*dr_name) != data_refs.end())
		{
			ref_ptr = data_refs[*dr_name].ref;
		}
		else
		{
			ref_ptr = add_data_ref_entry(dr_name);
		}
		if (ref_ptr != nullptr)
		{
			if (data_refs[*dr_name].dr_type & in->val_type)
			{
				set_data_ref_value(dr_name, in);
				return 1;
			}
			return 3;
		}
		return 0;
	}

	int DataBus::set_custom_data_ref(std::string* dr_name, generic_val* in)
	{
		if (custom_data_refs.find(*dr_name) != custom_data_refs.end())
		{
			if (custom_data_refs[*dr_name].ptr_type & in->val_type)
			{
				set_custom_data_ref_value(dr_name, in);
				return 1;
			}
			return 3;
		}
		return 0;
	}

	void DataBus::get_xplm_mag_var()
	{
		uint64_t counter = 0;
		while (mag_var_queue.size() && counter < max_queue_refresh)
		{
			std::lock_guard<std::mutex> lock(mag_var_queue_mutex);
			mag_var_req data = mag_var_queue.front();
			mag_var_queue.pop();

			float mag_var = XPLMGetMagneticVariation(data.point.lat, data.point.lon);

			data.prom->set_value(mag_var);

			counter++;
		}
	}

	void DataBus::get_data_refs()
	{
		uint64_t counter = 0;
		while (get_queue.size() && counter < max_queue_refresh)
		{
			std::lock_guard<std::mutex> lock(get_queue_mutex);
			get_req data = get_queue.front();
			get_queue.pop();
			generic_val tmp = { {0}, "", 0, data.offset};
			if (get_custom_data_ref(&data.dref, &tmp) != 1)
			{
				if (get_data_ref(&data.dref, &tmp) != 1)
				{
					tmp.offset = -1;
				}
			}
			data.prom->set_value(tmp);
			counter++;
		}
	}

	void DataBus::set_data_refs()
	{
		uint64_t counter = 0;
		while (set_queue.size() && counter < max_queue_refresh)
		{
			std::lock_guard<std::mutex> lock(set_queue_mutex);
			set_req data = set_queue.front();
			set_queue.pop();
			int retval = 0;
			if (set_custom_data_ref(&data.dref, &data.val) == 0)
			{
				retval = set_data_ref(&data.dref, &data.val);
			}
			counter++;
		}
	}

	XPLMFlightLoopID DataBus::reg_flt_loop()
	{
		XPLMCreateFlightLoop_t loop;
		loop.structSize = sizeof(XPLMCreateFlightLoop_t);
		loop.phase = 0; // ignored according to docs
		loop.refcon = this;
		loop.callbackFunc = [](float elapsedMe, float elapsedSim, int counter, void* ref) -> float 
								{
									DataBus* ptr = reinterpret_cast<DataBus*>(ref);
									ptr->get_xplm_mag_var();
									ptr->get_data_refs();
									ptr->set_data_refs();
									return -1;
								};
		return XPLMCreateFlightLoop(&loop);
	}

	void DataBus::cleanup()
	{
		while (get_queue.size())
		{
			generic_val tmp = { {0}, "", 0, 0 };
			get_req data = get_queue.front();
			get_queue.pop();
			data.prom->set_value(tmp);
		}
	}

	DataBus::~DataBus()
	{
		delete[] path_sep;
		if (flt_loop_id != nullptr)
		{
			XPLMDestroyFlightLoop(flt_loop_id);
		}
	}
}