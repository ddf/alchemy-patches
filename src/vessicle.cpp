/**
 * Copyright 2026 Damien Quartz
 * 
 * Firmware for testing various vessicle / vessl classes in isolation.
 */

#include "daisy_seed.h"
#include "alchemy/hw/alchemy_lab.h"
#include "alchemy/surface/control_loop.h"
#include "alchemy/surface/cv_matrix.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/pager.h"
#include "alchemy/surface/param_lock.h"
#include "alchemy/surface/presets.h"
#include "alchemy/surface/settings.h"
#include "alchemy/surface/virtual_knob.h"

#include "vessicle_dsp.h"
#include "vessicle_palette.h"

using namespace alchemy;
using namespace vessicle;
using namespace vessicle_dsp;

/* We define each knob's curve and LED Ring animation.  CV routing lives
 * in the CvMatrix declaration below; declaring it once at the matrix
 * level keeps the knob declarations purely about the knob.  */

/* Page 1 */

static VirtualKnob param_a = VirtualKnob(0, "Parameter A")
  .Linear(0, 1.f)
  .Ring(Level(lime_palette.color, FillAnim::Pulse));

static VirtualKnob param_b = VirtualKnob(2, "Parameter B")
  .Linear(0, 1.f)
  .Ring(Level(lime_palette.color, FillAnim::Pulse));

static VirtualKnob param_c = VirtualKnob(4, "Parameter C")
  .Linear(0, 1.f)
  .Ring(Level(lime_palette.color, FillAnim::Pulse));

static VirtualKnob param_d = VirtualKnob(1, "Parameter D")
  .Linear(0, 1.f)
  .Ring(Level(lime_palette.color, FillAnim::Pulse));

static VirtualKnob param_e = VirtualKnob(3, "Parameter E")
  .Linear(0, 1.f)
  .Ring(Level(lime_palette.color, FillAnim::Pulse));

static VirtualKnob param_f = VirtualKnob(5, "Parameter F")
  .Linear(0, 1.f)
  .Ring(Level(lime_palette.color, FillAnim::Pulse));

// /* Page 2 */
// static VirtualKnob r_hi_level = VirtualKnob(0, "Hi Level")
//     .Linear(-kGainMaxDb, +kGainMaxDb)
//     .Ring(Bipolar(kRightPalette.hi.level_pos,
//                   kRightPalette.hi.level_neg,
//                   kRightPalette.hi.level_center));

// static VirtualKnob r_hi_freq = VirtualKnob(1, "Hi Freq")
//     .Exp(1000.f, 16000.f)
//     .Ring(Level(kRightPalette.hi.freq, FillAnim::Pulse));

// static VirtualKnob r_mid_level = VirtualKnob(2, "Mid Level")
//     .Linear(-kGainMaxDb, +kGainMaxDb)
//     .Ring(Bipolar(kRightPalette.mid.level_pos,
//                   kRightPalette.mid.level_neg,
//                   kRightPalette.mid.level_center));

// static VirtualKnob r_mid_freq = VirtualKnob(3, "Mid Freq")
//     .Exp(200.f, 5000.f)
//     .Ring(Level(kRightPalette.mid.freq, FillAnim::Ripple));

// static VirtualKnob r_lo_level = VirtualKnob(4, "Lo Level")
//     .Linear(-kGainMaxDb, +kGainMaxDb)
//     .Ring(Bipolar(kRightPalette.lo.level_pos,
//                   kRightPalette.lo.level_neg,
//                   kRightPalette.lo.level_center));

// static VirtualKnob r_lo_freq = VirtualKnob(5, "Lo Freq")
//     .Exp(60.f, 600.f)
//     .Ring(Level(kRightPalette.lo.freq, FillAnim::Pulse));

// /* Bind knobs to page, declaration format matches arrangement on the device */
static Page page_one  = Page(0).Knobs(
  param_a, param_d,
  param_b, param_e,
  param_c, param_f
);

// static Page right_page = Page(1).Knobs(r_hi_level, r_hi_freq,
//                                        r_mid_level, r_mid_freq,
//                                        r_lo_level, r_lo_freq);

constexpr uint8_t page_count = 1;

/* Get our SDK surfaces and opt in to everything */
static AlchemyLab                        hw;
static ControlLoop                       loop    (hw);
static Pager                             pager   (hw.buttons[0], page_count, kNumPots);
static ParamLock<page_count * kNumPots>  locks   (hw.buttons[0], pager);
static Presets                           presets (hw.seed.qspi);
static Settings                          settings(hw, &pager);
static CvMatrix                          cv_matrix(kNumCvInputs);

/* summed CV+knob values → DSP each frame */
static void UpdateParams()
{
  SetParameter(Parameter::A, param_a.Value());
  SetParameter(Parameter::B, param_b.Value());
  SetParameter(Parameter::C, param_c.Value());
  SetParameter(Parameter::D, param_d.Value());
  SetParameter(Parameter::E, param_e.Value());
  SetParameter(Parameter::F, param_f.Value());

  Update();
}

int main()
{
    hw.Init(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ, 128);
    Init(hw.SampleRate());

    /* CV routing.  A static layout is just setting each channel once. */
    cv_matrix.Jack(0).To(param_a);
    cv_matrix.Jack(1).To(param_b);
    cv_matrix.Jack(2).To(param_c);
    cv_matrix.Jack(3).To(param_d);
    cv_matrix.Jack(4).To(param_e);
    cv_matrix.Jack(5).To(param_f);

    /* Opting into default settings gestures and controls.*/
    settings.UseBrightness();
    settings.UsePresets(presets);

    /* Preset payload — every Serializable surface gets walked on Save/Load. */
    presets.Manage(pager);
    presets.Manage(locks);
    presets.Manage(settings);
    presets.Init();
    presets.BootLoad();

    UpdateParams();
    hw.StartAudio(vessicle_dsp::Process);

    /* ControlLoop is a thin, opt-in driver for the canonical control-rate frame.
     * If desired, you can unroll and modify. */
    loop.Use(pager)
        .Use(locks)
        .Use(settings)
        .Use(cv_matrix)
        .Use(page_one)
        .OnFrame(UpdateParams);

    for (;;) loop.Tick();
}
