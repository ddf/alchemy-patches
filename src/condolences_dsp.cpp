/**
 * Copyright 2026 Damien Quartz
 */

#include "condolences_dsp.h"
#include "Condolences.h"

namespace condolences
{
  using sample_t = float;
  using complex_t = vessl::transform::complex<sample_t>;
  using SampleArray = vessl::array<sample_t>;
  using Condol = Condolences<sample_t, SpectrumSize, Overlap>;
  using Sympathies = typename Condol::Sympathies;
  using SpectralGen = typename Condol::Sympathies::SpectralGen;
  using FrequencyBand = typename SpectralGen::frequency_band;
  using SpectralData = typename SpectralGen::data;
  using Limiter = vessl::processors::limiter<sample_t>;
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

  // these wind up in DTCMRAM without any markup, which is good.
  
  // shared between both instances of Condol.  
  sample_t input_window[Condol::AnalysisSize];
  sample_t input_analysis[Condol::AnalysisSize];

  // split between the two instances of Condol
  sample_t input_buffer[Condol::AnalysisSize*2];
  complex_t input_spectrum[Condol::AnalysisSize];

  sample_t output_window[SpectrumSize];
  complex_t output_spectrum[SpectrumSize];
  FrequencyBand output_bands[SpectrumSize];

  constexpr size_t sample_data_count = SpectrumSize*Overlap*2*2;
  // this _might_ fit into DTCMRAM with everything else if we can switch to int16_t for sample type.
  // altho, if we then bumped up spectrum size to 4096, we'd probably run out of space again.
  // sample_t output_sample_data[SpectrumSize*Overlap*2*2];
  sample_t* output_sample_data = nullptr;

  // we allocate these, but don't directly modify them
  SpectralGen* spectral_[2];
  Sympathies* sympathies_[2];

  // we modify these
  Condol* condolences_[2];
  Limiter limiter_[2];
  Mode mode_;
  Smoother mix_smooth_[2];
  float mix_[2];
}

void Init(float sample_rate)
{
  //memset(data0, 0, sizeof(int16_t)*(SpectrumSize*Overlap*2));
  //memset(data1, 0, sizeof(int16_t)*(SpectrumSize*Overlap*2));

  output_sample_data = new sample_t[sample_data_count];

  constexpr size_t sample_data_size = sizeof(sample_t)*sample_data_count;
  memset(output_sample_data, 0, sample_data_size);

  vessl::sample::windows::render(vessl::sample::windows::type::hann, input_window, Condol::AnalysisSize);
  vessl::sample::windows::render(vessl::sample::windows::type::triangle, output_window, SpectrumSize);

  SpectralData data0 = {
    vessl::array<FrequencyBand>(output_bands, SpectrumSize/2),
    vessl::array<complex_t>(output_spectrum, SpectrumSize/2),
    vessl::array<sample_t>(output_sample_data, SpectrumSize*Overlap*2),
    vessl::array<sample_t>(output_window, SpectrumSize)
  };

  SpectralData data1 = {
    vessl::array<FrequencyBand>(output_bands + data0.bands.size(), data0.bands.size()),
    vessl::array<complex_t>(output_spectrum + data0.spectrum.size(), data0.spectrum.size()),
    vessl::array<sample_t>(output_sample_data + data0.signal.size(), data0.signal.size()),
    vessl::array<sample_t>(output_window, SpectrumSize)
  };

  spectral_[0] = new SpectralGen(data0, sample_rate);
  spectral_[1] = new SpectralGen(data1, sample_rate);

  sympathies_[0] = new Sympathies(spectral_[0], sample_rate);
  sympathies_[1] = new Sympathies(spectral_[1], sample_rate);

  condolences_[0] = new Condol(
    sample_rate, 
    input_window, 
    input_buffer, 
    input_analysis, 
    input_spectrum, 
    sympathies_[0]
  );

  condolences_[1] = new Condol(
    sample_rate, 
    input_window, 
    input_buffer + Condol::AnalysisSize, 
    input_analysis, 
    input_spectrum + Condol::AnalysisSize,
    sympathies_[1]
  );

  mode_ = Mode::TrueStereo;
}

void DeInit()
{
  delete[] output_sample_data;

  delete spectral_[0];
  delete spectral_[1];

  delete sympathies_[0];
  delete sympathies_[1];

  delete condolences_[0];
  delete condolences_[1];
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

void SetSensitivity(float x, float y)
{
  condolences_[0]->sensitivity() = x;
  condolences_[1]->sensitivity() = y;
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
    out_left[i] = limiter_[0].process(l);
    out_right[i] = limiter_[1].process(r);
  }
}
}