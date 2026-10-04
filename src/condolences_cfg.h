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

#pragma once

#include "alchemy/surface/serializable.h"
#include "alchemy/surface/settings.h"
#include "alchemy/surface/presets.h"

#include "condolences_dsp.h"
#include "condolences_gui.h"
#include "condolences_prm.h"

// #define PROFILE_ENABLED

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

  SelectorHandle cv_a_dest;
  SelectorHandle cv_b_dest;
  SelectorHandle cv_c_dest;
  SelectorHandle cv_d_dest;
  SelectorHandle cv_e_dest;
  SelectorHandle cv_f_dest;

  BipolarHandle cv_a_level;
  BipolarHandle cv_b_level;
  BipolarHandle cv_c_level;
  BipolarHandle cv_d_level;
  BipolarHandle cv_e_level;
  BipolarHandle cv_f_level;
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
  static constexpr float min_default = 0.0f;
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

namespace cv
{
constexpr float level_default = 1.0f;
}

constexpr uint8_t mode_page = 0;
constexpr uint8_t mode_pot  = kPotTopRight;
constexpr uint8_t mode_count = static_cast<uint8_t>(dsp::Mode::Count);
constexpr const char* mode_labels[mode_count] = { "Stereo", "Parallel Mono", "Series Mono" };

constexpr uint8_t params_page = 3;
constexpr uint8_t cv_assign_page = 1;
constexpr uint8_t cv_level_page = 2;
// we don't get more than this

void Configure(Settings& settings, Presets& presets)
{
  settings.Page(mode_page).Name("Config");

  settings::mode = settings.Page(mode_page)
          .Pot(kPotTopRight)
          .Selector(mode_labels)
          .Name("Mode")
          .Ident("config.mode");

  settings.Page(params_page).Name("Internals");

  settings::perception_min = settings.Page(params_page)
          .Pot(kPotTopLeft)
          .Knob()
          .Name("Perception Min")
          .Ident("percept.min")
          .Default(perception::min_default)
          .Color(vibe_palette.active.rgb);

  settings::perception_max = settings.Page(params_page)
          .Pot(kPotTopRight)
          .Knob()
          .Name("Perception Max")
          .Ident("percept.max")
          .Default(perception::max_default)
          .Color(vibe_palette.active.rgb);

  settings::ripple_amount_max = settings.Page(params_page)
          .Pot(kPotMiddleLeft)
          .Knob()
          .Name("Sizzle Max")
          .Ident("sizzle.max")
          .Default(ripple::amount_max_default)
          .Color(rizz_palette.active.rgb);

  settings::ripple_lfo_depth = settings.Page(params_page)
        .Pot(kPotMiddleRight)
        .Knob()
        .Name("Sizzle LFO Depth")
        .Ident("sizzle.depth")
        .Default(ripple::depth_default)
        .Color(rizz_palette.active.rgb);

  settings::ripple_damp_reduct = settings.Page(params_page)
          .Pot(kPotBottomLeft)
          .Knob()
          .Name("Sizzle Atten (SYM)")
          .Ident("sizzle.damp.reduct")
          .Default(ripple::damp_reduct_default)
          .Color(rizz_palette.active.rgb);

  settings::ripple_smear_boost = settings.Page(params_page)
          .Pot(kPotBottomRight)
          .Knob()
          .Name("Sizzle Boost (SMR)")
          .Ident("sizzle.smear.boost")
          .Default(ripple::smear_boost_default)
          .Color(rizz_palette.active.rgb);

  settings.Page(cv_assign_page).Name("CV Routing");

  settings::cv_a_dest = settings.Page(cv_assign_page)
          .Pot(kPotTopLeft)
          .Selector(param::name)
          .Default(param::Perception)
          .Name(CONDOLENCES_CV_A " Target")
          .Ident("cv.assign.a");

  settings::cv_b_dest = settings.Page(cv_assign_page)
          .Pot(kPotTopRight)
          .Selector(param::name)
          .Default(param::Empathy)
          .Name(CONDOLENCES_CV_B " Target")
          .Ident("cv.assign.b");
          
  settings::cv_c_dest = settings.Page(cv_assign_page)
          .Pot(kPotMiddleLeft)
          .Selector(param::name)
          .Default(param::Focus)
          .Name(CONDOLENCES_CV_C " Target")
          .Ident("cv.assign.c");

  settings::cv_d_dest = settings.Page(cv_assign_page)
          .Pot(kPotMiddleRight)
          .Selector(param::name)
          .Default(param::Sympathy)
          .Name(CONDOLENCES_CV_D " Target")
          .Ident("cv.assign.d");

  settings::cv_e_dest = settings.Page(cv_assign_page)
          .Pot(kPotBottomLeft)
          .Selector(param::name)
          .Default(param::Transpose)
          .Name(CONDOLENCES_CV_E " Target")
          .Ident("cv.assign.e");

  settings::cv_f_dest = settings.Page(cv_assign_page)
          .Pot(kPotBottomRight)
          .Selector(param::name)
          .Default(param::Warp)
          .Name(CONDOLENCES_CV_F " Target")
          .Ident("cv.assign.f");

  settings.Page(cv_level_page).Name("CV Amount");

  settings::cv_a_level = settings.Page(cv_level_page)
          .Pot(kPotTopLeft)
          .Bipolar()
          .Name(CONDOLENCES_CV_A " Amount")
          .Default(cv::level_default)
          .Ident("cv.level.a");

  settings::cv_b_level = settings.Page(cv_level_page)
          .Pot(kPotTopRight)
          .Bipolar()
          .Name(CONDOLENCES_CV_B " Amount")
          .Default(cv::level_default)
          .Ident("cv.level.b");
          
  settings::cv_c_level = settings.Page(cv_level_page)
          .Pot(kPotMiddleLeft)
          .Bipolar()
          .Name(CONDOLENCES_CV_C " Amount")
          .Default(cv::level_default)
          .Ident("cv.level.c");

  settings::cv_d_level = settings.Page(cv_level_page)
          .Pot(kPotMiddleRight)
          .Bipolar()
          .Name(CONDOLENCES_CV_D " Amount")
          .Default(cv::level_default)
          .Ident("cv.level.d");

  settings::cv_e_level = settings.Page(cv_level_page)
          .Pot(kPotBottomLeft)
          .Bipolar()
          .Name(CONDOLENCES_CV_E " Amount")
          .Default(cv::level_default)
          .Ident("cv.level.e");

  settings::cv_f_level = settings.Page(cv_level_page)
          .Pot(kPotBottomRight)
          .Bipolar()
          .Name(CONDOLENCES_CV_F " Amount")
          .Default(cv::level_default)
          .Ident("cv.level.f");

  settings.UseBrightness();
  settings.UsePresets(presets);
}

static VirtualKnob vk_none = VirtualKnob();

VirtualKnob& GetCvDest(uint8_t idx)
{
  switch(idx)
  {
    case 0: return *param::knob[settings::cv_a_dest.Value()];
    case 1: return *param::knob[settings::cv_b_dest.Value()];
    case 2: return *param::knob[settings::cv_c_dest.Value()];
    case 3: return *param::knob[settings::cv_d_dest.Value()];
    case 4: return *param::knob[settings::cv_e_dest.Value()];
    case 5: return *param::knob[settings::cv_f_dest.Value()];
  }
  return vk_none;
}

} // namespace config
} // namespace condolences