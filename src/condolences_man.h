#pragma once

#include "alchemy/host_link/host.h"
#include "alchemy/surface/virtual_knob.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/manual.h"

using namespace alchemy;

namespace condolences
{
extern Page vibe_page;
extern VirtualKnob vk_density;
extern VirtualKnob vk_spread;
extern VirtualKnob vk_sensitivity;
extern VirtualKnob vk_decay;
extern VirtualKnob vk_mix_dry;
extern VirtualKnob vk_mix_wet;

extern Page rizz_page;
extern VirtualKnob vk_shift;
extern VirtualKnob vk_warp;
extern VirtualKnob vk_melt;
extern VirtualKnob vk_smear;
extern VirtualKnob vk_ripple;
extern VirtualKnob vk_motion;

extern Page vibe_skew_page;
extern VirtualKnob vk_density_skew;
extern VirtualKnob vk_spread_skew;
extern VirtualKnob vk_sensitivity_skew;
extern VirtualKnob vk_decay_skew;

extern Page rizz_skew_page;
extern VirtualKnob vk_shift_skew;
extern VirtualKnob vk_warp_skew;
extern VirtualKnob vk_melt_skew;
extern VirtualKnob vk_smear_skew;
extern VirtualKnob vk_ripple_skew;
extern VirtualKnob vk_motion_skew;

namespace manual
{
const Manual kManual = Manual()
    .Tagline("Stereo spectral processor")
    .Preamble("We are sorry for your loss, resonate with your grief, and transform it into psychedelia.");

void AttachTo(hostlink::Host& host)
{
    host.Attach(kManual);

    vibe_page.Help("Control how the input spectrum resonates the output spectrum.");
    {
        vk_density.Help("Sets a threshold that the energy in each input spectrum frequency band must exceed "
                        "in order to be transferred to the output spectrum. "
                        "Increasing Perception lowers the threshold."
                       );
    }
}
} // namespace manual

} // namespace condolences