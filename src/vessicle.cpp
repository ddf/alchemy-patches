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
#include "alchemy/host_link/host.h"

#include "attributes.h"
#include "debugger.h"
#include "profiler.h"
#include "vessicle_dsp.h"
#include "vessicle_palette.h"

using namespace alchemy;
using namespace vessicle;

/* We define each knob's curve and LED Ring animation.  CV routing lives
 * in the CvMatrix declaration below; declaring it once at the matrix
 * level keeps the knob declarations purely about the knob.  */

/* Page 1 */


static ALCHEMY_SRAM VirtualKnob param_a = VirtualKnob(0, "Parameter A")
  .Linear(0, 1.f)
  .Ring(Level(vessicle::color::Lime.rgb, FillAnim::Pulse));

static ALCHEMY_SRAM VirtualKnob param_b = VirtualKnob(2, "Parameter B")
  .Linear(0, 1.f)
  .Ring(Level(vessicle::color::Lime.rgb, FillAnim::Pulse));

static ALCHEMY_SRAM VirtualKnob param_c = VirtualKnob(4, "Parameter C")
  .Linear(0, 1.f)
  .Ring(Level(vessicle::color::Lime.rgb, FillAnim::Pulse));

static ALCHEMY_SRAM VirtualKnob param_d = VirtualKnob(1, "Parameter D")
  .Linear(0, 1.f)
  .Ring(Level(vessicle::color::Lime.rgb, FillAnim::Pulse));

static ALCHEMY_SRAM VirtualKnob param_e = VirtualKnob(3, "Parameter E")
  .Linear(0, 1.f)
  .Ring(Level(vessicle::color::Lime.rgb, FillAnim::Pulse));

static ALCHEMY_SRAM VirtualKnob param_f = VirtualKnob(5, "Parameter F")
  .Linear(0, 1.f)
  .Ring(Level(vessicle::color::Lime.rgb, FillAnim::Pulse));

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
static ALCHEMY_SRAM Page page_one  = Page(0).Knobs(
  param_a, param_d,
  param_b, param_e,
  param_c, param_f
);

// static Page right_page = Page(1).Knobs(r_hi_level, r_hi_freq,
//                                        r_mid_level, r_mid_freq,
//                                        r_lo_level, r_lo_freq);

constexpr uint8_t page_count = 1;

/* Get our SDK surfaces and opt in to everything */
static ALCHEMY_SRAM AlchemyLab                        hw;
static ALCHEMY_SRAM ControlLoop                       loop    (hw);
static ALCHEMY_SRAM Pager                             pager   (hw.buttons[0], page_count, kNumPots);
static ALCHEMY_SRAM ParamLock<page_count * kNumPots>  locks   (hw.buttons[0], pager);
static ALCHEMY_SRAM Presets                           presets (hw.seed.qspi);
static ALCHEMY_SRAM Settings                          settings(hw, &pager);
static ALCHEMY_SRAM CvMatrix                          cv_matrix(kNumCvInputs);
static ALCHEMY_SRAM hostlink::Host                    host(presets, "vessicle", "VESSICLE", "0.1.0", "666777");
static ALCHEMY_SRAM Debugger                          debugger;
static ALCHEMY_SRAM Profiler::SettingsPage            profilerSettings;

/* summed CV+knob values → DSP each frame */
static void UpdateParams()
{
  vessicle_dsp::SetParameter(vessicle_dsp::Parameter::A, param_a.Value());
  vessicle_dsp::SetParameter(vessicle_dsp::Parameter::B, param_b.Value());
  vessicle_dsp::SetParameter(vessicle_dsp::Parameter::C, param_c.Value());
  vessicle_dsp::SetParameter(vessicle_dsp::Parameter::D, param_d.Value());
  vessicle_dsp::SetParameter(vessicle_dsp::Parameter::E, param_e.Value());
  vessicle_dsp::SetParameter(vessicle_dsp::Parameter::F, param_f.Value());

  vessicle_dsp::Update();
}

int main()
{
    hw.Init(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ, vessicle_dsp::GetBlockSize());

    vessicle_dsp::Init(hw.SampleRate(), hw.BlockSize());

    Profiler::Init(hw.SampleRate(), hw.BlockSize(), vessicle_dsp::Process);

    // debugger.Var(vessicle_dsp::process_us, "dbg.pus", "Process micro")
    //         .Range(0, 1000000)
    //         .Kind("linear")
    //         .Unit("us");

    // debugger.Var(vessicle_dsp::process_pct, "dbg.ppct", "Process %");

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
    //presets.Manage(debugger);
    presets.Manage(profilerSettings);
    presets.UseNames();


    /* ControlLoop is a thin, opt-in driver for the canonical control-rate frame.
     * If desired, you can unroll and modify. */
    loop.Use(pager)
        .Use(locks)
        .Use(settings)
        .Use(cv_matrix)
        .Use(page_one)
        .Use(host)
        .OnFrame(UpdateParams);

    presets.Init();
    presets.BootLoad();

    UpdateParams();
    hw.StartAudio(Profiler::Process);

    for (;;) loop.Tick();
}
