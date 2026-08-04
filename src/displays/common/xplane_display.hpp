#pragma once

#include <acfutils/mt_cairo_render.h>
#include <memory>
#include <utility>

#include <XPLMDisplay.h>

namespace StratosphereAvionics {

constexpr XPLMDrawingPhase kDefaultDisplayDrawPhase = xplm_Phase_Gauges;
constexpr int kDefaultDisplayDrawBefore = 0;

class DisplayBase {
public:
  virtual std::pair<double, double> GetDrawPos() const = 0;

  virtual std::pair<double, double> GetDrawSize() const = 0;

  virtual void Draw(cairo_t* cr) = 0;

  virtual ~DisplayBase() = default;
};

template<typename T>
class SpecificDisplay final : public DisplayBase {
  T obj_;
public:
  template<typename ... Args>
  SpecificDisplay(Args&&... args) : obj_{std::forward<Args>(args)...} {}

  std::pair<double, double> GetDrawPos() const override {
    return obj_.GetDrawPos();
  }

  std::pair<double, double> GetDrawSize() const override {
    return obj_.GetDrawSize();
  }

  void Draw(cairo_t* cr) override {
    obj_.Draw(cr);
  }
};

class XplaneDisplay final {
  vect2_t draw_pos_;
  vect2_t draw_sz_;
  XPLMDrawingPhase draw_phase_;
  int want_before_ = 0;
  std::unique_ptr<DisplayBase> ptr_;
  mt_cairo_render_t* render_;

public:
  static int XplaneDrawCb(
    XPLMDrawingPhase phase, int is_before, void* refcon) {
    UNUSED(phase);
    UNUSED(is_before);
    XplaneDisplay* disp = reinterpret_cast<XplaneDisplay*>(refcon);
    disp->DoXplaneDrawing();
    return 1;
  }

  static void CairoDrawCb(cairo_t *cr, unsigned w, unsigned h, void* data) {
    UNUSED(w);
    UNUSED(h);
    XplaneDisplay* disp = reinterpret_cast<XplaneDisplay*>(data);
    disp->DoCairoDrawing(cr);
  }

  template<typename T>
  XplaneDisplay(double cairo_fps, 
    XPLMDrawingPhase p_draw, int want_before, T&& obj) :
    draw_phase_{p_draw}, want_before_{want_before}, 
    ptr_{new SpecificDisplay<T>{std::move(obj)}} {
    std::pair<double, double> pos = ptr_->GetDrawPos();
    std::pair<double, double> sz = ptr_->GetDrawSize();
    draw_pos_ = {pos.first, pos.second};
    draw_sz_ = {sz.first, sz.second};
    render_ = mt_cairo_render_init(
            draw_sz_.x, draw_sz_.y,           // The size of our renderer/display
            cairo_fps,
            NULL, CairoDrawCb, NULL,  // callback. init, render, fini. only render is required
            this                            // arbitrary data pointer, passed to callbacks
        );
    XPLMRegisterDrawCallback(XplaneDrawCb, draw_phase_, want_before_, this);
  }

  void DoCairoDrawing(cairo_t *cr) {
    ptr_->Draw(cr);
  }

  void DoXplaneDrawing() {
    mt_cairo_render_draw(render_, draw_pos_, draw_sz_);
  }

  void Destroy() {
    XPLMUnregisterDrawCallback(XplaneDrawCb, draw_phase_, want_before_, this);
    mt_cairo_render_fini(render_);
    render_ = nullptr;
    ptr_.release();
  }
};

template<typename T, typename ... Args>
XplaneDisplay* CreateGenericXplaneDisplay(double fps, Args&&... args) {
  return new XplaneDisplay{fps, kDefaultDisplayDrawPhase, kDefaultDisplayDrawBefore, 
    T{std::forward<Args>(args)...}};
}
} // namespace StratosphereAvionics
