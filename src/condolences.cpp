/**
Condolences for Alchemy Lab
Copyright (C) 2026 Damien Quartz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "daisy_seed.h"
#include "alchemy/hw/alchemy_lab.h"
#include "alchemy/host_link/host.h"
#include "alchemy/surface/control_loop.h"
#include "alchemy/surface/cv_matrix.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/pager.h"
#include "alchemy/surface/param_lock.h"
#include "alchemy/surface/presets.h"
#include "alchemy/surface/settings.h"
#include "alchemy/surface/virtual_knob.h"
#include "alchemy/surface/virtual_button.h"
#include "alchemy/surface/button_bank.h"

#include "attributes.h"
#include "profiler.h"
#include "condolences_cfg.h"
#include "condolences_man.h"
#include "condolences_gui.h"
#include "condolences_dsp.h"
#include "vessl/vessl.h"
#include <stdio.h>

using namespace vessl;
using namespace alchemy;

namespace condolences
{

/**
 * Definitely:
 *  @todo define buttons to get help text in the web interface
 * 
 * Maybe and/or later:
 *  @todo input gain and output gain in Config Settings
 *  @todo use clip indicator
 *  @todo generated audio feedback path
 *  @todo animate LEDs to give some indication of the contents of the transformed spectrum
 */

Manual thee_manual = Manual();

/////////////////////////////////////////////////////////////////////////////
// Knobs  
VirtualKnob vk_mix_dry = VirtualKnob(kPotBottomLeft, param::name[param::Dry])
  .Ident("mix.dry")
  .Linear(0.f, 1.f)
  .Ring(Level(vibe_palette.active.rgb));

VirtualKnob vk_mix_wet = VirtualKnob(kPotBottomRight, param::name[param::Wet])
  .Ident("mix.wet")
  .Linear(0.f, 1.f)
  .Ring(Level(vibe_palette.active.rgb));

///////////////////////////////////////////////////////////////////////
// Skew Knobs
VirtualKnob vk_density_skew = VirtualKnob(kPotTopLeft, param::name[param::PerceptionSkew])
  .Ident("percept.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));

VirtualKnob vk_spread_skew = VirtualKnob(kPotTopRight, param::name[param::FocusSkew])
  .Ident("focus.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));
    
VirtualKnob vk_sensitivity_skew = VirtualKnob(kPotMiddleLeft, param::name[param::EmpathySkew])
  .Ident("sensi.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));
    
VirtualKnob vk_decay_skew = VirtualKnob(kPotMiddleRight, param::name[param::SympathySkew])
  .Ident("sympa.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));
    
VirtualKnob vk_shift_skew = VirtualKnob(kPotTopLeft, param::name[param::TransposeSkew])
  .Ident("trans.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_warp_skew = VirtualKnob(kPotTopRight, param::name[param::WarpSkew])
  .Ident("warp.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_melt_skew = VirtualKnob(kPotMiddleLeft, param::name[param::MeltSkew])
  .Ident("melt.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_smear_skew = VirtualKnob(kPotMiddleRight, param::name[param::SmearSkew])
  .Ident("smear.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_ripple_skew = VirtualKnob(kPotBottomLeft, param::name[param::SizzleSkew])
  .Ident("sizzle.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_motion_skew = VirtualKnob(kPotBottomRight, param::name[param::EmoteSkew])
  .Ident("emote.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

/////////////////////////////////////////////////////////////////////////
// Param Knobs which get skewed
VirtualKnob vk_density = VirtualKnob(kPotTopLeft, param::name[param::Perception])
  .Ident("percept.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_density_skew));

VirtualKnob vk_spread = VirtualKnob(kPotTopRight, param::name[param::Focus])
  .Ident("focus.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_spread_skew));

VirtualKnob vk_sensitivity = VirtualKnob(kPotMiddleLeft, param::name[param::Empathy])
  .Ident("sensi.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_sensitivity_skew));

// in seconds, sensible minimum value depends on spectrum size and sample rate
VirtualKnob vk_decay = VirtualKnob(kPotMiddleRight, param::name[param::Sympathy])
  .Ident("sympa.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_decay_skew));
    
VirtualKnob vk_shift = VirtualKnob(kPotTopLeft, param::name[param::Transpose])
  .Ident("trans.both")
  .Linear(-1.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_shift_skew));
    
VirtualKnob vk_warp = VirtualKnob(kPotTopRight, param::name[param::Warp])
  .Ident("warp.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_warp_skew));
    
VirtualKnob vk_melt = VirtualKnob(kPotMiddleLeft, param::name[param::Melt])
  .Ident("melt.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_melt_skew));
    
VirtualKnob vk_smear = VirtualKnob(kPotMiddleRight, param::name[param::Smear])
  .Ident("smear.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_smear_skew));
    
VirtualKnob vk_ripple = VirtualKnob(kPotBottomLeft, param::name[param::Sizzle])
  .Ident("sizzle.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_ripple_skew));
    
VirtualKnob vk_motion = VirtualKnob(kPotBottomRight, param::name[param::Emote])
  .Ident("emote.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_motion_skew));

//////////////////////////////////////////////////////////////////////
// Buttons (defined even though we don't use them so that they can show up in the web programmer)
VirtualButton vb_page_lock  = VirtualButton(kButtonB1, "Page / Lock").Ident("btn.page");
VirtualButton vb_vibe_latch = VirtualButton(kButtonB2, "Vibe <-> Skew").Ident("btn.vibe-skew");
VirtualButton vb_rizz_latch = VirtualButton(kButtonB3, "Rizz <-> Skew").Ident("btn.rizz-skew");
ButtonBank buttons;

//////////////////////////////////////////////////////////////////////
// Pages
enum PageId : uint8_t
{
  kPageVibe, 
  kPageRizz, 
  kPageVibeSkew, 
  kPageRizzSkew,

  kPageCount
};
    
Page vibe_page = Page(kPageVibe)
  .Name("Vibe")
  .Color(vibe_palette.active.hex)
  .Knobs(vk_density, vk_spread, vk_sensitivity, vk_decay, vk_mix_dry, vk_mix_wet);

Page vibe_skew_page = Page(kPageVibeSkew)
  .Name("Vibe Skew")
  .Color(vibe_palette.active.hex)
  .Knobs(vk_density_skew, vk_spread_skew, vk_sensitivity_skew, vk_decay_skew);
    
Page rizz_page = Page(kPageRizz)
  .Name("Rizz")
  .Color(rizz_palette.active.hex)
  .Knobs(vk_shift, vk_warp, vk_melt, vk_smear, vk_ripple, vk_motion);
    
Page rizz_skew_page = Page(kPageRizzSkew)
  .Name("Rizz Skew")
  .Color(rizz_palette.active.hex)
  .Knobs(vk_shift_skew, vk_warp_skew, vk_melt_skew, vk_smear_skew, vk_ripple_skew, vk_motion_skew);

//////////////////////////////////////////////////////////////////////
// Surfaces
static constexpr uint8_t kLockCount = kPageCount*kNumPots;
using LockSettings = LockLength<16, 10, LockStore::Preset>;

/* These are not declared static so condolences_gui.h can reference hw and pager. */
AlchemyLab                          hw;
ControlLoop                         loop    (hw);
Pager                               pager   (kPageCount, kNumPots);
ParamLock<kLockCount, LockSettings> locks   (hw.buttons[kButtonB1], pager);
Presets                             presets (hw.seed.qspi);
Settings                            settings(hw, &pager);
#ifdef PROFILE_ENABLED
Profiler                            profiler(hw);
#endif
CvMatrix                            cv_matrix(kNumCvInputs);
hostlink::Host                      host(presets, "condolences", "Condolences", "0.9.4", "525c5abedeae6124c159e0f3a6f306e39ffcb62d");

static void ConfigureInterface()
{
  pager.Cycle(hw.buttons[kButtonB1], kPageVibe, kPageRizz)
       .Latch(hw.buttons[kButtonB2], kPageVibe, kPageVibeSkew)
       .Latch(hw.buttons[kButtonB3], kPageRizz, kPageRizzSkew);

  float phys[] = { hw.pots[0].Value(), hw.pots[1].Value(), hw.pots[2].Value(), hw.pots[3].Value(), hw.pots[4].Value(), hw.pots[5].Value() };

  pager.SetStored(kPageVibe, vk_mix_dry.Pot(), 0.5f, phys);
  pager.SetStored(kPageVibe, vk_mix_wet.Pot(), 0.5f, phys);

  pager.SetStored(kPageRizz, vk_warp.Pot(), 0.f, phys);
  pager.SetStored(kPageRizz, vk_melt.Pot(), 0.f, phys);
  pager.SetStored(kPageRizz, vk_smear.Pot(), 0.f, phys);
  pager.SetStored(kPageRizz, vk_ripple.Pot(), 0.f, phys);
  pager.SetStored(kPageRizz, vk_motion.Pot(), 0.f, phys);

  config::Configure(settings, presets);
}

static void UpdateRouting(uint32_t t_ms)
{
  cv_matrix.Jack(0).To(config::GetCvDest(0)).Atten(config::settings::cv_a_level.Value());
  cv_matrix.Jack(1).To(config::GetCvDest(1)).Atten(config::settings::cv_b_level.Value());
  cv_matrix.Jack(2).To(config::GetCvDest(2)).Atten(config::settings::cv_c_level.Value());
  cv_matrix.Jack(3).To(config::GetCvDest(3)).Atten(config::settings::cv_d_level.Value());
  cv_matrix.Jack(4).To(config::GetCvDest(4)).Atten(config::settings::cv_e_level.Value());
  cv_matrix.Jack(5).To(config::GetCvDest(5)).Atten(config::settings::cv_f_level.Value());
}

/* summed CV+knob values → DSP each frame */
static void UpdateParams()
{
  const float dmin = math::lerp(config::band_density_min, config::band_density_max, config::settings::perception_min.Value());
  const float dmax = math::lerp(config::band_density_min, config::band_density_max, config::settings::perception_max.Value());
  {
    // decay
    float decsk = GetSkewValue(vk_decay_skew);
    float decl  = math::constrain(vk_decay.Value() - decsk, 0.f, 1.f);
    float decr  = math::constrain(vk_decay.Value() + decsk, 0.f, 1.f);

    // density
    float dsk = GetSkewValue(vk_density_skew);
    float dtl = math::constrain(vk_density.Value() - dsk, 0.f, 1.f);
    float dtr = math::constrain(vk_density.Value() + dsk, 0.f, 1.f);

    // spread
    float ssk = GetSkewValue(vk_spread_skew);
    float stl = math::constrain(vk_spread.Value() - ssk, 0.f, 1.f);
    float str = math::constrain(vk_spread.Value() + ssk, 0.f, 1.f);

    float dampngl   = math::interp<math::easing::quad::out>(config::dampi_min, config::dampi_max, decl);
    float dampngr   = math::interp<math::easing::quad::out>(config::dampi_min, config::dampi_max, decr);
    float density_l = math::lerp(dmin, dmax, dtl);
    float density_r = math::lerp(dmin, dmax, dtr);
    float spread_l  = math::lerp(config::focus::min_default, config::focus::max_default, stl);
    float spread_r  = math::lerp(config::focus::min_default, config::focus::max_default, str);

    float sens  = vk_sensitivity.Value();
    float sensk = GetSkewValue(vk_sensitivity_skew);
    float sensl = math::constrain(math::lerp(config::sensi_min, config::sensi_max, sens - sensk), 
                                  config::sensi_min, 
                                  config::sensi_max);

    float sensr = math::constrain(math::lerp(config::sensi_min, config::sensi_max, sens + sensk), 
                                  config::sensi_min, 
                                  config::sensi_max);

    float shft  = vk_shift.Value();
    float shfsk = GetSkewValue(vk_shift_skew);
    
    float warp  = vk_warp.Value();
    float warsk = GetSkewValue(vk_warp_skew);
    
    float smear = vk_smear.Value();
    float smesk = GetSkewValue(vk_smear_skew);

    float smrl  = math::constrain(math::lerp(config::smear_min, config::smear_max, smear - smesk), 
                                  config::smear_min, 
                                  config::smear_max);

    float smrr  = math::constrain(math::lerp(config::smear_min, config::smear_max, smear + smesk), 
                                  config::smear_min, 
                                  config::smear_max);

    float melt  = vk_melt.Value();
    float melsk = GetSkewValue(vk_melt_skew);

    float ripl  = vk_ripple.Value();
    float ripsk = GetSkewValue(vk_ripple_skew);
    float ripdmpl = 1.0f - config::settings::ripple_damp_reduct.Value()*decl;
    float ripdmpr = 1.0f - config::settings::ripple_damp_reduct.Value()*decr;

    float ripbstl = config::settings::ripple_smear_boost.Value()*math::constrain(smear - smesk, 
                                                                                 config::ripple::smear_boost_min, 
                                                                                 config::ripple::smear_boost_max);

    float ripbstr = config::settings::ripple_smear_boost.Value()*math::constrain(smear + smesk, 
                                                                                 config::ripple::smear_boost_min, 
                                                                                 config::ripple::smear_boost_max);

    float ripll = math::constrain(vessl::math::lerp(0.f, (config::settings::ripple_amount_max.Value()+ripbstl)*ripdmpl, ripl - ripsk), 
                                  config::ripple::amount_min, 
                                  config::ripple::amount_max);

    float riplr = math::constrain(vessl::math::lerp(0.f, (config::settings::ripple_amount_max.Value()+ripbstr)*ripdmpr, ripl + ripsk), 
                                  config::ripple::amount_min, 
                                  config::ripple::amount_max);

    float ripdl = math::constrain(config::settings::ripple_lfo_depth.Value() + ripbstl*0.25f, 
                                  config::ripple::depth_min, 
                                  config::ripple::depth_max);

    float ripdr = math::constrain(config::settings::ripple_lfo_depth.Value() + ripbstr*0.25f, 
                                  config::ripple::depth_min, 
                                  config::ripple::depth_max);

    float motn  = vk_motion.Value();
    float motsk = GetSkewValue(vk_motion_skew);

    float motl  = math::constrain(math::lerp(config::motio_min, config::motio_max, motn - motsk), 
                                  config::motio_min, 
                                  config::motio_max);

    float motr  = math::constrain(math::lerp(config::motio_min, config::motio_max, motn + motsk), 
                                  config::motio_min, 
                                  config::motio_max);

    float mixd  = vk_mix_dry.Value();
    float mixw  = vk_mix_wet.Value();

    uint8_t mode = config::settings::mode.Value();
    
    dsp::SetDensity(density_l, density_r);
    dsp::SetDamping(dampngl, dampngr);
    dsp::SetSpread(spread_l, spread_r);
    dsp::SetSensitivity(sensl, sensr);
    dsp::SetShift(shft - shfsk, shft + shfsk);
    dsp::SetSpacing(warp - warsk, warp + warsk);
    dsp::SetSmear(smrl, smrr);
    dsp::SetMelt(melt - melsk, melt + melsk);
    dsp::SetRipple(ripll, riplr, ripdl, ripdr);
    dsp::SetMotion(motl, motr);
    dsp::SetMix(mixd, mixw);
    dsp::SetMode(static_cast<dsp::Mode>(mode));
  }
}

void RenderOverlay(uint32_t t_ms)
{
  hw.leds.SetButtonPair(kButtonB2, vessicle::color::Black.rgb);
  hw.leds.SetButtonPair(kButtonB3, vessicle::color::Black.rgb);
  switch(pager.ActivePage())
  {
    case kPageVibe: hw.leds.SetButton(kButtonB2, LedPanel::ButtonLed::Top, vibe_palette.active.rgb); break;
    case kPageVibeSkew: hw.leds.SetButton(kButtonB2, LedPanel::ButtonLed::Bottom, vibe_palette.active.rgb); break;
    case kPageRizz: hw.leds.SetButton(kButtonB3, LedPanel::ButtonLed::Top, rizz_palette.active.rgb); break;
    case kPageRizzSkew: hw.leds.SetButton(kButtonB3, LedPanel::ButtonLed::Bottom, rizz_palette.active.rgb); break;
  }
}

} // namespace condolences

using namespace condolences;

int main()
{
    // set block size exactly equal to the overlap for synthesis.
    // this should mean we do exactly the same amount of work (generally speaking), every block.
    vessl::size_t block_size = dsp::GetBlockSize();
    hw.Init(daisy::SaiHandle::Config::SampleRate::SAI_32KHZ, block_size);
    dsp::Init(hw.SampleRate());

    ConfigureInterface();
    
    cv_matrix.Jack(0).To(config::GetCvDest(0));
    cv_matrix.Jack(1).To(config::GetCvDest(1));
    cv_matrix.Jack(2).To(config::GetCvDest(2));
    cv_matrix.Jack(3).To(config::GetCvDest(3));
    cv_matrix.Jack(4).To(config::GetCvDest(4));
    cv_matrix.Jack(5).To(config::GetCvDest(5));

    manual::AttachTo(host);
    
    /* Preset payload — every Serializable surface gets walked on Save/Load. Order IS layout! */
    presets.Manage(pager);
    presets.Manage(locks);
  #ifdef PROFILE_ENABLED
    presets.Manage(profiler);
  #endif
    presets.Manage(settings);
    // presets.Manage(buttons);
    presets.UseNames();

    /* ControlLoop is a thin, opt-in driver for the canonical control-rate frame.
     * If desired, you can unroll and modify. */
    loop.Use(pager)
        .Use(locks)
        .Use(settings)
        .Use(cv_matrix)
        .Use(vibe_page)
        .Use(vibe_skew_page)
        .Use(rizz_page)
        .Use(rizz_skew_page)
        //.Use(buttons)
        .Use(host)
        .OnPoll(UpdateRouting)
        .OnFrame(UpdateParams)
        .OnRender(RenderOverlay);

    presets.Init();
    presets.BootLoad();

    UpdateParams();
  #ifdef PROFILE_ENABLED
    profiler.StartAudio(dsp::Process);
  #else
    hw.StartAudio(dsp::Process);
  #endif

    for (;;) loop.Tick();
}
