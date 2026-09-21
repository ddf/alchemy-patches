#pragma once

#include "vessicle_palette.h"
#include "alchemy/surface/virtual_knob.h"

using namespace alchemy;

extern AlchemyLab hw;
extern Pager pager;

constexpr uint8_t kButtonShift = alchemy::kButtonB2;

DEFINE_VESSICLE_COLOR(vibe_color_right, 3300CC)
DEFINE_VESSICLE_COLOR(vibe_color_left,  CC0033)
DEFINE_VESSICLE_COLOR(rizz_color_right, 0033CC)
DEFINE_VESSICLE_COLOR(rizz_color_left,  00CC33)

vessicle::Palette vibe_palette = { 
  vessicle::color::Fuschia, 
  vessicle::color::Indigo,
  vessicle::color::Black,
  vibe_color_right,
  vibe_color_left
};

vessicle::Palette rizz_palette = {
  vessicle::color::Aqua,
  vessicle::color::Teal,
  vessicle::color::Black,
  rizz_color_right,
  rizz_color_left
};

static constexpr float skew_max    = 0.5f;
static constexpr float skew_detent = 0.05f;

static float GetSkewValue(const VirtualKnob& fromKnob)
{
  float value = vessl::math::abs(fromKnob.Value());
  float sign  = fromKnob.Value() > 0 ? 1.f : -1.f;
  float d = value - skew_detent;
  return d < 0 ? 0.f : vessl::math::lerp(0.f, skew_max*sign, d / (skew_max - skew_detent));
}

// for rendering knobs that have a parameter that has a skew param associated with it.
static void KnobWithSkew(
  LedPanel& panel, uint8_t pot,
  const ArcGeometry& geo, float norm,
  uint32_t t_ms, void* ctx
)
{
  VirtualKnob* skew_knob = static_cast<VirtualKnob*>(ctx);
  vessicle::Palette* palette = static_cast<vessicle::Palette*>(skew_knob->CustomCtx());
  const float skew_val = GetSkewValue(*skew_knob);
  
  FillDesc over_right;
  over_right.center_color = { 0u, 0u, 0u };
  over_right.compose = FillCompose::Replace;
  over_right.mode = FillMode::Center;
  over_right.color = palette->positive.rgb;
  over_right.neg_color = palette->positive.rgb;
  over_right.pivot01 = norm;

  FillDesc over_left;
  over_left.center_color = { 0u, 0u, 0u };
  over_left.compose = FillCompose::Overlay;
  over_left.mode = FillMode::Center;
  over_left.color = palette->negative.rgb;
  over_left.neg_color = palette->negative.rgb;
  over_left.pivot01 = norm;

  PipDesc center;
  center.color = palette->active.rgb;
  center.background = palette->background.rgb;
  center.compose = PipCompose::Add;
  center.smooth = true;

  PipDesc bottom;
  bottom.color = skew_val == 0 ? palette->active.rgb :
                 skew_val > 0 ? palette->positive.rgb : palette->negative.rgb;
  
  bottom.blink_hz = skew_val == 0 ? 0.f : 2.f;

  RingFrame f;
  f.Begin(geo);
  f.Base(over_right, norm + skew_val, t_ms);
  f.Pip(Region::BottomPip, bottom, 0.f, 1.f, t_ms);
  f.Emit(panel, pot);

  RingFrame g;
  g.BeginOverlay(geo);
  g.Base(over_left, norm - skew_val, t_ms);
  g.Pip(Region::Full, center, norm, 1.f, t_ms);
  g.Emit(panel, pot);
}

// for rendering skew param knobs
static void SkewKnob(
  LedPanel& panel, uint8_t pot,
  const ArcGeometry& geo, float norm,
  uint32_t t_ms, void* ctx
)
{
  vessicle::Palette* spec = static_cast<vessicle::Palette*>(ctx);

  FillDesc fill;
  fill.mode = FillMode::Center;
  fill.compose = FillCompose::Replace;
  fill.center_color = spec->active.rgb;
  fill.neg_color = spec->negative.rgb;
  fill.color = spec->positive.rgb;
  fill.pivot01 = 0.5f;

  RingFrame f;
  f.Begin(geo);
  f.Base(fill, norm, t_ms);
  f.Emit(panel, pot);
}