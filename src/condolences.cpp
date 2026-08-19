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

using namespace alchemy;

/** @todo
  - choose min/max for density
  - set spread_width based on density? 
    could put spread_width on same knob with density, have spread param always at 1.
  - try running in true stereo
  - get hostlink working
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
static Page left_page  = Page(0).Name("Left").Knobs(l_density, l_decay,
                                                    l_spacing, l_smear,
                                                    l_mix,     l_melt);

// static Page right_page = Page(1).Knobs(r_hi_level, r_hi_freq,
//                                        r_mid_level, r_mid_freq,
//                                        r_lo_level, r_lo_freq);

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

constexpr float density_min = condolences::SpectrumSize / 64;
constexpr float density_max = condolences::SpectrumSize / 8;

/* summed CV+knob values → DSP each frame */
static void UpdateParams()
{

  float density = vessl::math::lerp(density_min, density_max, l_density.Value());
  float spread  = vessl::math::lerp(0.35f, 1.f, l_density.Value());
  condolences::SetDensity(density);
  condolences::SetSpread(spread);
  condolences::SetDecay(l_decay.Value());
  condolences::SetSpacing(l_spacing.Value());
  condolences::SetSmear(l_smear.Value());
  condolences::SetMix(l_mix.Value());
  condolences::SetMelt(l_melt.Value());

  condolences::Update();
}

int main()
{
    hw.Init(daisy::SaiHandle::Config::SampleRate::SAI_32KHZ, 256);
    condolences::Init(hw.SampleRate(), hw.BlockSize());

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
