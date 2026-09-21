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
#include "condolences_dsp.h"
#include "condolences_gui.h"
#include "vessl/vessl.h"
#include <stdio.h>

using namespace alchemy;

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

////////////////////////////////////////////////////////////////////////////////
// Settings
constexpr float band_density_min = condolences::GetDensityMin();
constexpr float band_density_max = condolences::GetDensityMax();
constexpr float sensi_min        = 0.1f;
constexpr float sensi_max        = 0.9f;
constexpr float dampi_min        = 0.8;
constexpr float dampi_max        = 0.949;
constexpr float motio_min        = 0.05f;
constexpr float motio_max        = 1.0f;
constexpr float smear_min        = 0.25f;
constexpr float smear_max        = 4.0f;
constexpr float rippl_min        = 0.f;
constexpr float rippl_max        = 1.f;

struct DensitySettings : Serializable
{
  static constexpr float band_min_default = (24.f - band_density_min) / (band_density_max - band_density_min);
  static constexpr float band_max_default = (band_density_max - band_density_min) / (band_density_max - band_density_min);
  static constexpr float spread_min_dafault = 0.0f;
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

/////////////////////////////////////////////////////////////////////////////
// Knobs
ALCHEMY_SRAM  
static VirtualKnob vk_mix_dry = VirtualKnob(kPotBottomLeft, "Dry")
  .Ident("mix.dry")
  .Linear(0.f, 1.f)
  .Ring(Level(vibe_palette.active.rgb));

ALCHEMY_SRAM
static VirtualKnob vk_mix_wet = VirtualKnob(kPotBottomRight, "Wet")
  .Ident("mix.wet")
  .Linear(0.f, 1.f)
  .Ring(Level(vibe_palette.active.rgb));

///////////////////////////////////////////////////////////////////////
// Skew Knobs
ALCHEMY_SRAM
static VirtualKnob vk_density_skew = VirtualKnob(kPotTopLeft, "Perception Skew")
  .Ident("depth.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));

ALCHEMY_SRAM
static VirtualKnob vk_spread_skew = VirtualKnob(kPotTopRight, "Focus Skew")
  .Ident("breadth.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_sensitivity_skew = VirtualKnob(kPotMiddleLeft, "Empathy Skew")
  .Ident("sensi.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_decay_skew = VirtualKnob(kPotMiddleRight, "Sympathy Skew")
  .Ident("sympa.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &vibe_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_shift_skew = VirtualKnob(kPotTopLeft, "Transpose Skew")
  .Ident("shift.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_warp_skew = VirtualKnob(kPotTopRight, "Warp Skew")
  .Ident("warp.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_melt_skew = VirtualKnob(kPotMiddleLeft, "Melt Skew")
  .Ident("melt.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_smear_skew = VirtualKnob(kPotMiddleRight, "Smear Skew")
  .Ident("smear.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_ripple_skew = VirtualKnob(kPotBottomLeft, "Sizzle Skew")
  .Ident("ripple.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

ALCHEMY_SRAM  
static VirtualKnob vk_motion_skew = VirtualKnob(kPotBottomRight, "Emote Skew")
  .Ident("smear.skew")
  .Linear(-0.5f, 0.5f)
  .Ring(Custom(SkewKnob, &rizz_palette));

/////////////////////////////////////////////////////////////////////////
// Param Knobs which get skewed
ALCHEMY_SRAM
static VirtualKnob vk_density = VirtualKnob(kPotTopLeft, "Perception")
  .Ident("depth.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_density_skew));

ALCHEMY_SRAM
static VirtualKnob vk_spread = VirtualKnob(kPotTopRight, "Focus")
  .Ident("breadth.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_spread_skew));

ALCHEMY_SRAM
static VirtualKnob vk_sensitivity = VirtualKnob(kPotMiddleLeft, "Empathy")
  .Ident("sensi.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_sensitivity_skew));

// in seconds, sensible minimum value depends on spectrum size and sample rate
ALCHEMY_SRAM
static VirtualKnob vk_decay = VirtualKnob(kPotMiddleRight, "Sympathy")
  .Ident("sympa.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_decay_skew));

ALCHEMY_SRAM  
static VirtualKnob vk_shift = VirtualKnob(kPotTopLeft, "Transpose")
  .Ident("shift.both")
  .Linear(-1.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_shift_skew));

ALCHEMY_SRAM  
static VirtualKnob vk_warp = VirtualKnob(kPotTopRight, "Warp")
  .Ident("warp.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_warp_skew));

ALCHEMY_SRAM  
static VirtualKnob vk_melt = VirtualKnob(kPotMiddleLeft, "Melt")
  .Ident("melt.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_melt_skew));

ALCHEMY_SRAM  
static VirtualKnob vk_smear = VirtualKnob(kPotMiddleRight, "Smear")
  .Ident("smear.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_smear_skew));

ALCHEMY_SRAM  
static VirtualKnob vk_ripple = VirtualKnob(kPotBottomLeft, "Sizzle")
  .Ident("ripple.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_ripple_skew));


ALCHEMY_SRAM  
static VirtualKnob vk_motion = VirtualKnob(kPotBottomRight, "Emote")
  .Ident("motion.both")
  .Linear(0.f, 1.f)
  .Ring(Custom(KnobWithSkew, &vk_motion_skew));

//////////////////////////////////////////////////////////////////////
// Pages
enum PageId : uint8_t
{
  kPageVibes, kPageVibeSkew, kPageRizz, kPageRizzSkew,
  kPageCount
};

ALCHEMY_SRAM  
static Page vibe_page = Page(kPageVibes)
  .Name("Vibes")
  .Color(vibe_palette.active.hex)
  .Knobs(vk_density, vk_spread, vk_sensitivity, vk_decay, vk_mix_dry, vk_mix_wet);

ALCHEMY_SRAM
static Page vibe_skew_page = Page(kPageVibeSkew)
  .Name("Vibe Skew")
  .Color(vibe_palette.active.hex)
  .Knobs(vk_density_skew, vk_spread_skew, vk_sensitivity_skew, vk_decay_skew);

ALCHEMY_SRAM  
static Page rizz_page = Page(kPageRizz)
  .Name("Rizz")
  .Color(rizz_palette.active.hex)
  .Knobs(vk_shift, vk_warp, vk_melt, vk_smear, vk_ripple, vk_motion);

ALCHEMY_SRAM  
static Page rizz_skew_page = Page(kPageRizzSkew)
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
hostlink::Host                      host(presets, "condolences", "Condolences", "0.1.1", "4c9c46483d18218e42e47b4ddc9c4a74ac903017");

static DensitySettings density_settings;

constexpr uint8_t mode_page = 0;
constexpr uint8_t mode_pot  = kPotTopRight;
constexpr uint8_t mode_count = static_cast<uint8_t>(condolences::Mode::Count);
constexpr const char* mode_labels[mode_count] = { "True Stereo", "Parallel Mono", "Series Mono" };

static void ConfigureInterface()
{
  pager.Cycle(hw.buttons[kButtonB1], kPageVibes, kPageRizz)
       .Shift(hw.buttons[kButtonB2], kPageVibeSkew)
       .From(kPageVibes)
       .Shift(hw.buttons[kButtonB3], kPageRizzSkew)
       .From(kPageRizz);

  settings.UseBrightness();
  settings.UsePresets(presets);
  settings.Page(mode_page)
          .Name("Config")
          .Pot(kPotTopRight)
          .Selector(mode_labels)
          .Ident("config.mode");
}

/* summed CV+knob values → DSP each frame */
static void UpdateParams()
{
  const float dmin = vessl::math::lerp(band_density_min, band_density_max, density_settings.band_min);
  const float dmax = vessl::math::lerp(band_density_min, band_density_max, density_settings.band_max);
  {
    // decay
    float decsk = GetSkewValue(vk_decay_skew);
    float decl  = vessl::math::constrain(vk_decay.Value() - decsk, 0.f, 1.f);
    float decr  = vessl::math::constrain(vk_decay.Value() + decsk, 0.f, 1.f);

    // density
    float dsk = GetSkewValue(vk_density_skew);
    float dtl = vessl::math::constrain(vk_density.Value() - dsk, 0.f, 1.f);
    float dtr = vessl::math::constrain(vk_density.Value() + dsk, 0.f, 1.f);

    // spread
    float ssk = GetSkewValue(vk_spread_skew);
    float stl = vessl::math::constrain(vk_spread.Value() - ssk, 0.f, 1.f);
    float str = vessl::math::constrain(vk_spread.Value() + ssk, 0.f, 1.f);

    float dampngl   = vessl::math::interp<vessl::math::easing::quad::out>(dampi_min, dampi_max, decl);
    float dampngr   = vessl::math::interp<vessl::math::easing::quad::out>(dampi_min, dampi_max, decr);
    float density_l = vessl::math::lerp(dmin, dmax, dtl);
    float density_r = vessl::math::lerp(dmin, dmax, dtr);
    float spread_l  = vessl::math::lerp(density_settings.spread_min, density_settings.spread_max, stl);
    float spread_r  = vessl::math::lerp(density_settings.spread_min, density_settings.spread_max, str);

    condolences::SetDensity(density_l, density_r);
    condolences::SetDamping(dampngl, dampngr);
    condolences::SetSpread(spread_l, spread_r);

    float sens  = vk_sensitivity.Value();
    float sensk = GetSkewValue(vk_sensitivity_skew);
    float sensl = vessl::math::constrain(vessl::math::lerp(sensi_min, sensi_max, sens - sensk), sensi_min, sensi_max);
    float sensr = vessl::math::constrain(vessl::math::lerp(sensi_min, sensi_max, sens + sensk), sensi_min, sensi_max);

    float shft  = vk_shift.Value();
    float shfsk = GetSkewValue(vk_shift_skew);
    
    float warp  = vk_warp.Value();
    float warsk = GetSkewValue(vk_warp_skew);
    
    float smear = vk_smear.Value();
    float smesk = GetSkewValue(vk_smear_skew);
    float smrl  = vessl::math::constrain(vessl::math::lerp(smear_min, smear_max, smear - smesk), smear_min, smear_max);
    float smrr  = vessl::math::constrain(vessl::math::lerp(smear_min, smear_max, smear + smesk), smear_min, smear_max);

    float melt  = vk_melt.Value();
    float melsk = GetSkewValue(vk_melt_skew);

    float ripl  = vk_ripple.Value();
    float ripsk = GetSkewValue(vk_ripple_skew);
    float ripll = vessl::math::constrain(vessl::math::lerp(rippl_min, rippl_max, ripl - ripsk), rippl_min, rippl_max);
    float riplr = vessl::math::constrain(vessl::math::lerp(rippl_min, rippl_max, ripl + ripsk), rippl_min, rippl_max);

    float motn  = vk_motion.Value();
    float motsk = GetSkewValue(vk_motion_skew);
    float motl  = vessl::math::constrain(vessl::math::lerp(motio_min, motio_max, motn - motsk), motio_min, motio_max);
    float motr  = vessl::math::constrain(vessl::math::lerp(motio_min, motio_max, motn + motsk), motio_min, motio_max);

    float mixd  = vk_mix_dry.Value();
    float mixw  = vk_mix_wet.Value();

    condolences::SetSensitivity(sensl, sensr);
    condolences::SetShift(shft - shfsk, shft + shfsk);
    condolences::SetSpacing(warp - warsk, warp + warsk);
    condolences::SetSmear(smrl, smrr);
    condolences::SetMelt(melt - melsk, melt + melsk);
    condolences::SetRipple(ripll, riplr);
    condolences::SetMotion(motl, motr);
    condolences::SetMix(mixd, mixw);
  }
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

    /* Opting into default settings gestures and controls.*/
    ConfigureInterface();

    /* Preset payload — every Serializable surface gets walked on Save/Load. Order IS layout! */
    presets.Manage(pager);
    presets.Manage(locks);
    presets.Manage(settings);
    presets.Manage(density_settings);
    presets.Manage(profiler);
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
    profiler.StartAudio(condolences::Process);

    for (;;) loop.Tick();
}
