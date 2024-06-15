/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This header file contains declarations of classes, functions, etc 
	used in the PFD implementation. Author: discord/bruh4096#4512(Tim G.)
*/


#pragma once

#include <acfutils/log.h>
#include <acfutils/font_utils.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <string>
#include <libxp/databus.hpp>
#include <libtime/timer.hpp>
#include <cairo_utils.hpp>
#include <chrono>
#include <thread>


namespace StratosphereAvionics
{
    constexpr vect3_t GREEN = {0, 1, 0};

    constexpr XPLMDrawingPhase PFD_DRAW_PHASE = xplm_Phase_Gauges;
    constexpr int PFD_DRAW_BEFORE = 0;
    constexpr int PFD_FMA_RECT_LINE_SZ = 3;

    constexpr double PFD_FMA_FONT_SZ = 50;
    constexpr double PFD_FMA_TXT_Y = 32-5;
    constexpr double PFD_FMA_SPD_TXT_X = 337;
    constexpr double PFD_FMA_ROLL_TXT_X = 603;
    constexpr double PFD_FMA_PITCH_TXT_X = 857.5;

    constexpr double PFD_FMA_RECT_Y = 10-5;
    constexpr double PFD_FMA_SPD_RECT_X = 207;
    constexpr double PFD_FMA_ROLL_RECT_X = 483;
    constexpr double PFD_FMA_PITCH_RECT_X = 730;
    constexpr double PFD_FMA_RECT_H = 46;
    constexpr double PFD_FMA_SPD_RECT_W = 269;
    constexpr double PFD_FMA_ROLL_RECT_W = 240;
    constexpr double PFD_FMA_PITCH_RECT_W = 255;

    constexpr double PFD_FMA_AP_FONT_SZ = 72;
    constexpr double PFD_FMA_AP_RECT_X = 445;
    constexpr double PFD_FMA_AP_RECT_Y = 232-5;
    constexpr double PFD_FMA_AP_RECT_W = 276;
    constexpr double PFD_FMA_AP_RECT_H = 60;
    constexpr int PFD_FMA_AP_RECT_LINE_SZ = 4;

    constexpr double PFD_FMA_AP_TXT_X = PFD_FMA_AP_RECT_X + PFD_FMA_AP_RECT_W / 2;
    constexpr double PFD_FMA_AP_TXT_Y = PFD_FMA_AP_RECT_Y + PFD_FMA_AP_RECT_H / 2;

    constexpr double PFD_FMA_DATA_UPDATE_HZ = 5;
    constexpr double PFD_FMA_RECT_SHOW_SEC = 5;

    const std::string FLT_DIR_TXT = "FLT DIR";
    const std::string AP_TXT = "A/P";


    enum ATModes
    {
        AT_MODE_OFF = 0,
        AT_MODE_IAS_HOLD = 1,
        AT_MODE_RETARD = 2,
        AT_MODE_HOLD = 3,
        AT_MODE_THR_REF = 4,
        AT_MODE_FLC_RETARD = 5,
        AT_MODE_FLC_REF = 6
    };

    enum RollModes
    {
        ROLL_MODE_OFF = 0,
        ROLL_MODE_HDG_HOLD = 1,
        ROLL_MODE_HDG_SEL = 2,
        ROLL_MODE_TRK_HOLD = 3,
        ROLL_MODE_TRK_SEL = 4
    };

    enum PitchModes
    {
        PITCH_MODE_OFF = 0,
        PITCH_MODE_VSHOLD = 1,
        PITCH_MODE_FPAHOLD = 2,
        PITCH_MODE_ALTHOLD = 3,
        PITCH_MODE_FLC_CLB = 4,
        PITCH_MODE_FLC_DES = 5
    };

    inline std::string get_at_mode_txt(ATModes mode);

    inline std::string get_roll_mode_txt(RollModes mode);

    inline std::string get_pitch_mode_txt(PitchModes mode);


    inline void pfd_display_render_cb(cairo_t *cr, unsigned w, unsigned h, void *data);

    inline int pfd_draw_loop(XPLMDrawingPhase phase, int is_before, void *refcon);


    struct PFDdrs
    {
        std::string ap_eng, flt_dir_capt, flt_dir_fo, fma_thr, fma_roll, fma_pitch;
    };


    class PFDData
    {
    public:
        std::atomic<bool> is_stopped;

        cairo_utils::test_drs *test_drs;
        vect2_t test_pos, test_sz;
        double test_rad, test_thick;


        PFDData(std::shared_ptr<XPDataBus::DataBus> db, PFDdrs drs, 
            cairo_utils::test_drs *tst=nullptr, double ref=PFD_FMA_DATA_UPDATE_HZ);

        void update();

        void update_test();

        std::string get_capt_ap_fd();

        std::string get_fo_ap_fd();

        std::string get_spd_mode();

        std::string get_roll_mode();

        std::string get_pitch_mode();

        bool flt_dir_capt_changed();

        bool flt_dir_fo_changed();

        bool spd_changed();

        bool roll_changed();

        bool pitch_changed();

        void destroy();

    private:
        std::string ap_dir_cap, ap_dir_fo, spd_md, roll_md, pitch_md;
        double fd_cap_time, fd_fo_time, spd_time, roll_time, pitch_time;

        std::shared_ptr<XPDataBus::DataBus> data_bus;
        PFDdrs state_drs;

        double ref_hz;
        libtime::SteadyTimer *tmr;

        std::string get_ap_fd_txt(int ap_on, int fd_on);

        void update_ap_fd();

        void update_param(std::string& curr, std::string& prev, double *out);
    };

    class PFD
    {
    public:
        PFD(std::shared_ptr<PFDData> data, cairo_font_face_t* ff, 
            vect2_t p, vect2_t sz, double fps, bool capt=true);

        void update_screen();

        void perform_cairo_drawing(cairo_t* cr);

        void destroy();

    private:
        bool is_capt;  // True if PFD is captain side

        std::shared_ptr<PFDData> pfd_data;

        mt_cairo_render_t* render;
        cairo_font_face_t* font_face;
        vect2_t pos;
        vect2_t size;
        int spd_mode;
        int roll_mode;
        int pitch_mode;
        double frame_rate;


        void draw_ap_fd(cairo_t* cr);

        void draw_fma(cairo_t* cr);

        void draw_fma_spd(cairo_t* cr);

        void draw_fma_roll(cairo_t* cr);

        void draw_fma_pitch(cairo_t* cr);

        void draw_test(cairo_t* cr);

        void refresh_screen(cairo_t* cr);
    };
};
