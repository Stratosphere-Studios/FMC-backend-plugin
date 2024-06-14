#pragma once

#include <acfutils/log.h>
#include <acfutils/font_utils.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <string>
#include <databus.hpp>
#include <cairo_utils.hpp>
#include <chrono>
#include <thread>


namespace StratosphereAvionics
{
    constexpr vect3_t GREEN = {0, 1, 0};

    constexpr XPLMDrawingPhase PFD_DRAW_PHASE = xplm_Phase_Gauges;
    constexpr int PFD_DRAW_BEFORE = 0;

    constexpr int PFD_FMA_FONT_SZ = 50;
    constexpr int PFD_FMA_TXT_Y = 62;
    constexpr int PFD_FMA_SPD_TXT_X = 260;
    constexpr int PFD_FMA_ROLL_TXT_X = 515;
    constexpr int PFD_FMA_PITCH_TXT_X = 770;

    constexpr int PFD_FMA_RECT_Y = 66;
    constexpr int PFD_FMA_SPD_RECT_X = 207;
    constexpr int PFD_FMA_ROLL_RECT_X = 483;
    constexpr int PFD_FMA_PITCH_RECT_X = 730;
    constexpr int PFD_FMA_RECT_H = 46;
    constexpr int PFD_FMA_SPD_RECT_W = 269;
    constexpr int PFD_FMA_ROLL_RECT_W = 240;
    constexpr int PFD_FMA_PITCH_RECT_W = 255;

    constexpr double PFD_FMA_DATA_UPDATE_HZ = 5;


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
        std::string fma_thr, fma_roll, fma_pitch;
    };


    class PFDData
    {
    public:
        std::atomic<bool> is_stopped;


        PFDData(std::shared_ptr<XPDataBus::DataBus> db, PFDdrs drs, 
            double ref=PFD_FMA_DATA_UPDATE_HZ);

        void update();

        std::string get_spd_mode();

        std::string get_roll_mode();

        std::string get_pitch_mode();

    private:
        std::string spd_md, roll_md, pitch_md;

        std::shared_ptr<XPDataBus::DataBus> data_bus;
        PFDdrs state_drs;

        double ref_hz;
    };

    class PFD
    {
    public:
        PFD(std::shared_ptr<PFDData> data, cairo_font_face_t* ff, 
            vect2_t p, vect2_t sz, double fps);

        void update_screen();

        void perform_cairo_drawing(cairo_t* cr);

        void destroy();

    private:
        std::shared_ptr<PFDData> pfd_data;

        mt_cairo_render_t* render;
        cairo_font_face_t* font_face;
        vect2_t pos;
        vect2_t size;
        int spd_mode;
        int roll_mode;
        int pitch_mode;
        double frame_rate;


        void draw_fma(cairo_t* cr);

        void draw_fma_spd(cairo_t* cr);

        void refresh_screen(cairo_t* cr);
    };
};
