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
  /** 
   * @todo allocate all memory for Condolences instances from here instead of using create.
   * embed some of it in the binary for faster access.
   * goal would be to get us running with spectrum size 4096 and overlap 4.
   * 
   * @todo there is room for the signal arrays required by vessl::spectral if we work in 16-bit fixed point.
   * however, to do this we will need to add 16-bit fixed point support to vessl,
   * and then add overloads for the 16-bit ARM DSP transforms.
   * It would be a good feature add to vessl, so I think worth doing at some point.
   */
  //int16_t __attribute__((section(".text"))) data0[SpectrumSize*Overlap*2];
  //int16_t __attribute__((section(".text"))) data1[SpectrumSize*Overlap*2];

  Condol* condolences_[2];
  Mode mode_;
  Smoother mix_smooth_[2];
  float mix_[2];
}

void Init(float sample_rate)
{
  //memset(data0, 0, sizeof(int16_t)*(SpectrumSize*Overlap*2));
  //memset(data1, 0, sizeof(int16_t)*(SpectrumSize*Overlap*2));

  condolences_[0] = Condol::create(sample_rate);
  condolences_[1] = Condol::create(sample_rate);

  mode_ = Mode::TrueStereo;
}

void DeInit()
{
  Condol::destroy(condolences_[0]);
  Condol::destroy(condolences_[1]);
}


void SetMode(Mode m)
{
  mode_ = m;
}

void SetDensity(float x, float y)
{
  condolences_[0]->density() = x;
  condolences_[1]->density() = y;
}

void SetDecay(float x, float y)
{
  condolences_[0]->decay() = x;
  condolences_[1]->decay() = y;
}

void SetSpacing(float x, float y)
{
  condolences_[0]->spacing() = vessl::math::clamp_delta(x);
  condolences_[1]->spacing() = vessl::math::clamp_delta(y);
}

void SetSpread(float x, float y)
{
  condolences_[0]->spread() = vessl::math::clamp_delta(x);
  condolences_[1]->spread() = vessl::math::clamp_delta(y);
}

void SetSmear(float x, float y)
{
  condolences_[0]->smear() = vessl::math::clamp_delta(x);
  condolences_[1]->smear() = vessl::math::clamp_delta(y);
}

void SetMelt(float x, float y)
{
  condolences_[0]->melt() = vessl::math::clamp_delta(x)*0.98f;
  condolences_[1]->melt() = vessl::math::clamp_delta(y)*0.98f;
}

void SetMix(float x, float y)
{
  mix_[0] = vessl::math::clamp_delta(x);
  mix_[1] = vessl::math::clamp_delta(y);
}

float GetInputBandMagnitude(float freq)
{
  return condolences_[0]->get_input_band_magnitude(freq);
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

  switch(mode_)
  {
    case Mode::TrueStereo:
    {
      condolences_[0]->process(in_left, out_left);
      condolences_[1]->process(in_right, out_right);
    }
    break;

    case Mode::ParallelMono:
    {
      in_left.copy_to(out_left);
      out_left.add(in_right).scale(0.5f);
      condolences_[1]->process(out_left, out_right);
      condolences_[0]->process(out_left, out_left);
    }
    break;

    case Mode::SeriesMono:
    {
      in_left.copy_to(out_left);
      out_left.add(in_right).scale(0.5f);
      condolences_[0]->process(out_left, out_left);
      out_left.scale(0.5f);
      condolences_[1]->process(out_left, out_left);
      out_left.copy_to(out_right);
    }
    break;

    default: break;
  }

  mix_smooth_[0] = mix_[0];
  mix_smooth_[1] = mix_[1];
  
  float l,r;
  float ml = mix_smooth_[0].value;
  float mr = mix_smooth_[1].value;
  for(size_t i = 0; i < block_size; ++i)
  {
    vessl::sample::crossfade<vessl::math::easing::quad::in_out>(in_left[i], out_left[i], ml, &l);
    vessl::sample::crossfade<vessl::math::easing::quad::in_out>(in_right[i], out_right[i], mr, &r);
    out_left[i] = l;
    out_right[i] = r;
  }
}
}