/**
 * Copyright 2026 Damien Quartz
 */

#include "daisy_seed.h"
#include "Condolences.h"

namespace condolences
{
static constexpr size_t SpectrumSize = 1024;
static constexpr size_t Overlap = 4;

constexpr size_t GetBlockSize() { return Condolences<float, SpectrumSize, Overlap>::generate_block_size; }
constexpr float GetDensityMin() { return Condolences<float, SpectrumSize, Overlap>::density_min; }
constexpr float GetDensityMax() { return Condolences<float, SpectrumSize, Overlap>::density_max; }

 /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
void Init(float sample_rate);

/** Release resources */
void DeInit();
 
void SetDensity(float value);
void SetDecay(float value);
void SetSpacing(float value);
void SetSpread(float value);
void SetSmear(float value);
void SetMix(float value);
void SetMelt(float value);

float GetInputBandMagnitude(float freq);

void Update();

/**
 * Audio callback.
 */
void Process(daisy::AudioHandle::InputBuffer  in,
             daisy::AudioHandle::OutputBuffer out,
             size_t                           block_size);
} // namespace condolences_dsp