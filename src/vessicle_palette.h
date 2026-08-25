/**
 * Copyright 2026 Damien Quartz
 * 
 * Firmware for testing various vessicle / vessl classes in isolation.
 */

#pragma once

#include "alchemy/led/panel.h"

#define DEFINE_VESSICLE_COLOR(NAME, HEX) constexpr vessicle :: Color NAME = { \
   #HEX, \
    { \
      static_cast<uint8_t>((((HEX ## u)>>16))&0xFFu), \
      static_cast<uint8_t>((((HEX ## u)>> 8))&0xFFu), \
      static_cast<uint8_t>((((HEX ## u) >> 0))&0xFFu) \
    } \
  };

namespace vessicle
{

struct Color
{
  const char* hex;
  alchemy::LedPanel::Rgb rgb;
};

namespace color
{
// color values are the same as standard web colors:
// https://en.wikipedia.org/wiki/Web_colors
DEFINE_VESSICLE_COLOR(White, 0xFFFFFF)
DEFINE_VESSICLE_COLOR(Silver,0xC0C0C0)
DEFINE_VESSICLE_COLOR(Gray, 0x808080)
DEFINE_VESSICLE_COLOR(Black,0x000000)
DEFINE_VESSICLE_COLOR(Red, 0xFF0000)
DEFINE_VESSICLE_COLOR(Orange, 0xFFA500)
DEFINE_VESSICLE_COLOR(Yellow, 0xFFFF00)
DEFINE_VESSICLE_COLOR(Lime, 0x00FF00)
DEFINE_VESSICLE_COLOR(Green, 0x008000)
DEFINE_VESSICLE_COLOR(DarkOliveGreen, 0x556B2F)
DEFINE_VESSICLE_COLOR(Aqua, 0x00FFFF)
DEFINE_VESSICLE_COLOR(Teal, 0x008080)
DEFINE_VESSICLE_COLOR(Blue, 0x0000FF)
DEFINE_VESSICLE_COLOR(Navy, 0x000080)
DEFINE_VESSICLE_COLOR(Fuschia, 0xFF00FF)
DEFINE_VESSICLE_COLOR(Purple, 0x800080)
DEFINE_VESSICLE_COLOR(Indigo, 0x4B0082)
DEFINE_VESSICLE_COLOR(Violet, 0xEE82EE)
DEFINE_VESSICLE_COLOR(DarkSlateBlue, 0x483D8B)
}

struct Palette
{
  Color active;
  Color passive;
};

namespace palette
{
constexpr Palette Fuschia = { color::Fuschia, color::Indigo };
constexpr Palette Lime = { color::Lime, color::DarkOliveGreen };
}

}