/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file provides definitions of functions that manage custom datarefs 
	used by this plugin
	Author: discord/bruh4096#4512(Tim G.)
*/


#pragma once

#include "dataref_structs.hpp"
#include <fmc_sys.hpp>
#include <vector>


typedef std::vector<XPDataBus::custom_data_ref_entry>* custom_dr_ptr;
typedef std::vector<DRUtil::dref_i>* dr_i_ptr;
typedef std::vector<DRUtil::dref_d>* dr_d_ptr;
typedef std::vector<DRUtil::dref_ia>* dr_ia_ptr;
typedef std::vector<DRUtil::dref_fa>* dr_fa_ptr;
typedef std::vector<DRUtil::dref_s>* dr_s_ptr;


namespace fmc_dr
{
	struct dr_init
	{
		dr_i_ptr int_drs;
		dr_d_ptr double_drs;
		dr_ia_ptr int_arr_drs;
		dr_fa_ptr float_arr_drs;
		dr_s_ptr str_drs;
	};

	
	int register_data_refs(custom_dr_ptr data_refs, dr_init drs)
	{
		for (size_t i = 0; i < drs.int_drs->size(); i++)
		{
			int ret = drs.int_drs->at(i).init();

			if (!ret)
			{
				return 0;
			}

			XPDataBus::custom_data_ref_entry e = { drs.int_drs->at(i).dr.name, 
					{(void*)&drs.int_drs->at(i).val, xplmType_Int, 1} };
			data_refs->push_back(e);
		}

		for (size_t i = 0; i < drs.double_drs->size(); i++)
		{
			int ret = drs.double_drs->at(i).init();

			if (!ret)
			{
				return 0;
			}

			XPDataBus::custom_data_ref_entry e = { drs.double_drs->at(i).dr.name, {(void*)&drs.double_drs->at(i).val,
													xplmType_Double, 1} };
			data_refs->push_back(e);
		}

		for (size_t i = 0; i < drs.int_arr_drs->size(); i++)
		{
			int ret = drs.int_arr_drs->at(i).init();

			if (!ret)
			{
				return 0;
			}

			XPDataBus::custom_data_ref_entry e = { drs.int_arr_drs->at(i).dr.name, {(void*)drs.int_arr_drs->at(i).array,
													xplmType_IntArray, drs.int_arr_drs->at(i).n_length} };
			data_refs->push_back(e);
		}

		for (size_t i = 0; i < drs.float_arr_drs->size(); i++)
		{
			int ret = drs.float_arr_drs->at(i).init();

			if (!ret)
			{
				return 0;
			}

			XPDataBus::custom_data_ref_entry e = { drs.float_arr_drs->at(i).dr.name, {(void*)drs.float_arr_drs->at(i).array,
													xplmType_FloatArray, drs.float_arr_drs->at(i).n_length} };
			data_refs->push_back(e);
		}

		for (size_t i = 0; i < drs.str_drs->size(); i++)
		{
			int ret = drs.str_drs->at(i).init();

			if (!ret)
			{
				return 0;
			}

			XPDataBus::custom_data_ref_entry e = { drs.str_drs->at(i).dr.name, {(void*)drs.str_drs->at(i).str,
													xplmType_Data, drs.str_drs->at(i).n_length} };
			data_refs->push_back(e);
		}

		return 1;
	}

	void unregister_data_refs(dr_init drs)
	{
		for (size_t i = 0; i < drs.int_drs->size(); i++)
		{
			drs.int_drs->at(i).unReg();
		}

		for (size_t i = 0; i < drs.double_drs->size(); i++)
		{
			drs.double_drs->at(i).unReg();
		}

		for (size_t i = 0; i < drs.int_arr_drs->size(); i++)
		{
			drs.int_arr_drs->at(i).unReg();
		}

		for (size_t i = 0; i < drs.float_arr_drs->size(); i++)
		{
			drs.float_arr_drs->at(i).unReg();
		}

		for (size_t i = 0; i < drs.str_drs->size(); i++)
		{
			drs.str_drs->at(i).unReg();
		}
	}
}
