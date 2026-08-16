/**
 * Copyright 2026 Damien Quartz
 */

#include "condolences_dsp.h"
#include "Condolences.h"

namespace condolences
{
  using SampleArray = vessl::array<float>;
  using Condol = Condolences<float, 2048>;
  using Smoother = vessl::math::easing::smoother<float>;
// state
namespace
{
  Condol* condolences_;
  Smoother mix_;
}

void Init(float sample_rate, size_t block_size)
{
  condolences_ = Condol::create(sample_rate, block_size);
  condolences_->brightness() = 0.f;
  condolences_->spread() = 0.f;
  condolences_->spacing() = 1.f;
  condolences_->density() = 0.2f;
  condolences_->decay() = 1.f;
  condolences_->feedback() = 0.f;

  mix_.value = 0.5f;
  mix_.degree = 0.95f;
}

void DeInit()
{
  Condol::destroy(condolences_);
}

void SetDensity(float value)
{
  condolences_->density() = value;
}

void SetDecay(float value)
{
  condolences_->decay() = value;
}

void SetSpacing(float value)
{
  condolences_->spacing() = value;
}

void SetSpread(float value)
{
  condolences_->spread() = value;
}

void SetMix(float value)
{
  mix_ = value;
}

void SetFeedback(float value)
{
  condolences_->feedback() = value;
}

void Update()
{
}

void Process(
  daisy::AudioHandle::InputBuffer in, 
  daisy::AudioHandle::OutputBuffer out, 
  size_t block_size)
{
  SampleArray in_left(const_cast<float*>(in[0]), block_size);
  SampleArray in_right(const_cast<float*>(in[1]), block_size);
  SampleArray out_left(out[0], block_size);
  SampleArray out_right(out[1], block_size);

  // mixdown to mono, for now
  for(size_t i = 0; i < block_size; ++i)
  {
    out_left[i] = (in_left[i]+in_right[i])*0.5f;
  }

  condolences_->process(out_left, out_left);
  
  float l,r;
  float m = mix_.value;
  for(size_t i = 0; i < block_size; ++i)
  {
    vessl::sample::crossfade(in_left[i], out_left[i], m, &l);
    vessl::sample::crossfade(in_right[i], out_left[i], m, &r);
    out_left[i] = l;
    out_right[i] = r;
  }
}
}