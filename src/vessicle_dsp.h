/**
 * Copyright 2026 Damien Quartz
 * 
 * Firmware for testing various vessicle / vessl classes in isolation.
 */

#include "daisy_seed.h"

namespace vessicle_dsp
{

enum class Parameter : uint8_t
{
  A = 0, 
  B, 
  C, 
  D, 
  E, 
  F,

  Count,
  Max = UINT8_MAX
};

 /** Cache the sample rate, allocate resources. Call once after hw.Init(). */
void Init(float sample_rate);

/** Set a parameter value */
void SetParameter(Parameter param, float value);

/** Control callback. Call after setting parameters. */
void Update();

/**
 * Audio callback. Pass directly to hw.StartAudio(eq_dsp::Process).
 * Processes left and right independently through three series biquads each.
 */
void Process(daisy::AudioHandle::InputBuffer  in,
             daisy::AudioHandle::OutputBuffer out,
             size_t                           block_size);
} // namespace condolences_dsp

