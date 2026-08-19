/**
 * Copyright 2026 Damien Quartz
 */

#include "condolences_dsp.h"
#include "Condolences.h"

namespace condolences
{
  using SampleArray = vessl::array<float>;
  using Condol = Condolences<float, SpectrumSize, Overlap>;
  using Smoother = vessl::math::easing::smoother<float>;

// state
namespace
{
  Condol* condolences_[2];
  Smoother mix_;
  float mix_raw_;
}

void Init(float sample_rate, size_t block_size)
{
  condolences_[0] = Condol::create(sample_rate, block_size);
  condolences_[1] = Condol::create(sample_rate, block_size);

  mix_.value = 0.5f;
}

void DeInit()
{
  Condol::destroy(condolences_[0]);
  Condol::destroy(condolences_[1]);
}

void SetDensity(float value)
{
  condolences_[0]->density() = value;
  condolences_[1]->density() = value;
}

void SetDecay(float value)
{
  condolences_[0]->decay() = value;
  condolences_[1]->decay() = value;
}

void SetSpacing(float value)
{
  condolences_[0]->spacing() = value;
  condolences_[1]->spacing() = value;
}

void SetSpread(float value)
{
  condolences_[0]->spread() = value;
  condolences_[1]->spread() = value;
}

void SetSmear(float value)
{
  condolences_[0]->smear() = value;
  condolences_[1]->smear() = value;
}

void SetMelt(float value)
{
  condolences_[0]->melt() = value*0.98f;
  condolences_[1]->melt() = value*0.98f;
}

void SetMix(float value)
{
  mix_raw_ = value;
}

void Update()
{
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

  // // mixdown to mono, for now
  // for(size_t i = 0; i < block_size; ++i)
  // {
  //   out_left[i] = (in_left[i]+in_right[i])*0.5f;
  // }

  condolences_[0]->process(in_left, out_left);
  condolences_[1]->process(in_right, out_right);

  mix_ = mix_raw_;
  
  float l,r;
  float m = mix_.value;
  for(size_t i = 0; i < block_size; ++i)
  {
    vessl::sample::crossfade(in_left[i], out_left[i], m, &l);
    vessl::sample::crossfade(in_right[i], out_right[i], m, &r);
    out_left[i] = l;
    out_right[i] = r;
  }
}
}