/**
 * Copyright 2026 Damien Quartz
 */

#include "daisy_seed.h"

namespace condolences_dsp
{
 /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
void Init(float sample_rate);

/** Release resources */
void DeInit();

/**
 * Audio callback. Pass directly to hw.StartAudio(eq_dsp::Process).
 * Processes left and right independently through three series biquads each.
 */
void Process(daisy::AudioHandle::InputBuffer  in,
             daisy::AudioHandle::OutputBuffer out,
             size_t                           block_size);
} // namespace condolences_dsp