#include "dr_cache.hpp"


namespace XPDataBus
{
	DataRefCache::DataRefCache()
	{
		cache = {};
	}

	generic_val DataRefCache::get_val(std::string dr_name)
	{
		std::lock_guard<std::mutex> lock(dr_cache_mutex);
		generic_val tmp = { {0}, "", xplmType_Unknown, 0 };
		if (cache.find(dr_name) != cache.end())
		{
			tmp = cache.at(dr_name);
		}
		return tmp;
	}

	int DataRefCache::get_val_i(std::string dr_name)
	{
		generic_val tmp = get_val(dr_name);
		
		if (tmp.val_type == xplmType_Int)
		{
			return tmp.int_val;
		}
		return 0;
	}

	std::string DataRefCache::get_val_s(std::string dr_name)
	{
		generic_val tmp = get_val(dr_name);
		return tmp.str;
	}

	void DataRefCache::set_val(std::string dr_name, generic_val val)
	{
		std::lock_guard<std::mutex> lock(dr_cache_mutex);
		if (cache.find(dr_name) != cache.end())
		{
			cache[dr_name] = val;
		}
		else
		{
			std::pair<std::string, generic_val> tmp = std::make_pair(dr_name, val);
			cache.insert(tmp);
		}
	}

	void DataRefCache::set_val_i(std::string dr_name, int in)
	{
		generic_val v = { {0}, "", xplmType_Int, 0 };
		v.int_val = in;
		set_val(dr_name, v);
	}

	void DataRefCache::set_val_s(std::string dr_name, std::string in)
	{
		generic_val v = { {0}, in, xplmType_Data, 0 };
		set_val(dr_name, v);
	}
}
