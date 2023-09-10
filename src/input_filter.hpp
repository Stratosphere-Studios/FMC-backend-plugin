#pragma once

#include "lib/libxp/dataref_structs.hpp"
#include <XPLMUtilities.h>
#include <XPLMProcessing.h>


namespace StratosphereAvionics
{
	namespace InputFiltering
	{
		constexpr int JOY_PITCH_IDX = 1;
		constexpr int JOY_ROLL_IDX = 2;
		constexpr int JOY_HEADING_IDX = 3;
		constexpr int DR_OVRD_OFF = 0;
		constexpr int DR_OVRD_ON = 1;
		constexpr int JOY_DR_ARR_LENGTH = 4;
		constexpr float FLCB_FRAMES = -1; // The flight loop callback of InputFilter is run every frame.
		constexpr float INPUT_FILTER_GAIN = 0.2f;
		constexpr float DEAD_ZONE_DEFAULT = 0.03f;


		class InputFilter
		{
		public:

			/*
				Function: InputFilter
				Description:
				Constructs an InputFilter object
				Param:
				dz_pitch: pitch dead zone. If the absolute value of the pitch axis is below this, the pitch axis value
				will be considered 0.
				dz_roll: roll dead zone. Analagous to pitch dead zone.
				dz_hdg: heading(yaw) dead zone. Analagous to pitch dead zone.
			*/

			InputFilter(float dz_pitch, float dz_roll, float dz_hdg)
			{
				// Set up the dead zones

				dead_zone_pitch = dz_pitch;
				dead_zone_roll = dz_roll;
				dead_zone_hdg = dz_hdg;

				// Create all dataref structures

				override_joy_pitch = { {"sim/operation/override/override_joystick_pitch", DR_WRITABLE, false, nullptr}, nullptr };
				override_joy_roll = { {"sim/operation/override/override_joystick_roll", DR_WRITABLE, false, nullptr}, nullptr };
				override_joy_hdg = { {"sim/operation/override/override_joystick_heading", DR_WRITABLE, false, nullptr}, nullptr };

				joy_axes = { {"sim/joystick/joy_mapped_axis_value", DR_WRITABLE, false, nullptr}, nullptr, JOY_DR_ARR_LENGTH };

				pitch_ratio = { {"sim/cockpit2/controls/yoke_pitch_ratio", DR_WRITABLE, false, nullptr}, nullptr };
				roll_ratio = { {"sim/cockpit2/controls/yoke_roll_ratio", DR_WRITABLE, false, nullptr}, nullptr };
				hdg_ratio = { {"sim/cockpit2/controls/yoke_heading_ratio", DR_WRITABLE, false, nullptr}, nullptr };

				// Initialize all data accessors

				int stat = 1;
				stat *= override_joy_pitch.init();
				stat *= override_joy_roll.init();
				stat *= override_joy_hdg.init();
				stat *= joy_axes.init();
				stat *= pitch_ratio.init();
				stat *= roll_ratio.init();
				stat *= hdg_ratio.init();

				if (stat == 0) // if we've failed to acquire all of the neccessary data accessors, stop initialization to prevent CTD.
				{
					XPLMDebugString("777_FMS: Failed to initialize input filter\n");
					return;
				}
				// Set override datarefs to 1:

				override_joy_pitch.set(DR_OVRD_ON);
				override_joy_roll.set(DR_OVRD_ON);
				override_joy_hdg.set(DR_OVRD_ON);

				// Register the flight loop callback

				flt_loop_id = reg_flt_loop();
				XPLMScheduleFlightLoop(flt_loop_id, FLCB_FRAMES, true);
			}

			/*
				Function: update_filter
				Description:
				Filters and updates all of the control datarefs.
			*/
			
			void update_filter()
			{
				update_dr(&joy_axes, &pitch_ratio, JOY_PITCH_IDX, dead_zone_pitch);
				update_dr(&joy_axes, &roll_ratio, JOY_ROLL_IDX, dead_zone_roll);
				update_dr(&joy_axes, &hdg_ratio, JOY_HEADING_IDX, dead_zone_hdg);
			}

			/*
				Function: update_dr
				Description:
				Applies an IIR(Infinite Impulse Response) low-pass filter to an input dataref.
				Param:
				in: pointer to input dataref.
				out: pointer to output dataref.
				in_idx: index of the input in the input dataref.
				dz: dead zone
			*/

			static void update_dr(DRUtil::dref_fa* in, DRUtil::dref_f* out, int in_idx, float dz)
			{
				float curr_val = out->get();
				if (fabs(curr_val) <= dz)
				{
					curr_val = 0;
				}
				float cmd = curr_val + (in->get(in_idx) - curr_val) * INPUT_FILTER_GAIN;
				out->set(cmd);
			}

			/*
				Function: ~InputFilter
				Description:
				Destroys an InputFilter object.
			*/

			~InputFilter()
			{
				// Set override datarefs to 0:

				override_joy_pitch.set(DR_OVRD_OFF);
				override_joy_roll.set(DR_OVRD_OFF);
				override_joy_hdg.set(DR_OVRD_OFF);

				if (flt_loop_id != nullptr)
				{
					XPLMDestroyFlightLoop(flt_loop_id);
				}
			}

		private:
			float dead_zone_pitch, dead_zone_roll, dead_zone_hdg;

			XPLMFlightLoopID flt_loop_id;

			DRUtil::dref_i override_joy_pitch, override_joy_roll, override_joy_hdg;

			DRUtil::dref_fa joy_axes;
			DRUtil::dref_f roll_ratio, pitch_ratio, hdg_ratio;


			/*
				Function: reg_flt_loop
				Description:
				Creates a flight loop for the input filter.
				Return:
				Returns the id of the registered flight loop.
			*/

			XPLMFlightLoopID reg_flt_loop()
			{
				XPLMCreateFlightLoop_t loop;
				loop.structSize = sizeof(XPLMCreateFlightLoop_t);
				loop.phase = 0; // ignored according to docs
				loop.refcon = this;
				loop.callbackFunc = [](float elapsedMe, float elapsedSim, int counter, void* ref) -> float
				{
					InputFilter* ptr = reinterpret_cast<InputFilter*>(ref);
					ptr->update_filter();
					return FLCB_FRAMES;
				};
				return XPLMCreateFlightLoop(&loop);
			}
		};

	} // namespace InputFiltering
} // namespace StratosphereAvionics
