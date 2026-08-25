#pragma once

#include "vessicle_palette.h"
#include "alchemy/surface/virtual_knob.h"

using namespace alchemy;

extern AlchemyLab hw;
extern Pager pager;

constexpr uint8_t kButtonShift = alchemy::kButtonB2;

static bool IsShiftPressed()
{
    // can't shift while performing the settings chord
    return hw.buttons[kButtonShift].Pressed() && !hw.buttons[kButtonB3].Pressed();
}

static bool shift_enabled;

static bool IsShiftEnabled()
{
    return shift_enabled;
}

static void SetShiftEnabled(bool state)
{
    shift_enabled = state;
}

DEFINE_VESSICLE_COLOR(skew_color_right, 3300CC)
DEFINE_VESSICLE_COLOR(skew_color_left,  CC0033)
DEFINE_VESSICLE_COLOR(skew_color_center, FF00FF)
DEFINE_VESSICLE_COLOR(skew_color_back, 080008)

static Level vibe_spec = Level(vessicle::palette::Fuschia.active.rgb, FillAnim::Pulse);
  //.Passive(vessicle::palette::Fuschia.passive.rgb);

static Level rizz_spec = Level(vessicle::palette::Lime.active.rgb, FillAnim::Ripple)
  .Passive(vessicle::palette::Lime.passive.rgb);

static Bipolar skew_spec = Bipolar(
  skew_color_right.rgb,
  skew_color_left.rgb,
  skew_color_center.rgb
);

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
static void DrawKnobWithSkew(
  LedPanel& panel, uint8_t pot,
  const ArcGeometry& geo, float norm,
  uint32_t t_ms, void* ctx
)
{
  VirtualKnob* skew_knob = static_cast<VirtualKnob*>(ctx);
  const float skew_val = GetSkewValue(*skew_knob);
  
  FillDesc over_right;
  over_right.center_color = { 0u, 0u, 0u };
  over_right.compose = FillCompose::Replace;
  over_right.mode = FillMode::Center;
  over_right.color = skew_color_right.rgb;
  over_right.neg_color = skew_color_right.rgb;
  over_right.pivot01 = norm;

  FillDesc over_left;
  over_left.center_color = { 0u, 0u, 0u };
  over_left.compose = FillCompose::Overlay;
  over_left.mode = FillMode::Center;
  over_left.color = skew_color_left.rgb;
  over_left.neg_color = skew_color_left.rgb;
  over_left.pivot01 = norm;

  PipDesc center;
  center.color = skew_color_center.rgb;
  center.background = { 0u, 0u, 0u }; // skew_color_back.rgb;
  center.compose = PipCompose::Add;
  center.smooth = true;

  PipDesc bottom;
  bottom.color = skew_val == 0 ? skew_color_center.rgb :
                 skew_val > 0 ? skew_color_right.rgb : skew_color_left.rgb;
  if( IsShiftEnabled() )
  {
    bottom.blink_hz = 4.f;
  }
  else
  {
    bottom.blink_hz = skew_val == 0 ? 0.f : 2.f;
  }

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
static void DrawSkewKnob(
  LedPanel& panel, uint8_t pot,
  const ArcGeometry& geo, float norm,
  uint32_t t_ms, void* ctx
)
{
  VirtualKnob* this_knob = static_cast<VirtualKnob*>(ctx);

  if (IsShiftEnabled())
  {
    PotState param_state = pager.State(0, pot);
    DrawKnobWithSkew(panel, pot, geo, param_state.stored, t_ms, this_knob);
  }
  else
  {
    FillDesc fill;
    fill.mode = FillMode::Center;
    fill.compose = FillCompose::Replace;
    fill.center_color = skew_spec.center;
    fill.neg_color = skew_spec.neg;
    fill.color = skew_spec.pos;
    fill.pivot01 = skew_spec.pivot;

    RingFrame f;
    f.Begin(geo);
    f.Base(fill, norm, t_ms);
    f.Emit(panel, pot);
  }
}