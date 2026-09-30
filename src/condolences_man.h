#pragma once

#include "alchemy/host_link/host.h"
#include "alchemy/surface/virtual_knob.h"
#include "alchemy/surface/jack.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/manual.h"
#include "condolences_cfg.h"

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
constexpr Jack jk_in_l = Jack("IN_L", "In L", JackSig::AudioIn).Short("IN L");
constexpr Jack jk_in_r = Jack("IN_R", "In R", JackSig::AudioIn).Short("IN R");
constexpr Jack jk_cv_1 = Jack("J3", "Perception CV", JackSig::CvBi).Short("PRCPTN").SeeAlso(vk_density);
constexpr Jack jk_cv_2 = Jack("J4", "Empathy CV", JackSig::CvBi).Short("EMPTHY").SeeAlso(vk_sensitivity);
constexpr Jack jk_cv_3 = Jack("J5", "Focus CV", JackSig::CvBi).Short("FOCUS").SeeAlso(vk_spread);
constexpr Jack jk_cv_4 = Jack("J6", "Sympathy CV", JackSig::CvBi).Short("SYMPA").SeeAlso(vk_decay);
constexpr Jack jk_cv_5 = Jack("J7", "Transpose CV", JackSig::CvBi).Short("TRNSP").SeeAlso(vk_shift);
constexpr Jack jk_cv_6 = Jack("J8", "Warp CV", JackSig::CvBi).Short("WARP").SeeAlso(vk_warp);
constexpr Jack jk_out_l = Jack("OUT_L", "Out L", JackSig::AudioOut).Short("OUT L");
constexpr Jack jk_out_r = Jack("OUT_R", "Out R", JackSig::AudioOut).Short("OUT R");

void AttachTo(hostlink::Host& host)
{
    thee_manual .Tagline("Stereo spectral processor loosely based on the phenomenon of sympathetic vibration.")
                .Preamble("Your signal will be analyzed with varying degrees of attention to ascertain the frequency of the vibrations contained therein. "
                         "We shall then attempt to reproduce your signal by responding to its content with empathy and sympathy. "
                         "However, we are _not_ you and are bound to introduce our own interpretations, extensions, transformations, and misunderstandings. "
                         "You may then mix your signal with ours to produce a novel perspective that is greater than the sum of its parts.  \n  \n"
                         "In other words: we are sorry for your loss, resonate with your grief, and transform it into psychedelia.")
                .Section("overview", "Overview", 
                        "**Condolences** is a [phase vocoder](https://en.wikipedia.org/wiki/Phase_vocoder) with an analysis frame size of 1024 samples, "
                        "a synthesis frame size of 4096 samples, and an overlap of 512 samples. This means that every 512 samples, the input signal "
                        "is analyzed and transferred to the synthesis spectrum, which is then resynthesized to produce the next frame of output samples. "
                        "Spectral information is modified during both the transfer from analysis to synthesis, as well as prior to synthesizing a frame. "
                        "The parameters on the Vibe and Rizz pages give the performer control over this modification process."
                        )
                .Section("analysis-to-synthesis", "Analysis -> Synthesis",
                        "After analysis, we conditionally transfer all 512 frequency bands from the analysis spectrum to the synthesis spectrum. "
                        "In order to be transferred, an analysis band must have a magnitude greater than the threshold set by the **Perception** parameter (Vibe page, top left). "
                        "After passing the Perception test, the target frequency band in the synthesis spectrum is determined by first shifting the frequency "
                        "of the analysis band using the **Transpose** parameter (Rizz page, top left), obtaining a target band index, " 
                        "and then using the **Warp** parameter (Rizz page, top right) to 'linearize' it. "
                        "In oscillator language, *Transpose* is akin to shifting the frequency of each analysis band using exponential FM and *Warp* is akin to linear FM."
                        )
                .Section("sympathetic-vibration", "Sympathetic Vibration",
                        "Once the target frequency band in the synthesis spectrum has been calculated, it is potentially 'excited' by the analysis frequency band. "
                        "The **Empathy** parameter (Vibe page, middle left) is essentially how responsive synthesis is to the input signal. Empathy sets a magnitude "
                        "threshold that the synthesis band must be _below_ in order for the analysis band value to replace it. High Empathy means that if an analysis "
                        "band passes the Perception test, it will be almost certainly be transferred to the synthesis spectrum. Low Empathy means that synthesis bands will not change "
                        "until the energy in them has decayed enough for analysis energy to replace it. The **Sympathy** parameter (Vibe page, middle right) is what controls "
                        "the rate of this decay. High Sympathy results in long decays in the synthesis spectrum, creating reverb-like effects. Low Sympathy decays quickly, "
                        "allowing the synthesis spectrum to 'keep up' with high Empathy analysis.")
                .Section("focus", "Focus",
                        "The final step in transferring the analysis spectrum to the synthesis spectrum is to _widen_ the impact of analysis bands on the sythesis spectrum. "
                        "The **Focus** parameter (Vibe page, top right) sets a falloff amount that controls how much of the analysis band is _also_ placed into the four adjacent "
                        "bands above and below the target synthesis band. Higher Focus tends to improve the accuracy of resynthesis because the the bandwidth of one synthesis band "
                        "is 4 times that of one analysis band. In other words, a single analysis band represents the energy in a range of frequencies that is represented in the synthesis "
                        "spectrum by 4 adjacent frequency bands."
                        )
                .Section("interpretation", "Interpretation",
                         "As with any attempt to empathize or sympathize with another person, our attempt gets some things wrong. In this case, on purpose. "
                         "Right before synthesizing a new frame of audio we massage the synthesis spectrum in three ways: Melt, Sizzle, and Smear.  \n  \n"
                         "First, the magnitudes of the synthesis frequency bands are run through a low pass filter in reverse order. " 
                         "The **Melt** parameter (Rizz page, middle left) controls the cutoff of this filter. As Melt increases the cutoff lowers, "
                         "smoothing out jumps in magnitude from one band to the next. The effect is that the frequency content of the spectrum sounds "
                         "as if it is 'melting' downwards in pitch. High Melt also tends to remove energy from the spectrum, making it decay more quickly.  \n  \n"
                         "After melting the spectrum, _sizzle_ is applied. The **Sizzle** parameter (Rizz page, bottom left) controls modulation depth of an LFO that is applied "
                         " to each band's magnitude. The amount of modulation also decreases with the magnitude of the band, so we don't introduce too much signal where there was none. "
                        )
                .Section("misunderstanding", "Misunderstanding",
                         "Finally, the spectrum is 'smeared' by running the complex representation through a high pass filter whose cutoff is modulated by an LFO. "
                         "The direction in which we process the spectrum is determined by the phase of the LFO. The **Smear** parameter (Rizz page, middle right) controls the speed of the LFO "
                         "and the **Emote** parameter (Rizz page, bottom right) controls the depth of filter cutoff modulation. The effect is that the frequency content of the synthesized signal sounds "
                         "as if it is swimming up and down, a bit like warble on a vinyl record, but can be made much more extreme."
                        )
                .Section("resynthesizing", "Resynthesizing",
                         "The operations of Interpretation and Misunderstanding occur in-place on the spectral information. This means that they interact with each other and with all other parameters "
                         "in sometimes unexpected ways. We recommend changing things slowly to find interesting combinations. " 
                         "After all of this faffing about, the output signal is synthesized using a standard overlap-add algorithm."
                        )
                .Section("modes", "Modes",
                         "By default, Condolences runs two parallel instances of the above effect on the left and right audio inputs of the Alchemy Lab. "
                         "This means you can use it as a stereo signal processor or as two mono processors. "
                         "There are two additional modes available that can be set from the Config Settings page: Parallel Mono and Series Mono. "
                         "In **Parallel Mono** mode, the two input channels are summed to one mono signal, which is then run through the left and right processors in parallel and output "
                         "to the left and right outputs as in Stereo mode. In **Series Mono** mode, the two input channels are summed to one mono signal, which is then run through "
                         "the left and right processors in series. That is: input -> mono -> left processor -> right processor -> output. In this mode, the left and right audio outputs "
                         "of Alchemy Lab will be identical."
                        );

    host.Attach(thee_manual);
    host.Jacks(jk_cv_1, jk_cv_2, jk_cv_3, jk_cv_4, jk_cv_5, jk_cv_6);

    config::settings::mode.Help("**Stereo Mode** runs two parallel instances of Condolences on the left and right audio inputs. "
                                "**Parallel Mono** sums the input to mono and processes it in parallel. "
                                "**Series Mono** sums the input to mono and processes it through both processors in series. Left and Right output are the same.");

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