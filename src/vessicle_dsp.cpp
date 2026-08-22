/**
 * Copyright 2026 Damien Quartz
 * 
 * Firmware for testing various vessicle / vessl classes in isolation.
 */

#include "vessicle_dsp.h"
#include "vessl/vessl.h"
#include "SpectralGenerator.h"

using SampleArray = vessl::array<float>;
using SpectralGen = SpectralGenerator<float, 4096>;
using WindowType = vessl::sample::windows::type;

namespace vessicle_dsp
{

namespace
{
  constexpr uint8_t parameters_size_ = static_cast<uint8_t>(Parameter::Count);
  float parameters_[parameters_size_];
  SpectralGen* spectral_gen_;
}

void Init(float sample_rate)
{
  spectral_gen_ = SpectralGen::create(sample_rate, WindowType::triangle);
  memset(&parameters_, 0, parameters_size_*sizeof(float));
}

void SetParameter(Parameter param, float value)
{
  uint8_t idx = static_cast<uint8_t>(param);
  if (idx < parameters_size_)
  {
    parameters_[idx] = value;
  }
}

void Update()
{
  float freq = vessl::math::lerp(120.f, 8800.f, parameters_[0]);
  float decay = vessl::math::lerp(0.9f, 0.999f, parameters_[1]);
  size_t bidx = spectral_gen_->get_band_index(freq);

  for(size_t i = 1; i < spectral_gen_->get_band_count(); ++i)
  {
    auto& band = spectral_gen_->get_band(i);
    if (i == bidx)
    {
      band.set_magnitude(1.f);
    }
    else
    {
      band.scale(decay);
    }
  }
}

void Process(
  daisy::AudioHandle::InputBuffer in, 
  daisy::AudioHandle::OutputBuffer out, 
  size_t block_size
)
{
  SampleArray in_left(const_cast<float*>(in[0]), block_size);
  SampleArray in_right(const_cast<float*>(in[1]), block_size);
  SampleArray out_left(out[0], block_size);
  SampleArray out_right(out[1], block_size);

  auto lw = out_left.make_writer();
  auto rw = out_right.make_writer();
  while(lw && rw)
  {
    float v = spectral_gen_->generate();
    lw << v;
    rw << v;
  }
}

}
