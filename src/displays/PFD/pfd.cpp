/*
	This project is licensed under
	Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License (CC BY-NC-SA 4.0).

	A SUMMARY OF THIS LICENSE CAN BE FOUND HERE: https://creativecommons.org/licenses/by-nc-sa/4.0/

	This source file contains definitions of classes, functions, etc 
	used in the PFD implementation. Author: discord/bruh4096#4512(Tim G.)
*/


#include "pfd.hpp"


namespace StratosphereAvionics
{
    inline std::string get_at_mode_txt(ATModes mode)
    {
        switch(mode)
        {
        case AT_MODE_OFF:
            return "";
        case AT_MODE_IAS_HOLD:
            return "SPD";
        case AT_MODE_RETARD:
            return "IDLE";
        case AT_MODE_HOLD:
            return "HOLD";
        case AT_MODE_THR_REF:
            return "THR REF";
        default:
            return "THR";
        }
    }

    inline std::string get_roll_mode_txt(RollModes mode)
    {
        switch(mode)
        {
        case ROLL_MODE_HDG_HOLD:
            return "HDG HOLD";
        case ROLL_MODE_HDG_SEL:
            return "HDG SEL";
        case AT_MODE_HOLD:
            return "TRK HOLD";
        case AT_MODE_THR_REF:
            return "TRK SEL";
        default:
            return "";
        }
    }

    inline std::string get_pitch_mode_txt(PitchModes mode)
    {
        switch(mode)
        {
        case PITCH_MODE_OFF:
            return "";
        case PITCH_MODE_VSHOLD:
            return "V/S";
        case PITCH_MODE_FPAHOLD:
            return "FPA";
        case PITCH_MODE_ALTHOLD:
            return "ALT";
        default:
            return "FLC SPD";
        }
    }
    

    void pfd_display_render_cb(cairo_t *cr, unsigned w, unsigned h, void *data)
    {
        UNUSED(w);
        UNUSED(h);

        PFD* dsp = reinterpret_cast<PFD*>(data);
        dsp->perform_cairo_drawing(cr);
    }

    int pfd_draw_loop(XPLMDrawingPhase phase, int is_before, void *refcon)
    {
        UNUSED(phase);
        UNUSED(is_before);

        PFD* dsp = reinterpret_cast<PFD*>(refcon);
        dsp->update_screen();
        return 1;
    }


    // PFDData member functions:
    // Public member functions:

    PFDData::PFDData(std::shared_ptr<XPDataBus::DataBus> db, PFDdrs drs, 
        cairo_utils::test_drs *tst, double ref)
    {
        test_drs = tst;

        data_bus = db;
        state_drs = drs;
        ref_hz = ref;

        ap_dir_cap = "";
        ap_dir_fo = "";
        spd_md = "";
        roll_md = "";
        pitch_md = "";

        fd_cap_time = -PFD_FMA_RECT_SHOW_SEC;
        fd_fo_time = -PFD_FMA_RECT_SHOW_SEC;
        spd_time = -PFD_FMA_RECT_SHOW_SEC;
        roll_time = -PFD_FMA_RECT_SHOW_SEC;
        pitch_time = -PFD_FMA_RECT_SHOW_SEC;

        tmr = new libtime::SteadyTimer();

        is_stopped.store(false, std::memory_order_relaxed);
    }

    void PFDData::update()
    {
        while(!is_stopped.load(std::memory_order_relaxed))
        {
            update_test();

            update_ap_fd();

            int curr_spd_md = data_bus->get_datai(state_drs.fma_thr);
            int curr_roll_md = data_bus->get_datai(state_drs.fma_roll);
            int curr_pitch_md = data_bus->get_datai(state_drs.fma_pitch);

            std::string curr_spd = get_at_mode_txt(ATModes(curr_spd_md));
            std::string curr_roll = get_roll_mode_txt(RollModes(curr_roll_md));
            std::string curr_pitch = get_pitch_mode_txt(PitchModes(curr_pitch_md));

            update_param(curr_spd, spd_md, &spd_time);
            update_param(curr_roll, roll_md, &roll_time);
            update_param(curr_pitch, pitch_md, &pitch_time);
            
            spd_md = curr_spd;
            roll_md = curr_roll;
            pitch_md = curr_pitch;

            std::this_thread::sleep_for(std::chrono::milliseconds(int(1000 / ref_hz)));
        }
    }

    void PFDData::update_test()
    {
        if(test_drs != nullptr)
        {
            test_pos.x = data_bus->get_datad(test_drs->x);
            test_pos.y = data_bus->get_datad(test_drs->y);
            test_sz.x = data_bus->get_datad(test_drs->w);
            test_sz.y = data_bus->get_datad(test_drs->h);
            test_rad = data_bus->get_datad(test_drs->radius);
            test_thick = data_bus->get_datad(test_drs->l_thick);
        }
    }

    std::string PFDData::get_capt_ap_fd()
    {
        return ap_dir_cap;
    }

    std::string PFDData::get_fo_ap_fd()
    {
        return ap_dir_fo;
    }

    std::string PFDData::get_spd_mode()
    {
        return spd_md;
    }

    std::string PFDData::get_roll_mode()
    {
        return roll_md;
    }

    std::string PFDData::get_pitch_mode()
    {
        return pitch_md;
    }

    bool PFDData::flt_dir_capt_changed()
    {
        return tmr->get_curr_time() < (fd_cap_time + PFD_FMA_RECT_SHOW_SEC);
    }

    bool PFDData::flt_dir_fo_changed()
    {
        return tmr->get_curr_time() < (fd_fo_time + PFD_FMA_RECT_SHOW_SEC);
    }

    bool PFDData::spd_changed()
    {
        return tmr->get_curr_time() < (spd_time + PFD_FMA_RECT_SHOW_SEC);
    }

    bool PFDData::roll_changed()
    {
        return tmr->get_curr_time() < (roll_time + PFD_FMA_RECT_SHOW_SEC);
    }

    bool PFDData::pitch_changed()
    {
        return tmr->get_curr_time() < (pitch_time + PFD_FMA_RECT_SHOW_SEC);
    }

    void PFDData::destroy()
    {
        delete tmr;
    }

    // Private member functions:

    std::string PFDData::get_ap_fd_txt(int ap_on, int fd_on)
    {
        if(fd_on)
        {
            return FLT_DIR_TXT;
        }
        else if(ap_on)
        {
            return AP_TXT;
        }
        return "";
    }

    void PFDData::update_ap_fd()
    {
        int curr_ap = data_bus->get_datai(state_drs.ap_eng);
        int curr_fd_cap = data_bus->get_datai(state_drs.flt_dir_capt);
        int curr_fd_fo = data_bus->get_datai(state_drs.flt_dir_fo);

        std::string txt_cap = get_ap_fd_txt(curr_ap, curr_fd_cap);
        std::string txt_fo = get_ap_fd_txt(curr_ap, curr_fd_fo);
        
        update_param(txt_cap, ap_dir_cap, &fd_cap_time);
        update_param(txt_fo, ap_dir_fo, &fd_fo_time);

        ap_dir_cap = txt_cap;
        ap_dir_fo = txt_fo;
    }

    void PFDData::update_param(std::string& curr, std::string& prev, double *out)
    {
        if(curr != prev && curr != "")
        {
            *out = tmr->get_curr_time();
        }
    }

    // PFD member functions:
    // Public member functions:

    PFD::PFD(std::shared_ptr<PFDData> data, cairo_font_face_t* ff, 
        vect2_t p, vect2_t sz, double fps, bool capt)
    {
        is_capt = capt;

        pfd_data = data;

        font_face = ff;
        pos = p;
        size = sz;
        frame_rate = fps;

        render = mt_cairo_render_init(
            size.x, size.y,           // The size of our renderer/display
            fps,
            NULL, pfd_display_render_cb, NULL,  // callback. init, render, fini. only render is required
            this                            // arbitrary data pointer, passed to callbacks
        );

        XPLMRegisterDrawCallback(pfd_draw_loop, PFD_DRAW_PHASE, PFD_DRAW_BEFORE, this);
    }

    void PFD::update_screen()
    {
        mt_cairo_render_draw(render, pos, size);
    }

    void PFD::perform_cairo_drawing(cairo_t* cr)
    {
        refresh_screen(cr);

        draw_fma(cr);
        draw_ap_fd(cr);
    }

    void PFD::destroy()
    {
        XPLMUnregisterDrawCallback(pfd_draw_loop, PFD_DRAW_PHASE, PFD_DRAW_BEFORE, this);
        mt_cairo_render_fini(render);
        render = nullptr;
    }

    // Private member functions:

    void PFD::draw_ap_fd(cairo_t* cr)
    {
        std::string curr_txt = "";
        bool is_chng = false;

        if(is_capt)
        {
            curr_txt = pfd_data->get_capt_ap_fd();
            is_chng = pfd_data->flt_dir_capt_changed();
        }
        else
        {
            curr_txt = pfd_data->get_fo_ap_fd();
            is_chng = pfd_data->flt_dir_fo_changed();
        }

        if(curr_txt != "")
        {
            cairo_utils::draw_centered_text(cr, font_face, curr_txt, 
                {PFD_FMA_AP_TXT_X, PFD_FMA_AP_TXT_Y}, GREEN, PFD_FMA_AP_FONT_SZ);
            
            if(is_chng)
            {
                cairo_utils::draw_rect(cr, {PFD_FMA_AP_RECT_X, PFD_FMA_AP_RECT_Y}, 
                    {PFD_FMA_AP_RECT_W, PFD_FMA_AP_RECT_H}, GREEN, PFD_FMA_AP_RECT_LINE_SZ);
            }
        }
    }

    void PFD::draw_fma(cairo_t* cr)
    {
        draw_fma_spd(cr);
        draw_fma_roll(cr);
        draw_fma_pitch(cr);

        draw_test(cr);
    }

    void PFD::draw_fma_spd(cairo_t* cr)
    {
        std::string curr_at_mode_str = pfd_data->get_spd_mode();

        if(curr_at_mode_str != "")
        {
            cairo_utils::draw_centered_text(cr, font_face, curr_at_mode_str, 
                {PFD_FMA_SPD_TXT_X, PFD_FMA_TXT_Y}, GREEN, PFD_FMA_FONT_SZ);

            bool mode_changed = pfd_data->spd_changed();
            if(mode_changed)
            {
                cairo_utils::draw_rect(cr, {PFD_FMA_SPD_RECT_X, PFD_FMA_RECT_Y}, 
                    {PFD_FMA_SPD_RECT_W, PFD_FMA_RECT_H}, GREEN, PFD_FMA_RECT_LINE_SZ);
            }
        }   
    }

    void PFD::draw_fma_roll(cairo_t* cr)
    {
        std::string curr_roll_mode_str = pfd_data->get_roll_mode();

        if(curr_roll_mode_str != "")
        {
            cairo_utils::draw_centered_text(cr, font_face, curr_roll_mode_str, 
                {PFD_FMA_ROLL_TXT_X, PFD_FMA_TXT_Y}, GREEN, PFD_FMA_FONT_SZ);

            bool mode_changed = pfd_data->roll_changed();
            if(mode_changed)
            {
                cairo_utils::draw_rect(cr, {PFD_FMA_ROLL_RECT_X, PFD_FMA_RECT_Y}, 
                    {PFD_FMA_ROLL_RECT_W, PFD_FMA_RECT_H}, GREEN, PFD_FMA_RECT_LINE_SZ);
            }
        }   
    }

    void PFD::draw_fma_pitch(cairo_t* cr)
    {
        std::string curr_pitch_mode_str = pfd_data->get_pitch_mode();

        if(curr_pitch_mode_str != "")
        {
            cairo_utils::draw_centered_text(cr, font_face, curr_pitch_mode_str, 
                {PFD_FMA_PITCH_TXT_X, PFD_FMA_TXT_Y}, GREEN, PFD_FMA_FONT_SZ);

            bool mode_changed = pfd_data->pitch_changed();
            if(mode_changed)
            {
                cairo_utils::draw_rect(cr, {PFD_FMA_PITCH_RECT_X, PFD_FMA_RECT_Y}, 
                    {PFD_FMA_PITCH_RECT_W, PFD_FMA_RECT_H}, GREEN, 3);
            }
        }   
    }

    void PFD::draw_test(cairo_t* cr)
    {
        if(pfd_data->test_drs)
        {
            cairo_utils::draw_rounded_rect(cr, pfd_data->test_pos, pfd_data->test_sz, 
                pfd_data->test_rad, GREEN, pfd_data->test_thick);
        }
    }

    void PFD::refresh_screen(cairo_t* cr)
    {
        cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cr);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    }
} // namespace StratosphereAvionics
