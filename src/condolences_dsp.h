/**
 * Copyright 2026 Damien Quartz
 */

#include "daisy_seed.h"
#include "Condolences.h"

namespace condolences
{
    /**
     * @todo try 4096 spectrum size after switching over to 16-bit fixed point processing.
     */
    static constexpr size_t SpectrumSize = 2048;
    static constexpr size_t Overlap = 4;

    constexpr size_t GetBlockSize() { return Condolences<float, SpectrumSize, Overlap>::GenerateBlockSize; }
    constexpr float GetDensityMin() { return Condolences<float, SpectrumSize, Overlap>::DensityMin; }
    constexpr float GetDensityMax() { return Condolences<float, SpectrumSize, Overlap>::DensityMax; }

    /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
    void Init(float sample_rate);

    /** Release resources */
    void DeInit();

    void SetDensity(float value);
    void SetSpread(float value);
    void SetDecay(float value);
    void SetSpacing(float value);
    void SetMelt(float value);
    void SetSmear(float value);
    void SetMix(float value);

    float GetInputBandMagnitude(float freq);

    void Update();

    /**
     * Audio callback.
     */
    void Process(daisy::AudioHandle::InputBuffer in,
                 daisy::AudioHandle::OutputBuffer out,
                 size_t block_size);
} // namespace condolences_dsp