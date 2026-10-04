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

#include "alchemy/surface/virtual_knob.h"

#define CONDOLENCES_CV_A "∴"
#define CONDOLENCES_CV_B "∵"
#define CONDOLENCES_CV_C "⊙"
#define CONDOLENCES_CV_D "∞"
#define CONDOLENCES_CV_E "↕"
#define CONDOLENCES_CV_F "↔"

namespace condolences
{
using namespace alchemy;

extern VirtualKnob vk_density;
extern VirtualKnob vk_spread;
extern VirtualKnob vk_sensitivity;
extern VirtualKnob vk_decay;
extern VirtualKnob vk_mix_dry;
extern VirtualKnob vk_mix_wet;

extern VirtualKnob vk_shift;
extern VirtualKnob vk_warp;
extern VirtualKnob vk_melt;
extern VirtualKnob vk_smear;
extern VirtualKnob vk_ripple;
extern VirtualKnob vk_motion;

extern VirtualKnob vk_density_skew;
extern VirtualKnob vk_spread_skew;
extern VirtualKnob vk_sensitivity_skew;
extern VirtualKnob vk_decay_skew;

extern VirtualKnob vk_shift_skew;
extern VirtualKnob vk_warp_skew;
extern VirtualKnob vk_melt_skew;
extern VirtualKnob vk_smear_skew;
extern VirtualKnob vk_ripple_skew;
extern VirtualKnob vk_motion_skew;

namespace param
{
enum
{
    Perception = 0,
    Focus,
    Empathy,
    Sympathy,
    Transpose,
    Warp,
    Melt,
    Smear,
    Sizzle,
    Emote,
    Dry,
    Wet,
    PerceptionSkew,
    FocusSkew,
    EmpathySkew,
    SympathySkew,
    TransposeSkew,
    WarpSkew,
    MeltSkew,
    SmearSkew,
    SizzleSkew,
    EmoteSkew,

    Count
};

static constexpr const char* name[] = {
    "Perception",
    "Focus",
    "Empathy",
    "Sympathy",
    "Transpose",
    "Warp",
    "Melt",
    "Smear",
    "Sizzle",
    "Emote",
    "Dry",
    "Wet",
    "Perception Skew",
    "Focus Skew",
    "Empathy Skew",
    "Sympathy Skew",
    "Transpose Skew",
    "Warp Skew",
    "Melt Skew",
    "Smear Skew",
    "Sizzle Skew",
    "Emote Skew",
};

static constexpr VirtualKnob* knob[] = {
    &vk_density,
    &vk_spread,
    &vk_sensitivity,
    &vk_decay,
    &vk_shift,
    &vk_warp,
    &vk_melt,
    &vk_smear,
    &vk_ripple,
    &vk_motion,
    &vk_mix_dry,
    &vk_mix_wet,
    &vk_density_skew,
    &vk_spread_skew,
    &vk_sensitivity_skew,
    &vk_decay_skew,
    &vk_shift_skew,
    &vk_warp_skew,
    &vk_melt_skew,
    &vk_smear_skew,
    &vk_ripple_skew,
    &vk_motion_skew
};

} // namespace param

namespace cv
{
enum
{
    A, 
    B, 
    C, 
    D, 
    E, 
    F,

    Count
};

static constexpr const char* jack[] = {
    "J3",
    "J4",
    "J5",
    "J6",
    "J7",
    "J8",
};

static constexpr const char* name[] = {
    CONDOLENCES_CV_A,
    CONDOLENCES_CV_B,
    CONDOLENCES_CV_C,
    CONDOLENCES_CV_D,
    CONDOLENCES_CV_E,
    CONDOLENCES_CV_F,
};

} // namespace cv
} // namespace condolences