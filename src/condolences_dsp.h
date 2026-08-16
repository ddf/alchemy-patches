/**
 * Copyright 2026 Damien Quartz
 */

#include "daisy_seed.h"

namespace condolences
{
 /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
void Init(float sample_rate, size_t block_size);

/** Release resources */
void DeInit();

void SetDensity(float value);
void SetDecay(float value);
void SetSpacing(float value);
void SetSpread(float value);
void SetMix(float value);
void SetFeedback(float value);

void Update();

/**
 * Audio callback. Pass directly to hw.StartAudio(eq_dsp::Process).
 * Processes left and right independently through three series biquads each.
 */
void Process(daisy::AudioHandle::InputBuffer  in,
             daisy::AudioHandle::OutputBuffer out,
             size_t                           block_size);
} // namespace condolences_dsp