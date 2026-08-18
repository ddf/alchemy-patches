/**
 * Copyright 2026 Damien Quartz
 */

#include "daisy_seed.h"

namespace condolences
{
static constexpr size_t SpectrumSize = 2048;
static constexpr size_t Overlap = 4;

 /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
void Init(float sample_rate, size_t block_size);

/** Release resources */
void DeInit();

void SetDensity(float value);
void SetDecay(float value);
void SetSpacing(float value);
void SetSpread(float value);
void SetSmear(float value);
void SetMix(float value);
void SetMelt(float value);

void Update();

/**
 * Audio callback.
 */
void Process(daisy::AudioHandle::InputBuffer  in,
             daisy::AudioHandle::OutputBuffer out,
             size_t                           block_size);
} // namespace condolences_dsp