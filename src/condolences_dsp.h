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

#include "daisy_seed.h"
#include "Condolences.h"

namespace condolences
{
namespace dsp
{
    static constexpr size_t SpectrumSize = 4096;
    static constexpr size_t Overlap = 4;

    enum class Mode : uint8_t
    {
        TrueStereo,   // left/right sent to dedicated processors output as a stereo pair
        ParallelMono, // left/right summed to mono, which is sent to both processors output as a stereo pair
        SeriesMono,   // left/right summed to mono, sent thru both processors in series, output as mono

        Count
    };

    constexpr size_t GetBlockSize() { return Condolences<float, SpectrumSize, Overlap>::BlockSize;  }
    constexpr float GetDensityMin() { return Condolences<float, SpectrumSize, Overlap>::DensityMin; }
    constexpr float GetDensityMax() { return Condolences<float, SpectrumSize, Overlap>::DensityMax; }

    /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
    void Init(float sample_rate);

    /** Release resources */
    void DeInit();

    void SetDensity(float x, float y);
    void SetSpread(float x, float y);
    void SetDamping(float x, float y);
    void SetSensitivity(float x, float y);
    void SetShift(float x, float y);
    void SetSpacing(float x, float y);
    void SetMelt(float x, float y);
    void SetRipple(float x, float y, float xd, float yd);
    void SetSmear(float x, float y);
    void SetMotion(float x, float y);
    void SetMix(float x, float y);
    void SetMode(Mode m);

    float GetInputBandMagnitude(float freq);

    void Update();

    /**
     * Audio callback.
     */
    void Process(daisy::AudioHandle::InputBuffer in,
                 daisy::AudioHandle::OutputBuffer out,
                 size_t block_size);
} // namespace dsp
} // namespace condolences