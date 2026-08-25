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

    enum class Mode : uint8_t
    {
        TrueStereo,   // left/right sent to dedicated processors output as a stereo pair
        ParallelMono, // left/right summed to mono, which is sent to both processors output as a stereo pair
        SeriesMono,   // left/right summed to mono, sent thru both processors in series, output as mono

        Count
    };

    constexpr size_t GetBlockSize() { return Condolences<float, SpectrumSize, Overlap>::GenerateBlockSize; }
    constexpr float GetDensityMin() { return Condolences<float, SpectrumSize, Overlap>::DensityMin; }
    constexpr float GetDensityMax() { return Condolences<float, SpectrumSize, Overlap>::DensityMax; }

    /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
    void Init(float sample_rate);

    /** Release resources */
    void DeInit();

    void SetDensity(float x, float y);
    void SetSpread(float x, float y);
    void SetDecay(float x, float y);
    void SetSpacing(float x, float y);
    void SetMelt(float x, float y);
    void SetSmear(float x, float y);
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
} // namespace condolences_dsp