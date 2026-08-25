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

#include "condolences_dsp.h"
#include "condolences_gui.h"
#include "vessl/vessl.h"
#include <stdio.h>

using namespace alchemy;

/**
 * Definitely:
 *  @todo setup CV routing
 *  @todo use clip indicator
 *  @todo limiting on the output
 *  @todo animate LEDs to give some indication of the contents of the transformed spectrum
 *  @todo implement Help documentation 
 *  @todo add gesture with B2 for adjusting skew from the first page (essentially a shift function)
 *  @todo change skew rendering to use two pips, unique colors for each that blend to the main color when added.
 * 
 * Maybe and/or later:
 *  @todo generated audio feedback path
 *  @todo parameter for smear LFO speed and depth?
 *  @todo parameter for blending between exponential decay and linear decay?
 *  @todo second page could be "sub" parameters of what's on the first page, so:
 *        - density -> spread
 *        - decay -> exp to lin
 *        - warp -> compress (reduce size of mapped to range in the sympathies)
 *        - smear -> LFO speed
 *        - melt -> LFO depth or maybe larger jump to band below?
 *        - mix -> control over crossfade curve 
 */

 
constexpr float band_density_min = condolences::GetDensityMin();
constexpr float band_density_max = condolences::GetDensityMax();

struct DensitySettings : Serializable
{
  static constexpr float band_min_default = (8.f - band_density_min) / (band_density_max - band_density_min);
  static constexpr float band_max_default = (32.f - band_density_min) / (band_density_max - band_density_min);
  static constexpr float spread_min_dafault = 1.0f;
  static constexpr float spread_max_default = 1.0f;

  /* Normalized 0..1; the disp hint maps the readout to 0..2× gain. */
  float band_min = band_min_default;
  float band_max = band_max_default;
  float spread_min = spread_min_dafault;
  float spread_max = spread_max_default;

  size_t SerializedSize() const override { return 4u * sizeof(float); }

  void Serialize(uint8_t* out) const override
  {
    std::memcpy(out + 0, &band_min, 4);
    std::memcpy(out + 4, &band_max, 4);
    std::memcpy(out + 8, &spread_min, 4);
    std::memcpy(out + 12, &spread_max, 4);
  }

  bool Deserialize(const uint8_t* in) override
  {
    std::memcpy(&band_min, in + 0, 4);
    std::memcpy(&band_max, in + 4, 4);
    std::memcpy(&spread_min, in + 8, 4);
    std::memcpy(&spread_max, in + 12, 4);
    return true;
  }

  uint32_t SchemaHash() const override { return 0x54524D32u; /* 'TRM2' */ }

  bool Describe(hostlink::ComponentWriter& w) const override
  {
    w.Label("Density Settings");
    
    char band_disp_json[64];
    sprintf(band_disp_json, "{\"kind\":\"linear\",\"lo\":%d,\"hi\":%d}", 
      static_cast<int>(band_density_min), static_cast<int>(band_density_max));

    bool ok = w.Field("density.min", "Bands Min", 0, hostlink::FieldType::F32, band_min_default, band_disp_json);
    ok &= w.Field("density.max", "Bands Max", 4, hostlink::FieldType::F32, band_max_default, band_disp_json);
    ok &= w.Field("spread.min", "Spread Min", 8, hostlink::FieldType::F32, spread_min_dafault);
    ok &= w.Field("spread.max", "Spread Max", 12, hostlink::FieldType::F32, spread_max_default);

    return ok;
  }
};

constexpr size_t page_count = 2;

/* Get our SDK surfaces and opt in to everything.
 * Not declared static so condolences_gui.h can reference hw and pager.
 */
AlchemyLab                        hw;
ControlLoop                       loop    (hw);
Pager                             pager   (hw.buttons[kButtonB1], page_count, kNumPots);
ParamLock<page_count * kNumPots>  locks   (hw.buttons[kButtonB1], pager);
Presets                           presets (hw.seed.qspi);
Settings                          settings(hw, &pager);
CvMatrix                          cv_matrix(kNumCvInputs);
hostlink::Host                    host(presets, "condolences", "Condolences", "0.1.0", "abcdefg");
DensitySettings                   density_settings;

static VirtualKnob vk_density_l = VirtualKnob(kPotTopLeft, "Density Left")
  .Linear(0.f, 1.f).Ident("density.left")
  .Ring(vibe_spec);

static VirtualKnob vk_density_r = VirtualKnob(kPotTopRight, "Density Right")
  .Linear(0.f, 1.f).Ident("density.right")
  .Ring(vibe_spec);

// in seconds, sensible minimum value depends on spectrum size and sample rate
static VirtualKnob vk_decay_l = VirtualKnob(kPotMiddleLeft, "Decay Left")
  .Exp(0.1f, 10.f).Unit("s").Ident("decay.left")
  .Ring(vibe_spec);

static VirtualKnob vk_decay_r = VirtualKnob(kPotMiddleRight, "Decay Right")
  .Exp(0.1f, 10.f).Unit("s").Ident("decay.right")
  .Ring(vibe_spec);

static VirtualKnob vk_mix_l = VirtualKnob(kPotBottomLeft, "Mix Left")
  .Linear(0.f, 1.f).Ident("mix.left")
  .Ring(vibe_spec);

static VirtualKnob vk_mix_r = VirtualKnob(kPotBottomRight, "Mix Right")
  .Linear(0.f, 1.f).Ident("mix.right")
  .Ring(vibe_spec);

static VirtualKnob vk_warp_l = VirtualKnob(kPotTopLeft, "Warp Left")
  .Linear(0.f, 1.f).Ident("warp.left")
  .Ring(rizz_spec);

static VirtualKnob vk_warp_r = VirtualKnob(kPotTopRight, "Warp Right")
  .Linear(0.f, 1.f).Ident("warp.right")
  .Ring(rizz_spec);

static VirtualKnob vk_smear_l = VirtualKnob(kPotMiddleLeft, "Smear Left")
  .Linear(0.f, 1.f).Ident("smear.left")
  .Ring(rizz_spec);

static VirtualKnob vk_smear_r = VirtualKnob(kPotMiddleRight, "Smear Right")
  .Linear(0.f, 1.f).Ident("smear.right")
  .Ring(rizz_spec);

static VirtualKnob vk_melt_l = VirtualKnob(kPotBottomLeft, "Melt Left")
  .Linear(0.f, 1.f).Ident("melt.left")
  .Ring(rizz_spec);

static VirtualKnob vk_melt_r = VirtualKnob(kPotBottomRight, "Melt Right")
  .Linear(0.f, 1.f).Ident("melt.right")
  .Ring(rizz_spec);

///////////////////////////////////////////////////////////////////////
// Skew Knobs
static VirtualKnob vk_density_skew = VirtualKnob(kPotTopLeft, "Density Skew")
  .Ident("density.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(DrawSkewKnob, &vk_density_skew));

static VirtualKnob vk_decay_skew = VirtualKnob(kPotMiddleLeft, "Decay Skew")
  .Ident("decay.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(DrawSkewKnob, &vk_decay_skew));

static VirtualKnob vk_mix_skew = VirtualKnob(kPotBottomLeft, "Mix Skew")
  .Ident("mix.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(DrawSkewKnob, &vk_mix_skew));

static VirtualKnob vk_warp_skew = VirtualKnob(kPotTopRight, "Warp Skew")
  .Ident("warp.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(DrawSkewKnob, &vk_warp_skew));
  
static VirtualKnob vk_smear_skew = VirtualKnob(kPotMiddleRight, "Smear Skew")
  .Ident("smear.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(DrawSkewKnob, &vk_smear_skew));

static VirtualKnob vk_melt_skew = VirtualKnob(kPotBottomRight, "Melt Skew")
  .Ident("melt.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(DrawSkewKnob, &vk_melt_skew));

/////////////////////////////////////////////////////////////////////////
// Param Knobs which get skewed
static VirtualKnob vk_density = VirtualKnob(kPotTopLeft, "Density")
  .Ident("density.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(DrawKnobWithSkew, &vk_density_skew));

// in seconds, sensible minimum value depends on spectrum size and sample rate
static VirtualKnob vk_decay = VirtualKnob(kPotMiddleLeft, "Decay")
  .Ident("decay.both")
  .Exp(0.1f, 10.f).Unit("s")
  .Ring(Custom(DrawKnobWithSkew, &vk_decay_skew));

static VirtualKnob vk_mix = VirtualKnob(kPotBottomLeft, "Mix")
  .Ident("mix.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(DrawKnobWithSkew, &vk_mix_skew));

static VirtualKnob vk_warp = VirtualKnob(kPotTopRight, "Warp")
  .Ident("warp.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(DrawKnobWithSkew, &vk_warp_skew));
  
static VirtualKnob vk_smear = VirtualKnob(kPotMiddleRight, "Smear")
  .Ident("smear.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(DrawKnobWithSkew, &vk_smear_skew));

static VirtualKnob vk_melt = VirtualKnob(kPotBottomRight, "Melt")
  .Ident("melt.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(DrawKnobWithSkew, &vk_melt_skew));

// /* Bind knobs to page */
static Page left_page  = Page(0).Name("Left")
  .Color(vessicle::palette::Fuschia.active.hex)
  .Knobs(vk_density_l, vk_density_r, vk_decay_l, vk_decay_r, vk_mix_l, vk_mix_r);

static Page right_page = Page(1).Name("Right")
  .Color(vessicle::palette::Lime.active.hex)
  .Knobs(vk_warp_l, vk_warp_r, vk_smear_l, vk_smear_r, vk_melt_l, vk_melt_r);

static Page vibe_page = Page(0).Name("Vibe")
  .Color(vessicle::palette::Fuschia.active.hex)
  .Knobs(vk_density, vk_decay, vk_mix, vk_warp, vk_melt, vk_smear);

static Page rizz_page = Page(1).Name("Rizz")
  .Color(vessicle::palette::Lime.active.hex)
  .Knobs(vk_density_skew, vk_decay_skew, vk_mix_skew, vk_warp_skew, vk_melt_skew, vk_smear_skew);

//////////////////////////////////////////////////////////////////////
// button, button, whose got the button?
static VirtualButton vb_shift = VirtualButton(kButtonB2, "Shift")
  .Ident("btn.shift");

static ButtonBank buttons;

static void OnPoll(uint32_t t_ms)
{
  if (pager.Page() == 0 && IsShiftPressed())
  {
    SetShiftEnabled(true);
    pager.GoToPage(1, loop.Phys());
  }
  else if (pager.Page() == 1 && IsShiftEnabled() && !IsShiftPressed())
  {
    SetShiftEnabled(false);
    pager.GoToPage(0, loop.Phys());
  }

  if (IsShiftEnabled())
  {
    pager.ConsumeButton();
  }
}

/* summed CV+knob values → DSP each frame */
static void UpdateParams()
{
  const float dmin = vessl::math::lerp(band_density_min, band_density_max, density_settings.band_min);
  const float dmax = vessl::math::lerp(band_density_min, band_density_max, density_settings.band_max);

  /** @todo expose this on a setting or a knob */
  bool skew = true;
  
  // when control both with skew
  if(skew)
  {
    float dsk = GetSkewValue(vk_density_skew);
    float dtl = vessl::math::constrain(vk_density.Value() - dsk, 0.f, 1.f);
    float dtr = vessl::math::constrain(vk_density.Value() + dsk, 0.f, 1.f);
    float stl = dtl;
    float str = dtr;
    float density_l = vessl::math::lerp(dmin, dmax, dtl);
    float density_r = vessl::math::lerp(dmin, dmax, dtr);
    float spread_l  = vessl::math::lerp(density_settings.spread_min, density_settings.spread_max, stl);
    float spread_r  = vessl::math::lerp(density_settings.spread_min, density_settings.spread_max, str);
    condolences::SetDensity(density_l, density_r);
    condolences::SetSpread(spread_l, spread_r);

    float decay = vk_decay.Value();
    float decsk = GetSkewValue(vk_decay_skew);
    float warp  = vk_warp.Value();
    float warsk = GetSkewValue(vk_warp_skew);
    float smear = vk_smear.Value();
    float smesk = GetSkewValue(vk_smear_skew);
    float melt  = vk_melt.Value();
    float melsk = GetSkewValue(vk_melt_skew);
    float mix   = vk_mix.Value();
    float mixsk = GetSkewValue(vk_mix_skew);
    condolences::SetDecay(decay * 1.f - decsk, decay * 1.f + decsk);
    condolences::SetSpacing(warp * 1.f - warsk, warp * 1.f + warsk);
    condolences::SetSmear(smear * 1.f - smesk, smear * 1.f + smesk);
    condolences::SetMelt(melt * 1.f - melsk, melt * 1.f + melsk);
    condolences::SetMix(mix * 1.f - mixsk, mix * 1.f + mixsk);
  }
  // when controlling left and right independently
  else
  {
    float dtl = vk_density_l.Value();
    float dtr = vk_density_r.Value();
    float stl = dtl;
    float str = dtr;
    float density_l = vessl::math::lerp(dmin, dmax, dtl);
    float density_r = vessl::math::lerp(dmin, dmax, dtr);
    float spread_l  = vessl::math::lerp(density_settings.spread_min, density_settings.spread_max, stl);
    float spread_r  = vessl::math::lerp(density_settings.spread_min, density_settings.spread_max, str);
    condolences::SetDensity(density_l, density_r);
    condolences::SetSpread(spread_l, spread_r);
    condolences::SetDecay(vk_decay_l.Value(), vk_decay_r.Value());
    condolences::SetSpacing(vk_warp_l.Value(), vk_warp_r.Value());
    condolences::SetSmear(vk_smear_l.Value(), vk_smear_r.Value());
    condolences::SetMix(vk_mix_l.Value(), vk_mix_r.Value());
    condolences::SetMelt(vk_melt_l.Value(), vk_melt_r.Value());
  }

  condolences::Update();
}

int main()
{
    // set block size exactly equal to the overlap for synthesis.
    // this should mean we do exactly the same amount of work (generally speaking), every block.
    size_t block_size = condolences::GetBlockSize();
    hw.Init(daisy::SaiHandle::Config::SampleRate::SAI_32KHZ, block_size);
    condolences::Init(hw.SampleRate());

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

    buttons.Global(vb_shift);

    /* Opting into default settings gestures and controls.*/
    settings.UseBrightness();
    settings.UsePresets(presets);

    /* Preset payload — every Serializable surface gets walked on Save/Load. Order IS layout! */
    presets.Manage(pager);
    presets.Manage(locks);
    presets.Manage(settings);
    presets.Manage(density_settings);
    //presets.Manage(buttons);
    presets.UseNames();

    /* ControlLoop is a thin, opt-in driver for the canonical control-rate frame.
     * If desired, you can unroll and modify. */
    loop.Use(pager)
        .Use(buttons)
        .Use(locks)
        .Use(settings)
        //.Use(cv_matrix)
        .Use(vibe_page)
        .Use(rizz_page)
        .Use(host)
        .OnPoll(OnPoll)
        .OnFrame(UpdateParams);

    presets.Init();
    presets.BootLoad();

    UpdateParams();
    hw.StartAudio(condolences::Process);

    for (;;) loop.Tick();
}
