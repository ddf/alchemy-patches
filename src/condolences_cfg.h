#pragma once

#include "condolences_dsp.h"
#include "condolences_gui.h"
#include "alchemy/surface/serializable.h"
#include "alchemy/surface/settings.h"
#include "alchemy/surface/presets.h"

namespace condolences
{
namespace config
{
using namespace alchemy;

// User adjustable via knobs on Settings pages.
namespace settings
{
  SelectorHandle mode;
  KnobHandle perception_min;
  KnobHandle perception_max;
  KnobHandle focus_min;
  KnobHandle focus_max;
  KnobHandle ripple_amount_max;
  KnobHandle ripple_lfo_depth;
  KnobHandle ripple_damp_reduct;
  KnobHandle ripple_smear_boost;
}


constexpr float band_density_min = dsp::GetDensityMin();
constexpr float band_density_max = dsp::GetDensityMax();
constexpr float sensi_min        = 0.1f;
constexpr float sensi_max        = 0.9f;
constexpr float dampi_min        = 0.8;
constexpr float dampi_max        = 0.999;
constexpr float motio_min        = 0.1f;
constexpr float motio_max        = 1.0f;
constexpr float smear_min        = 1.0f;
constexpr float smear_max        = 8.0f;

namespace perception
{
  static constexpr float min_default = (192.f - band_density_min) / (band_density_max - band_density_min);
  static constexpr float max_default = (band_density_max - band_density_min) / (band_density_max - band_density_min);
}

namespace focus
{
  static constexpr float min_dafault = 0.0f;
  static constexpr float max_default = 1.0f; 
}

// absolute min/max values that ripple params are allowed to have
namespace ripple
{
constexpr float amount_min = 0.f;
constexpr float amount_max = 1.f;
constexpr float depth_min  = 0.0f;
constexpr float depth_max  = 1.0f;
constexpr float damp_reduct_min = 0.0f;
constexpr float damp_reduct_max = 1.0f;
constexpr float smear_boost_min = 0.f;
constexpr float smear_boost_max = 1.0f;

constexpr float amount_max_default = 0.7f;
constexpr float depth_default = 0.15f;
constexpr float damp_reduct_default = 0.6f;
constexpr float smear_boost_default = 0.20f;
}

constexpr uint8_t mode_page = 0;
constexpr uint8_t mode_pot  = kPotTopRight;
constexpr uint8_t mode_count = static_cast<uint8_t>(dsp::Mode::Count);
constexpr const char* mode_labels[mode_count] = { "Stereo", "Parallel Mono", "Series Mono" };

constexpr uint8_t vibe_page = 1;
constexpr uint8_t rizz_page = 2;

void Configure(Settings& settings, Presets& presets)
{
  settings.Page(mode_page).Name("Config");

  settings::mode = settings.Page(mode_page)
          .Pot(kPotTopRight)
          .Selector(mode_labels)
          .Ident("config.mode");

  settings.Page(vibe_page).Name("Vibe Settings");

  settings::perception_min = settings.Page(vibe_page)
          .Pot(kPotTopLeft)
          .Knob()
          .Name("Perception Min")
          .Ident("percept.min")
          .Default(perception::min_default)
          .Color(vibe_palette.active.rgb);

  settings::perception_max = settings.Page(vibe_page)
          .Pot(kPotTopRight)
          .Knob()
          .Name("Perception Max")
          .Ident("percept.max")
          .Default(perception::max_default)
          .Color(vibe_palette.active.rgb);

  settings::focus_min = settings.Page(vibe_page)
          .Pot(kPotMiddleLeft)
          .Knob()
          .Name("Focus Min")
          .Ident("focus.min")
          .Default(focus::min_dafault)
          .Color(vibe_palette.active.rgb);

  settings::focus_max = settings.Page(vibe_page)
          .Pot(kPotMiddleRight)
          .Knob()
          .Name("Focus Max")
          .Ident("focus.max")
          .Default(focus::max_default)
          .Color(vibe_palette.active.rgb);

  settings.Page(rizz_page).Name("Rizz Settings");

  settings::ripple_amount_max = settings.Page(rizz_page)
          .Pot(kPotTopLeft)
          .Knob()
          .Name("Sizzle Max")
          .Ident("sizzle.max")
          .Default(ripple::amount_max_default)
          .Color(rizz_palette.active.rgb);

  settings::ripple_lfo_depth = settings.Page(rizz_page)
        .Pot(kPotTopRight)
        .Knob()
        .Name("Sizzle LFO Depth")
        .Ident("sizzle.depth")
        .Default(ripple::depth_default)
        .Color(rizz_palette.active.rgb);

  settings::ripple_damp_reduct = settings.Page(rizz_page)
          .Pot(kPotMiddleLeft)
          .Knob()
          .Name("Sizzle Atten (SYM)")
          .Ident("sizzle.damp.reduct")
          .Default(ripple::damp_reduct_default)
          .Color(rizz_palette.active.rgb);

  settings::ripple_smear_boost = settings.Page(rizz_page)
          .Pot(kPotMiddleRight)
          .Knob()
          .Name("Sizzle Boost (SMR)")
          .Ident("sizzle.smear.boost")
          .Default(ripple::smear_boost_default)
          .Color(rizz_palette.active.rgb);

  settings.UseBrightness();
  settings.UsePresets(presets);
}

} // namespace config
} // namespace condolences