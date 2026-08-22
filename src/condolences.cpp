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

#include "condolences_dsp.h"
#include "vessicle_palette.h"
#include "vessl/vessl.h"
#include <stdio.h>

using namespace alchemy;

/** @todo
  - setup CV routing
*/

/* We define each knob's curve and LED Ring animation.  CV routing lives
 * in the CvMatrix declaration below; declaring it once at the matrix
 * level keeps the knob declarations purely about the knob.  */

/* Page 1 */
static VirtualKnob l_density = VirtualKnob(0, "Density")
  .Linear(0.f, 1.f).Ident("density")
  .Ring(Level(vessicle::fuschia_palette.color, FillAnim::Pulse));

// in seconds, sensible minimum value depends on spectrum size and sample rate
static VirtualKnob l_decay = VirtualKnob(1, "Decay")
  .Linear(0.05f, 10.f).Unit("s").Ident("decay")
  .Ring(Level(vessicle::fuschia_palette.color, FillAnim::Pulse));

static VirtualKnob l_spacing = VirtualKnob(2, "Spacing")
  .Linear(0.f, 1.f).Unit("exp -> lin").Ident("spacing")
  .Ring(Level(vessicle::fuschia_palette.color, FillAnim::Pulse));

static VirtualKnob l_smear = VirtualKnob(3, "Smear")
  .Linear(0.f, 1.f).Ident("smear")
  .Ring(Level(vessicle::fuschia_palette.color, FillAnim::Pulse));

static VirtualKnob l_mix = VirtualKnob(4, "Mix")
  .Linear(0.f, 1.f).Ident("mix")
  .Ring(Level(vessicle::fuschia_palette.color, FillAnim::Pulse));

static VirtualKnob l_melt = VirtualKnob(5, "Melt")
  .Linear(0.f, 1.f)
  .Ring(Level(vessicle::fuschia_palette.color, FillAnim::Pulse));


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

// /* Bind knobs to page */
static Page left_page  = Page(0).Name("Left").Knobs(
  l_density, l_decay, l_spacing, l_smear, l_mix, l_melt
);

// static Page right_page = Page(1).Knobs(r_hi_level, r_hi_freq,
//                                        r_mid_level, r_mid_freq,
//                                        r_lo_level, r_lo_freq);

constexpr float band_density_min = condolences::GetDensityMin();
constexpr float band_density_max = condolences::GetDensityMax();

struct DensitySettings : Serializable
{
  static constexpr float band_min_default = (32.f - band_density_min) / (band_density_max - band_density_min);
  static constexpr float band_max_default = (128.f - band_density_min) / (band_density_max - band_density_min);
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

constexpr size_t page_count = 1;

/* Get our SDK surfaces and opt in to everything */
static AlchemyLab                        hw;
static ControlLoop                       loop    (hw);
static Pager                             pager   (hw.buttons[0], page_count, kNumPots);
static ParamLock<page_count * kNumPots>  locks   (hw.buttons[0], pager);
static Presets                           presets (hw.seed.qspi);
static Settings                          settings(hw, &pager);
static CvMatrix                          cv_matrix(kNumCvInputs);
static hostlink::Host                    host(presets, "condolences", "Condolences", "0.1.0", "abcdefg");
static DensitySettings                   density_settings;

/* summed CV+knob values → DSP each frame */
static void UpdateParams()
{
  float dt = l_density.Value();
  float st = dt;
  float dmin = vessl::math::lerp(band_density_min, band_density_max, density_settings.band_min);
  float dmax = vessl::math::lerp(band_density_min, band_density_max, density_settings.band_max);
  float density = vessl::math::lerp(dmin, dmax, dt);
  float spread  = vessl::math::lerp(density_settings.spread_min, density_settings.spread_max, st);
  condolences::SetDensity(density);
  condolences::SetSpread(spread);
  condolences::SetDecay(l_decay.Value());
  condolences::SetSpacing(l_spacing.Value());
  condolences::SetSmear(l_smear.Value());
  condolences::SetMix(l_mix.Value());
  condolences::SetMelt(l_melt.Value());

  static constexpr float sample_freqs[6] = { 60.f, 120.f, 240.f, 480.f, 480.f*2, 480.f*3 };
  for (uint8_t j = 0; j < kNumCvInputs; ++j)
  {
    float mag = condolences::GetInputBandMagnitude(sample_freqs[j]);
    hw.cv_jacks[j].SetVolts(mag*5.f);
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
    for (uint8_t j = 0; j < kNumCvInputs; ++j)
    {
      hw.cv_jacks[j].EnableCvOutput();
    }

    /* CV routing.  A static layout is just setting each channel once. */
    // cv_matrix.Jack(0).To(l_hi_level);
    // cv_matrix.Jack(1).To(l_hi_freq);
    // cv_matrix.Jack(2).To(l_mid_level);
    // cv_matrix.Jack(3).To(l_mid_freq);
    // cv_matrix.Jack(4).To(l_lo_level);
    // cv_matrix.Jack(5).To(l_lo_freq);

    /* Opting into default settings gestures and controls.*/
    settings.UseBrightness();
    settings.UsePresets(presets);

    /* Preset payload — every Serializable surface gets walked on Save/Load. */
    presets.Manage(pager);
    presets.Manage(density_settings);
    presets.Manage(locks);
    presets.Manage(settings);
    presets.UseNames();

    /* ControlLoop is a thin, opt-in driver for the canonical control-rate frame.
     * If desired, you can unroll and modify. */
    loop.Use(pager)
        .Use(locks)
        .Use(settings)
        //.Use(cv_matrix)
        .Use(left_page)
        //.Use(right_page)
        .Use(host)
        .OnFrame(UpdateParams);

    presets.Init();
    presets.BootLoad();

    UpdateParams();
    hw.StartAudio(condolences::Process);

    for (;;) loop.Tick();
}
