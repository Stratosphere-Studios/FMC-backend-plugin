/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file contains functions that are used by various source files of libxp
	Author: discord/bruh4096#4512(Tim G.)
*/


#pragma once

#include "XPLMDataAccess.h"
#include <string>


inline void strcpy_safe(char* dst, size_t length, const char* src)
{
	if (dst != nullptr && src != nullptr && length != 0)
	{
		for (size_t i = 0; i < length; i++)
		{
			dst[i] = src[i];
			if (src[i] == '\0')
			{
				return;
			}
		}
	}
}

inline void strcpy_adv(char* dst, size_t length, std::string* src)
{
	if (dst != nullptr && src != nullptr && length != 0)
	{
		for (size_t i = 0; i < length; i++)
		{
			char curr_char = src->at(i);
			dst[i] = curr_char;
			if (curr_char == '\0')
			{
				return;
			}
		}
	}
}

inline void strip_str(std::string* src, std::string* dst)
{
	size_t i = 0;
	while (i < src->length() && src->at(i) && src->at(i) != ' ')
	{
		dst->push_back(src->at(i));
		i++;
	}
}

namespace XPDataBus
{
	struct generic_ptr
	{
		void* ptr;
		int ptr_type;
		int n_length;
	};

	struct generic_val
	{
		union
		{
			int int_val;
			float float_val;
			double double_val;
		};
		std::string str;
		int val_type;
		int offset;
	};
}
