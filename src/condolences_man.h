#pragma once

#include "alchemy/host_link/host.h"
#include "alchemy/surface/virtual_knob.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/manual.h"

using namespace alchemy;

namespace condolences
{
extern Manual thee_manual;
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
void AttachTo(hostlink::Host& host)
{
    thee_manual.Tagline("Stereo spectral processor loosely based on the phenomenon of sympathetic vibration.")
               .Preamble("Your signal will be analyzed with varying degrees of attention to ascertain the frequency of the vibrations contained therein. "
                         "We shall then attempt to reproduce your signal by responding to its content with empathy and sympathy. "
                         "However, we are _not_ you and are bound to introduce our own interpretations, extensions, transformations, and misunderstandings. "
                         "You may then mix your signal with ours to produce a novel perspective that is greater than the sum of its parts.  \n  \n"
                         "In other words: we are sorry for your loss, resonate with your grief, and transform it into psychedelia.")
               .Section("overview", "Overview", 
                        "This is the overview.");

    host.Attach(thee_manual);

    vibe_page.Help("Control how the input spectrum influences the output spectrum.  \n"
                   "Perception, Focus, Empathy, and Sympathy can be skewed using controls on the Vibe Skew page.  \n  \n"
                   "_When skew is present the pip under the parameter knob will blink. " 
                   "The amount and direction of skew will render as two colored bands extending out from the center value._");
    {
        vk_density.Help("Sets a threshold that the energy in each input spectrum frequency band must exceed "
                        "in order to be transferred to the output spectrum.  \n"
                        "Increasing Perception lowers the threshold.")
                  .SeeAlso(vk_density_skew);

        vk_sensitivity.Help("Sets a threshold that the energy in an output spectrum frequency band must be below"
                            "in order for incoming energy to 'excite' it, making the output more responsive to the input.  \n"
                            "Increasing Empathy increases the threshold.")
                      .SeeAlso(vk_sensitivity_skew);

        vk_spread.Help("Sets a falloff coefficient used to 'excite' frequency bands " 
                       "on either side of a target band in the output spectrum.  \n"
                       "Increasing Focus increases the width of the falloff.")
                 .SeeAlso(vk_spread_skew);

        vk_decay.Help("Sets a damping coefficient that reduces the energy present in each output spectrum frequency band "
                      "when the spectrum is processed during each overlap-add block.  \n"
                      "Increasing Sympathy causes energy to decay more slowly, or 'ring out' for longer.")
                .SeeAlso(vk_decay_skew);

        vk_mix_dry.Help("Sets the level of the dry signal in the final output.");

        vk_mix_wet.Help("Sets the level of the wet signal in the final output.");
    }

    rizz_page.Help("Control the input-to-output spectrum mapping and the output spectrum processing.  \n"
                   "All parameters can be skewed using controls on the Rizz Skew page.  \n  \n"
                   "_When skew is present the pip under the parameter knob will blink. " 
                   "The amount and direction of skew will render as two colored bands extending out from the center value._");
    {
        vk_shift.Help("Transpose input frequency content when transferring it to the output spectrum.  \n"
                      "Noon is no transposition, clockwise transposes up to an octave higher, counterclockwise up to an octave lower.")
                .SeeAlso(vk_shift_skew);

        vk_warp.Help("Warps the mapping of frequency bands from the input spectrum to the output spectrum.  \n"
                     "Increasing warp 'spreads out' the frequency content of the input into the upper end of the output.")
               .SeeAlso(vk_warp_skew);

        vk_melt.Help("Animates the frequency contents of the output spectrum downward.  \n"
                     "Increasing Melt increases the speed at which pitch drops while reducing decay time.")
               .SeeAlso(vk_decay, vk_melt_skew);

        vk_ripple.Help("Applies an LFO to frequency band magnitudes.  \n"
                       "Increasing Sizzle increases how much the LFO changes frequency content at each overlap-add. "
                       "LFO frequency is related to Smear.")
                 .SeeAlso(vk_ripple_skew, vk_smear);

        vk_smear.Help("Sets the speed of the LFO used to 'smear' frequencies up and down in a slow vibrato effect.  \n"
                      "Increasing Smear increases the LFO rate.")
                .SeeAlso(vk_smear_skew, vk_motion);

        vk_motion.Help("Sets the depth of Smear LFO.  \n" 
                       "Increasing Emote increases the width of the vibrato effect.")
                 .SeeAlso(vk_motion_skew, vk_smear);
    }

    vibe_skew_page.Help("Skew controls for the first four parameters on the Vibe page.  \n  \n" 
                        "Turning a knob clockwise from noon will increase the value of the parameter for the right channel processor "
                        "while decreasing the value for the left channel processor. Turning counterclockwise decreases the value "
                        "for the right channel while increasing the value for the left channel.");
    {
        vk_density_skew.Help("Skew control for Perception.").SeeAlso(vk_density);
        vk_spread_skew.Help("Skew control for Focus.").SeeAlso(vk_spread);
        vk_sensitivity_skew.Help("Skew control for Empathy.").SeeAlso(vk_sensitivity);
        vk_decay_skew.Help("Skew control for Sympathy.").SeeAlso(vk_decay_skew);
    }

    rizz_skew_page.Help("Skew controls for all six paramters on the Rizz page.  \n  \n" 
                        "Turning a knob clockwise from noon will increase the value of the parameter for the right channel processor "
                        "while decreasing the value for the left channel processor. Turning counterclockwise decreases the value "
                        "for the right channel while increasing the value for the left channel.");
    {
        vk_shift_skew.Help("Skew control for Transpose.").SeeAlso(vk_shift);
        vk_warp_skew.Help("Skew control for Warp.").SeeAlso(vk_warp);
        vk_melt_skew.Help("Skew control for Melt.").SeeAlso(vk_melt);
        vk_spread_skew.Help("Skew control for Smear.").SeeAlso(vk_spread);
        vk_motion_skew.Help("Skew control for Emote.").SeeAlso(vk_motion);
        vk_ripple_skew.Help("Skew control for Sizzle.").SeeAlso(vk_ripple);
    }
}
} // namespace manual

} // namespace condolences