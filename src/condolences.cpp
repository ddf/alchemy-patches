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
 *  @todo setup CV routing
 *  @todo use clip indicator
 *  @todo animate LEDs to give some indication of the contents of the transformed spectrum
 *  @todo implement Help documentation
 * 
 * Maybe and/or later:
 *  @todo generated audio feedback path
 */

Manual thee_manual = Manual();

/////////////////////////////////////////////////////////////////////////////
// Knobs  
VirtualKnob vk_mix_dry = VirtualKnob(kPotBottomLeft, "Dry")
  .Ident("mix.dry")
  .Linear(0.f, 1.f)
  .Ring(Level(vibe_palette.active.rgb));

VirtualKnob vk_mix_wet = VirtualKnob(kPotBottomRight, "Wet")
  .Ident("mix.wet")
  .Linear(0.f, 1.f)
  .Ring(Level(vibe_palette.active.rgb));

///////////////////////////////////////////////////////////////////////
// Skew Knobs
VirtualKnob vk_density_skew = VirtualKnob(kPotTopLeft, "Perception Skew")
  .Ident("percept.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));

VirtualKnob vk_spread_skew = VirtualKnob(kPotTopRight, "Focus Skew")
  .Ident("focus.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));
    
VirtualKnob vk_sensitivity_skew = VirtualKnob(kPotMiddleLeft, "Empathy Skew")
  .Ident("sensi.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));
    
VirtualKnob vk_decay_skew = VirtualKnob(kPotMiddleRight, "Sympathy Skew")
  .Ident("sympa.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));
    
VirtualKnob vk_shift_skew = VirtualKnob(kPotTopLeft, "Transpose Skew")
  .Ident("trans.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_warp_skew = VirtualKnob(kPotTopRight, "Warp Skew")
  .Ident("warp.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_melt_skew = VirtualKnob(kPotMiddleLeft, "Melt Skew")
  .Ident("melt.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_smear_skew = VirtualKnob(kPotMiddleRight, "Smear Skew")
  .Ident("smear.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_ripple_skew = VirtualKnob(kPotBottomLeft, "Sizzle Skew")
  .Ident("sizzle.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));
    
VirtualKnob vk_motion_skew = VirtualKnob(kPotBottomRight, "Emote Skew")
  .Ident("emote.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

/////////////////////////////////////////////////////////////////////////
// Param Knobs which get skewed
VirtualKnob vk_density = VirtualKnob(kPotTopLeft, "Perception")
  .Ident("percept.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_density_skew));

VirtualKnob vk_spread = VirtualKnob(kPotTopRight, "Focus")
  .Ident("focus.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_spread_skew));

VirtualKnob vk_sensitivity = VirtualKnob(kPotMiddleLeft, "Empathy")
  .Ident("sensi.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_sensitivity_skew));

// in seconds, sensible minimum value depends on spectrum size and sample rate
VirtualKnob vk_decay = VirtualKnob(kPotMiddleRight, "Sympathy")
  .Ident("sympa.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_decay_skew));
    
VirtualKnob vk_shift = VirtualKnob(kPotTopLeft, "Transpose")
  .Ident("trans.both")
  .Linear(-1.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_shift_skew));
    
VirtualKnob vk_warp = VirtualKnob(kPotTopRight, "Warp")
  .Ident("warp.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_warp_skew));
    
VirtualKnob vk_melt = VirtualKnob(kPotMiddleLeft, "Melt")
  .Ident("melt.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_melt_skew));
    
VirtualKnob vk_smear = VirtualKnob(kPotMiddleRight, "Smear")
  .Ident("smear.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_smear_skew));
    
VirtualKnob vk_ripple = VirtualKnob(kPotBottomLeft, "Sizzle")
  .Ident("sizzle.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_ripple_skew));
    
VirtualKnob vk_motion = VirtualKnob(kPotBottomRight, "Emote")
  .Ident("emote.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_motion_skew));

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
Profiler                            profiler(hw);
CvMatrix                            cv_matrix(kNumCvInputs);
hostlink::Host                      host(presets, "condolences", "Condolences", "0.9.2", "ff54a749e877014d4dc6d808103f1259914b5413");

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
    float spread_l  = math::lerp(config::settings::focus_min.Value(), config::settings::focus_max.Value(), stl);
    float spread_r  = math::lerp(config::settings::focus_min.Value(), config::settings::focus_max.Value(), str);

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
} // namespace condolences

using namespace condolences;

int main()
{
    // set block size exactly equal to the overlap for synthesis.
    // this should mean we do exactly the same amount of work (generally speaking), every block.
    vessl::size_t block_size = dsp::GetBlockSize();
    hw.Init(daisy::SaiHandle::Config::SampleRate::SAI_32KHZ, block_size);
    dsp::Init(hw.SampleRate());

    /* Drive every switchable jack as a CV output (J3..J8). */
    // for (uint8_t j = 0; j < kNumCvInputs; ++j)
    // {
    //   hw.cv_jacks[j].EnableCvOutput();
    // }

    /* CV routing.  A static layout is just setting each channel once. */
    // cv_matrix.Jack(0).To(l_hi_level);
    // cv_matrix.Jack(1).To(l_hi_freq);
    // cv_matrix.Jack(2).To(l_mid_level);
    // cv_matrix.Jack(3).To(l_mid_freq);
    // cv_matrix.Jack(4).To(l_lo_level);
    // cv_matrix.Jack(5).To(l_lo_freq);

    manual::AttachTo(host);
    ConfigureInterface();

    /* Preset payload — every Serializable surface gets walked on Save/Load. Order IS layout! */
    presets.Manage(pager);
    presets.Manage(locks);
    presets.Manage(profiler);
    presets.Manage(settings);
    //presets.Manage(buttons);
    presets.UseNames();

    /* ControlLoop is a thin, opt-in driver for the canonical control-rate frame.
     * If desired, you can unroll and modify. */
    loop.Use(pager)
        .Use(locks)
        .Use(settings)
        //.Use(cv_matrix)
        .Use(vibe_page)
        .Use(vibe_skew_page)
        .Use(rizz_page)
        .Use(rizz_skew_page)
        .Use(host)
        .OnFrame(UpdateParams);

    presets.Init();
    presets.BootLoad();

    UpdateParams();
    profiler.StartAudio(dsp::Process);

    for (;;) loop.Tick();
}
