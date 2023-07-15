#include "dr_cache.hpp"


namespace XPDataBus
{
	DataRefCache::DataRefCache()
	{
		cache = {};
	}

	generic_val DataRefCache::get_val(std::string dr_name)
	{
		generic_val tmp = { {0}, "", xplmType_Unknown, 0 };
		if (cache.find(dr_name) != cache.end())
		{
			tmp = cache.at(dr_name);
		}
		return tmp;
	}

	std::string DataRefCache::get_val_s(std::string dr_name)
	{
		generic_val tmp = get_val(dr_name);
		return tmp.str;
	}

	void DataRefCache::set_val(std::string dr_name, generic_val val)
	{
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

	void DataRefCache::set_val_s(std::string dr_name, std::string in)
	{
		generic_val v = { {0}, in, xplmType_Data, 0 };
		set_val(dr_name, v);
	}
}
