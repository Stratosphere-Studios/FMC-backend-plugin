/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file provides a simplified and scalable interface for X-plane's datarefs.
	Author: discord/bruh4096#4512(Tim G.)
*/


#pragma once

#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>
#include <XPLMPlugin.h>
#include <XPLMUtilities.h>
#include <string>

#define MSG_ADD_DATAREF 0x01000000

enum dr_access
{
	DR_READONLY = false,
	DR_WRITABLE = true
};


namespace DRUtil
{
	constexpr char DEFAULT_STR_FILL_CHAR = ' ';


	float regDRInDRE(float elapsedMe, float elapsedSim, int counter, void* ref);

	struct dref
	{
		/*
		Generic dataref structure. DO NOT USE!. This one has no type/value.
		*/
		const char* name;
		bool is_writable, is_allocated;
		XPLMDataRef xpdr = nullptr;

		void regInDRE()
		{
			/*
			"Tells" dataref editor that the dataref has been created.
			*/
			XPLMRegisterFlightLoopCallback(regDRInDRE, 1, this);
		}
		void unReg()
		{
			if (xpdr != nullptr)
			{
				XPLMUnregisterDataAccessor(xpdr);
				XPLMUnregisterFlightLoopCallback(regDRInDRE, this);
			}
		}
	};

	struct dref_i
	{
		/*
		Integer dataref structure.
		*/
		dref dr;
		int val;

		int get()
		{
			if (dr.xpdr != nullptr)
			{
				return XPLMGetDatai(dr.xpdr);
			}
			return -1;
		}

		void set(int v)
		{
			if (dr.xpdr != nullptr)
			{
				XPLMSetDatai(dr.xpdr, v);
			}
		}

		int init()
		{
			/*
			Initializes the dataref inside the sim.
			Also notifies the dataref editor about the new dataref.
			If the dataref already exists, just gives a handle to it
			*/

			XPLMDataRef test_dr = XPLMFindDataRef(dr.name);
			if (test_dr == nullptr)
			{
				dr.xpdr = XPLMRegisterDataAccessor(dr.name, xplmType_Int, dr.is_writable,
						[](void* ref) {
							dref_i* ptr = reinterpret_cast<dref_i*>(ref);
							return ptr->val;
						},
						[](void* ref, int newVal) {
							dref_i* ptr = reinterpret_cast<dref_i*>(ref);
							ptr->val = newVal;
						},
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						this, this);
				dr.xpdr = XPLMFindDataRef(dr.name);
				dr.regInDRE();
			}
			else
			{
				//Set variable to dataref's value if dataref already exists
				dr.xpdr = test_dr;
				val = XPLMGetDatai(dr.xpdr);
			}
			if (dr.xpdr == nullptr)
			{
				return 0;
			}
			return 1;
		}

		void unReg()
		{
			dr.unReg();
		}
	};

	struct dref_f
	{
		/*
		Double dataref structure.
		*/
		dref dr;
		float val;

		float get()
		{
			if (dr.xpdr != nullptr)
			{
				return XPLMGetDataf(dr.xpdr);
			}
			return -1;
		}

		void set(float v)
		{
			if (dr.xpdr != nullptr)
			{
				XPLMSetDataf(dr.xpdr, v);
			}
		}

		int init()
		{
			/*
			Initializes the dataref inside the sim.
			Also notifies the dataref editor about the new dataref.
			If the dataref already exists, just gives a handle to it
			*/

			XPLMDataRef test_dr = XPLMFindDataRef(dr.name);
			if (test_dr == nullptr)
			{
				dr.xpdr = XPLMRegisterDataAccessor(dr.name, xplmType_Float, dr.is_writable,
						nullptr, nullptr,
						[](void* ref) -> float {
							dref_f* ptr = reinterpret_cast<dref_f*>(ref);
							return ptr->val;
						},
						[](void* ref, float newVal) {
							dref_f* ptr = reinterpret_cast<dref_f*>(ref);
							ptr->val = newVal;
						},
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						this, this);
				dr.xpdr = XPLMFindDataRef(dr.name);
				dr.regInDRE();
			}
			else
			{
				//Set variable to dataref's value if dataref already exists
				dr.xpdr = test_dr;
				val = XPLMGetDataf(dr.xpdr);
			}
			if (dr.xpdr == nullptr)
			{
				return 0;
			}
			return 1;
		}

		void unReg()
		{
			dr.unReg();
		}
	};

	struct dref_d
	{
		/*
		Double dataref structure.
		*/
		dref dr;
		double val;

		double get()
		{
			if (dr.xpdr != nullptr)
			{
				val = XPLMGetDatad(dr.xpdr);
				return val;
			}
			return -1;
		}

		void set(double v)
		{
			if (dr.xpdr != nullptr)
			{
				XPLMSetDatad(dr.xpdr, v);
			}
		}

		int init()
		{
			/*
			Initializes the dataref inside the sim.
			Also notifies the dataref editor about the new dataref.
			If the dataref already exists, just gives a handle to it
			*/

			XPLMDataRef test_dr = XPLMFindDataRef(dr.name);
			if (test_dr == nullptr)
			{
				dr.xpdr = XPLMRegisterDataAccessor(dr.name, xplmType_Double, dr.is_writable,
						nullptr, nullptr,
						nullptr, nullptr,
						[](void* ref) -> double {
							dref_d* ptr = reinterpret_cast<dref_d*>(ref);
							return ptr->val;
						},
						[](void* ref, double newVal) {
							dref_d* ptr = reinterpret_cast<dref_d*>(ref);
							ptr->val = newVal;
						},
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						this, this);
				dr.xpdr = XPLMFindDataRef(dr.name);
				dr.regInDRE();
			}
			else
			{
				//Set variable to dataref's value if dataref already exists
				dr.xpdr = test_dr;
				val = XPLMGetDatad(dr.xpdr);
			}
			if (dr.xpdr == nullptr)
			{
				return 0;
			}
			return 1;
		}

		void unReg()
		{
			dr.unReg();
		}
	};

	struct dref_ia
	{
		/*
		Integer array dataref structure.
		*/
		dref dr;
		int* array;
		int n_length;

		int get(int pos)
		{
			if (dr.xpdr != nullptr)
			{
				int n_dr_length = XPLMGetDatavi(dr.xpdr, nullptr, 0, 0);
				int retval;
				if (pos < n_dr_length)
				{
					XPLMGetDatavi(dr.xpdr, &retval, pos, 1);
					return retval;
				}
			}
			return -1;
		}

		void set(int pos)
		{
			if (dr.xpdr != nullptr && pos < n_length)
			{
				XPLMSetDatavi(dr.xpdr, &array[pos], pos, 1);
			}
		}

		int init()
		{
			/*
			Initializes the dataref inside the sim.
			Also notifies the dataref editor about the new dataref.
			If the dataref already exists, just gives a handle to it
			*/

			if (array == nullptr)
			{
				dr.is_allocated = true;
				array = new int[n_length];
				for (int i = 0; i < n_length; i++)
				{
					array[i] = 0;
				}
			}

			if (array == nullptr)
			{
				// If we've failed to allocate memory for the dataref,
				// return.
				return 0;
			}

			XPLMDataRef test_dr = XPLMFindDataRef(dr.name);
			if (test_dr == nullptr)
			{
				dr.xpdr = XPLMRegisterDataAccessor(dr.name, xplmType_IntArray, dr.is_writable,
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						[](void* ref, int* out_values, int in_offset, int in_max) {
							dref_ia* ptr = reinterpret_cast<dref_ia*>(ref);
							int arr_length = ptr->n_length;
							if (out_values == nullptr || in_offset >= arr_length)
							{
								return arr_length;
							}
							int r = arr_length - in_offset;
							if (r > in_max)
							{
								r = in_max;
							}
							for (int i = 0; i < r; i++)
							{
								out_values[i] = ptr->array[i + in_offset];
							}
							return r;
						},
						[](void* ref, int* in_values, int in_offset, int in_max) {
							if (in_values != nullptr)
							{
								dref_ia* ptr = reinterpret_cast<dref_ia*>(ref);
								int arr_length = ptr->n_length;
								int r = arr_length - in_offset;
								if (r > in_max)
									r = in_max;
								for (int i = 0; i < r; i++)
								{
									ptr->array[i + in_offset] = in_values[i];
								}
							}
						},
						nullptr, nullptr,
						nullptr, nullptr,
						this, this);
				dr.xpdr = XPLMFindDataRef(dr.name);
				XPLMSetDatavi(dr.xpdr, array, 0, n_length);
				dr.regInDRE();
			}
			else
			{
				//Set variable to dataref's value if dataref already exists
				dr.xpdr = test_dr;
				int n_dr_length = XPLMGetDatavi(dr.xpdr, nullptr, 0, 0);
				int n_val_get = n_length;
				if (n_dr_length < n_val_get)
				{
					n_val_get = n_dr_length;
				}
				XPLMGetDatavi(dr.xpdr, array, 0, n_val_get);
			}
			if (dr.xpdr == nullptr)
			{
				return 0;
			}
			
			return 1;
		}

		void unReg()
		{
			dr.unReg();
			if (dr.is_allocated)
			{
				delete[] array;
				array = nullptr;
			}
		}
	};

	struct dref_fa
	{
		/*
			Float array dataref structure.
		*/
		dref dr;
		float* array;
		int n_length;

		float get(int pos)
		{
			if (dr.xpdr != nullptr)
			{
				int n_dr_length = XPLMGetDatavf(dr.xpdr, nullptr, 0, 0);
				float retval;
				if (pos < n_dr_length)
				{
					XPLMGetDatavf(dr.xpdr, &retval, pos, 1);
					return retval;
				}
			}
			return -1;
		}

		void set(int pos)
		{
			if (dr.xpdr != nullptr && pos < n_length)
			{
				XPLMSetDatavf(dr.xpdr, &array[pos], pos, 1);
			}
		}

		int init()
		{
			/*
				Initializes the dataref inside the sim.
				Also notifies the dataref editor about the new dataref.
				If the dataref already exists, just gives a handle to it
			*/

			if (array == nullptr)
			{
				dr.is_allocated = true;
				array = new float[n_length];
				for (int i = 0; i < n_length; i++)
				{
					array[i] = 0;
				}
			}

			if (array == nullptr)
			{
				// If we've failed to allocate memory for the dataref,
				// return.
				return 0;
			}

			XPLMDataRef test_dr = XPLMFindDataRef(dr.name);
			if (test_dr == nullptr)
			{
				dr.xpdr = XPLMRegisterDataAccessor(dr.name, xplmType_FloatArray, dr.is_writable,
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						nullptr, nullptr,
						[](void* ref, float* out_values, int in_offset, int in_max) {
							dref_fa* ptr = reinterpret_cast<dref_fa*>(ref);
							int arr_length = ptr->n_length;
							if (out_values == nullptr || in_offset >= arr_length)
							{
								return arr_length;
							}
							int r = arr_length - in_offset;
							if (r > in_max)
							{
								r = in_max;
							}
							for (int i = 0; i < r; i++)
							{
								out_values[i] = ptr->array[i + in_offset];
							}
							return r;
						},
						[](void* ref, float* in_values, int in_offset, int in_max) {
							if (in_values != nullptr)
							{
								dref_fa* ptr = reinterpret_cast<dref_fa*>(ref);
								int arr_length = ptr->n_length;
								int r = arr_length - in_offset;
								if (r > in_max)
									r = in_max;
								for (int i = 0; i < r; i++)
								{
									ptr->array[i + in_offset] = in_values[i];
								}
							}
						},
						nullptr, nullptr,
						this, this);
				dr.xpdr = XPLMFindDataRef(dr.name);
				XPLMSetDatavf(dr.xpdr, array, 0, n_length);
				dr.regInDRE();
			}
			else
			{
				//Set variable to dataref's value if dataref already exists
				dr.xpdr = test_dr;
				int n_dr_length = XPLMGetDatavf(dr.xpdr, nullptr, 0, 0);
				int n_val_get = n_length;
				if (n_dr_length < n_val_get)
				{
					n_val_get = n_dr_length;
				}
				XPLMGetDatavf(dr.xpdr, array, 0, n_val_get);
			}
			
			if (dr.xpdr == nullptr)
			{
				return 0;
			}
			
			return 1;
		}

		void unReg()
		{
			dr.unReg();
			if (dr.is_allocated)
			{
				delete[] array;
				array = nullptr;
			}
		}
	};

	struct dref_s
	{
		/*
		String array dataref structure.
		*/
		dref dr;
		char* str;
		int n_length;

		//TODO: add get() and set()

		int init()
		{
			/*
			Initializes the dataref inside the sim.
			Also notifies the dataref editor about the new dataref.
			*/

			if (str == nullptr)
			{
				dr.is_allocated = true;
				str = new char[n_length];
				for (int i = 0; i < n_length; i++)
				{
					str[i] = DEFAULT_STR_FILL_CHAR;
				}
			}

			if (str == nullptr)
			{
				// If we've failed to allocate memory for the dataref,
				// return.
				return 0;
			}

			XPLMDataRef test_dr = XPLMFindDataRef(dr.name);
			if (test_dr == nullptr)
			{
				dr.xpdr = XPLMRegisterDataAccessor(dr.name, xplmType_Data, dr.is_writable,
					nullptr, nullptr,
					nullptr, nullptr,
					nullptr, nullptr,
					nullptr, nullptr,
					nullptr, nullptr,
					[](void* ref, void* out_values, int in_offset, int in_max) {
						//This stuff is really janky
						dref_s* ptr = reinterpret_cast<dref_s*>(ref);
						char* out_ptr = reinterpret_cast<char*>(out_values);
						int str_length = ptr->n_length;
						if (out_values == nullptr || in_offset >= str_length)
						{
							return str_length;
						}
						int r = str_length - in_offset;
						if (r > in_max)
						{
							r = in_max;
						}
						for (int i = 0; i < r; i++)
						{
							out_ptr[i] = ptr->str[i + in_offset];
						}
						return str_length;
					},
					[](void* ref, void* in_values, int in_offset, int in_max) {
						//This is janky as well
						if (in_values != nullptr)
						{
							dref_s* ptr = reinterpret_cast<dref_s*>(ref);
							char* in_ptr = reinterpret_cast<char*>(in_values);
							int str_length = ptr->n_length;
							int r = str_length - in_offset;
							if (r > in_max)
								r = in_max;
							for (int i = 0; i < r; i++)
							{
								ptr->str[i + in_offset] = in_ptr[i];
							}
						}
					},
						this, this);
				dr.xpdr = XPLMFindDataRef(dr.name);
				XPLMSetDatab(dr.xpdr, str, 0, n_length);
				dr.regInDRE();
			}
			else
			{
				dr.xpdr = test_dr;
			}
			
			if (dr.xpdr == nullptr)
			{
				return 0;
			}
			
			return 1;
		}

		void unReg()
		{
			dr.unReg();
			if (dr.is_allocated)
			{
				delete[] str;
				str = nullptr;
			}
		}
	};

	float regDRInDRE(float elapsedMe, float elapsedSim, int counter, void* ref)
	{
		/*
		* Do NOT use this function in any of your own code. This function is intended
		* to be used only within this library.
		*/

		(void)elapsedMe;
		(void)elapsedSim;
		(void)counter;

		XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
		if (PluginID != XPLM_NO_PLUGIN_ID)
		{
			dref* ptr = reinterpret_cast<dref*>(ref);
			XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)ptr->name);
		}
		return 0;
	}
};
