/**
 * Copyright 2026 Damien Quartz
 * 
 * Firmware for testing various vessicle / vessl classes in isolation.
 */

#pragma once

#include "alchemy/led/panel.h"

#define DEFINE_VESSICLE_COLOR(NAME, HEX) constexpr vessicle :: Color NAME = { \
   "#" #HEX, \
    { \
      static_cast<uint8_t>((((0x ## HEX ## u)>>16))&0xFFu), \
      static_cast<uint8_t>((((0x ## HEX ## u)>> 8))&0xFFu), \
      static_cast<uint8_t>((((0x ## HEX ## u) >> 0))&0xFFu) \
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
DEFINE_VESSICLE_COLOR(White,  FFFFFF)
DEFINE_VESSICLE_COLOR(Silver, C0C0C0)
DEFINE_VESSICLE_COLOR(Gray,   808080)
DEFINE_VESSICLE_COLOR(Black,  000000)
DEFINE_VESSICLE_COLOR(Red,    FF0000)
DEFINE_VESSICLE_COLOR(Orange, FFA500)
DEFINE_VESSICLE_COLOR(Yellow, FFFF00)
DEFINE_VESSICLE_COLOR(Lime,   00FF00)
DEFINE_VESSICLE_COLOR(Green,  008000)
DEFINE_VESSICLE_COLOR(DarkOliveGreen, 556B2F)
DEFINE_VESSICLE_COLOR(Aqua, 00FFFF)
DEFINE_VESSICLE_COLOR(Teal, 008080)
DEFINE_VESSICLE_COLOR(Blue, 0000FF)
DEFINE_VESSICLE_COLOR(Navy, 000080)
DEFINE_VESSICLE_COLOR(Fuschia, FF00FF)
DEFINE_VESSICLE_COLOR(Purple, 800080)
DEFINE_VESSICLE_COLOR(Indigo, 4B0082)
DEFINE_VESSICLE_COLOR(Violet, EE82EE)
DEFINE_VESSICLE_COLOR(DarkSlateBlue, 483D8B)
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