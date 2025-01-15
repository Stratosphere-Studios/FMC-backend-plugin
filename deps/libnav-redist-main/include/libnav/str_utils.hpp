/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	Author: discord/bruh4096#4512

	This file contains utulity functions for strings. These allow you to convert 
	lat/lon to dms and vice versa and etc.
*/


#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctype.h>
#include <math.h>
#include <iostream>
#include <cstdint>


namespace strutils
{
	constexpr int N_LAT_STR_LENGTH = 9;
	constexpr int N_LON_STR_LENGTH = 10;
	constexpr char DEGREE_SYMBOL = '\370';


	inline bool is_numeric(std::string& s)
	{
		for(auto i: s)
        {
            if(std::isalpha(i))
            {
                return false;
            }
        }

		return true;
	}

	inline std::string double_to_str(double num, uint8_t precision)
	{
		std::stringstream s;
		s << std::fixed << std::setprecision(precision) << num;
		return s.str();
	}

	inline double strtod(std::string s)
    {
        size_t d_idx = 0;

        while(s[d_idx] != '.' && d_idx < s.size())
            d_idx++;

        double out = 0;
        double curr_p = 1;

        bool is_neg = s[0] == '-';
        
        if(d_idx)
        {
            for(size_t i = d_idx - 1; i >= size_t(is_neg); i--)
            {
                out += double(s[i] - '0') * curr_p;
                curr_p *= 10;
                if(i == size_t(is_neg))
                    break;
            }
        }

        curr_p = 0.1;

        for(size_t i = d_idx+1; i < s.size(); i++)
        {
            out += double(s[i] - '0') * curr_p;
            curr_p *= 0.1;
        }

        if(is_neg)
            out *= -1;

        return out;
    }

	/*
		Converts a double frequency to Boeing-style string representation
	*/

	inline std::string freq_to_str(double freq)
	{
		uint64_t freq_str_length = 0;
		for (double i = 1; i <= freq; i *= 10)
		{
			freq_str_length++;
		}
		if (freq > 9999)
		{
			freq /= 100;
			freq_str_length++;
		}
		else
		{
			freq_str_length += 2;
		}
		return std::to_string(freq).substr(0, freq_str_length);
	}

	/*
		Converts value in degrees to string Deg,Min,Sec notation
	*/

	inline std::string deg_to_str(double abs_deg, char deg_sbl=DEGREE_SYMBOL)
	{
		std::string s;
		std::vector<int> repr;

		if (abs_deg < 10)
		{
			s.append("0");
		}

		for (int i = 0; i < 3; i++)
		{
			int v = int(abs_deg);
			repr.push_back(v);
			abs_deg -= v;
			abs_deg *= 60;
		}

		s.append(std::to_string(repr[0]));
		s.append(std::string(1, deg_sbl));
		s.append(std::to_string(repr[1]));
		s.append(".");
		s.append(std::to_string(round(double(repr[2]) / 10)).substr(0, 1));

		return s;
	}

	/*
		Converts latitude value in degrees to string Deg,Min,Sec notation
	*/

	inline std::string lat_to_str(double lat_deg, char deg_sbl=DEGREE_SYMBOL)
	{
		std::string s;
		double abs_lat = abs(lat_deg);

		if (lat_deg < 0)
		{
			s.append("S");
		}
		else
		{
			s.append("N");
		}

		s.append(deg_to_str(abs_lat, deg_sbl));

		return s;
	}

	inline double str_to_lat(std::string& s)
	{
		if(int(s.length()) == N_LAT_STR_LENGTH)
		{
			double digit_pairs[4];
			int curr_idx = 0;
			for(size_t i = 1; i < N_LAT_STR_LENGTH + 1; i += 2)
			{
				int curr_pair = (s[i] - '0') * 10 + (s[i+1] - '0');
				digit_pairs[curr_idx] = double(curr_pair);

				curr_idx++;
			}

			double lat = digit_pairs[0] + digit_pairs[1] / 60 +
				digit_pairs[2] / 3600 + digit_pairs[3] / 360000;
			
			if(s[0] == 'S')
			{
				lat *= -1;
			}
			return lat;
		}
		return 0;
	}

	/*
		Converts longitude value in degrees to string Deg,Min,Sec notation
	*/

	inline std::string lon_to_str(double lon_deg, char deg_sbl=DEGREE_SYMBOL)
	{
		std::string s;
		double abs_lon = abs(lon_deg);
		if (lon_deg < 0)
		{
			s.append("W");
		}
		else
		{
			s.append("E");
		}

		if (abs_lon < 100)
		{
			s.append("0");
		}

		s.append(deg_to_str(abs_lon, deg_sbl));

		return s;
	}

	inline double str_to_lon(std::string& s)
	{
		if(int(s.length()) == N_LON_STR_LENGTH)
		{
			double digit_pairs[4];
			digit_pairs[0] = (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0');
			int curr_idx = 1;
			for(size_t i = 4; i < N_LON_STR_LENGTH + 1; i += 2)
			{
				int curr_pair = (s[i] - '0') * 10 + (s[i+1] - '0');
				digit_pairs[curr_idx] = double(curr_pair);

				curr_idx++;
			}

			double lon = digit_pairs[0] + digit_pairs[1] / 60 +
				digit_pairs[2] / 3600 + digit_pairs[3] / 360000;
			
			if(s[0] == 'W')
			{
				lon *= -1;
			}
			return lon;
		}
		return 0;
	}

	/*
		Converts magnetic variation value in degrees to Boeing-style notation
	*/

	inline std::string mag_var_to_str(double mag_var_deg)
	{
		int mag_var_rnd = int(round(mag_var_deg));
		int mag_var = ((mag_var_rnd) + 360) % 360;
		std::string str_mag_var = std::to_string(abs(mag_var_rnd));
		
		if (mag_var < 180)
		{
			return "W" + str_mag_var;
		}
		return "E" + str_mag_var;
	}

	/*
		Function: strip
		Description: removes a designated character from the start and end of the string
		@param in: input string
		@param sep: separator
		@Return: vector of strings
	*/

	inline std::string strip(std::string& in, char sep=' ')
    {
        size_t i_first = 0;
        size_t i_last = in.length()-1;

        while(in[i_first] == sep)
        {
            i_first++;
        }
        while(in[i_last] == sep)
        {
            i_last--;
        }

        return in.substr(i_first, i_last - i_first + 1);
    }

	/*
		Function: str_split
		Description: splits the string by a designated character
		@param in: input string
		@param sep: separator
		@param n_split: maximum number of columns to separate
		@Return: vector of strings
	*/

	inline std::vector<std::string> str_split(std::string& in, char sep=' ', 
		int n_split = INT32_MAX)
	{
		std::stringstream s(in);
		std::string tmp;
		std::vector<std::string> out;

		while(n_split && std::getline(s, tmp, sep))
		{
			if(tmp != "")
			{
				n_split--;
				out.push_back(strip(tmp, '\r'));
			}
		}
		if(!n_split)
		{
			std::getline(s, tmp);
			if(tmp.length())
				out.push_back(strip(tmp, '\r'));
		}

		return out;
	}

	inline int stoi_with_strip(std::string& s, char s_char=' ')
	{
		std::string s_stripped = strip(s, s_char);
		if(s_stripped != "")
		{
			return atoi(s_stripped.c_str());
		}

		return 0;
	}

	inline float stof_with_strip(std::string& s, char s_char=' ')
	{
		std::string s_stripped = strip(s, s_char);
		if(s_stripped != "")
		{
			return float(atof(s_stripped.c_str()));
		}

		return 0;
	}

	/*
		Function: normalize_rnw_id
		Description:
		Adds a leading 0 to runway IDs that need it. Runway IDs in some airports in e.g. US don't have
		leading 0s, however, in Boeing's data bases all runways have them.
		Param:
		id: target id
		Return:
		Returns a modified id.
	*/

	inline std::string normalize_rnw_id(std::string id)
	{
		if (id.length() == 1 || (id.length() == 2 && std::isalpha(id[1])))
		{
			id = "0" + id;
		}
		return id;
	}

	inline std::string get_rnw_id(std::string id, bool ignore_all=false)
	{
		if(id == "ALL" && !ignore_all)
		{
			return id;
		}
		if(id.length() > 2 && id.length() < 6 && id[0] == 'R' && id[1] == 'W')
		{
			size_t i = 2;
			while(i < id.length() && !isalpha(id[i]))
			{
				i++;
			}

			if(i >= id.length()-1)
			{
				std::string num_part = id.substr(2, id.length()-2);
				return normalize_rnw_id(num_part);
			}
		}
		return "";
	}
}; // namespace strutils
