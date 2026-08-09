/**
 * Copyright 2026 Damien Quartz
 */

#include "condolences_dsp.h"
#include "Condolences.h"

namespace condolences_dsp
{
  using SampleArray = vessl::array<float>;
  using Condol = Condolences<float, 2048>;
// state
namespace
{
  Condol* condolences_;
}

void Init(float sample_rate)
{
  condolences_ = Condol::create(sample_rate);
  condolences_->brightness() = 0.f;
  condolences_->spread() = 0.f;
  condolences_->spacing() = 1.f;
  condolences_->density() = 0.2f;
  condolences_->decay() = 1.f;
  condolences_->feedback() = 0.f;
}

void DeInit()
{
  Condol::destroy(condolences_);
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
  for(size_t i = 0; i < block_size; ++i)
  {
    vessl::sample::crossfade(in_left[i], out_left[i], 1.f, &l);
    vessl::sample::crossfade(in_right[i], out_left[i], 1.f, &r);
    out_left[i] = l;
    out_right[i] = r;
  }
}
}