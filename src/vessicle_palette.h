/**
 * Copyright 2026 Damien Quartz
 * 
 * Firmware for testing various vessicle / vessl classes in isolation.
 */

#pragma once

#include "alchemy/led/panel.h"

using Panel = alchemy::LedPanel;

namespace vessicle
{

struct ParamPalette
{
  Panel::Rgb color;
  float scale_low, scale_high;
};

constexpr ParamPalette violet_palette = { alchemy::kColorViolet, 0.2f, 1.0f };
constexpr ParamPalette lime_palette = { alchemy::kColorLime, 0.2f, 1.f };

}