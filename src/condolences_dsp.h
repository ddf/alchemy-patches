/**
 * Copyright 2026 Damien Quartz
 */

#include "daisy_seed.h"

namespace condolences
{
// 1024 with overlap of 2 is best we can do in stereo for now.
// would like to have an Overlap of at least four.
static constexpr size_t SpectrumSize = 1024;
static constexpr size_t Overlap = 2;

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