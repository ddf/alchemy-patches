/**
 * Copyright 2026 Damien Quartz
 * 
 * Firmware for testing various vessicle / vessl classes in isolation.
 */

#include "attributes.h"
#include "vessicle_dsp.h"
#include "vessl/vessl.h"
#include "SpectralGenerator.h"
#include "profiler.h"

static constexpr size_t SpectrumSize = 4096;
static constexpr size_t Overlap = 4;

using SampleArray = vessl::array<float>;
using SpectralGen = SpectralGenerator<float, SpectrumSize, Overlap>;
using WindowType = vessl::sample::windows::type;
using namespace alchemy;

namespace vessicle_dsp
{

namespace
{
  ALCHEMY_SRAM constexpr uint8_t parameters_size_ = static_cast<uint8_t>(Parameter::Count);
  ALCHEMY_SRAM float parameters_[parameters_size_];

  // generate() is called every sample.
  SpectralGen* spectral_gen_s_;
  // generate(array) is called with blocks
  SpectralGen* spectral_gen_b_;

  // data for spectral_gen_b_
  SpectralGen::frequency_band bands_data[SpectrumSize/2];
  SpectralGen::sample_t  window_data[SpectrumSize];

  SpectralGen::complex_t spectrum_data[SpectrumSize/2];
  SpectralGen::sample_t  sample_data[SpectrumSize*2];

  float block_out_[SpectralGen::block_size];
  size_t block_out_read_idx_;

  Profiler::Timer tm_gen_left("Gen Left");
  Profiler::Timer tm_gen_right("Gen Right");
}

uint32_t GetBlockSize()
{
  return static_cast<uint32_t>(SpectralGen::block_size);
}

void Init(float sample_rate, size_t block_size)
{
  spectral_gen_s_ = SpectralGen::create(sample_rate, WindowType::triangle);

  SpectralGen::data gen_data = {
    vessl::array<SpectralGen::frequency_band>(bands_data, SpectrumSize/2),
    vessl::array<SpectralGen::complex_t>(spectrum_data, SpectrumSize/2),
    vessl::array<SpectralGen::sample_t>(sample_data, SpectrumSize),
    vessl::array<SpectralGen::sample_t>(window_data, SpectrumSize),
    vessl::array<SpectralGen::sample_t>(sample_data+SpectrumSize, SpectrumSize)
  };
  vessl::sample::windows::render(WindowType::triangle, gen_data.window);
  spectral_gen_b_ = new SpectralGen(gen_data, sample_rate);

  memset(parameters_, 0, parameters_size_*sizeof(float));
  memset(block_out_, 0, sizeof(float)*SpectralGen::block_size);
  block_out_read_idx_ = SpectralGen::block_size;
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
  size_t bidx = spectral_gen_s_->get_band_index(freq);

  for(size_t i = 1; i < spectral_gen_s_->get_band_count(); ++i)
  {
    auto& band_s = spectral_gen_s_->get_band(i);
    auto& band_b = spectral_gen_b_->get_band(i);
    if (i == bidx)
    {
      band_s.set_magnitude(1.f);
      band_b.set_magnitude(1.f);
    }
    else
    {
      band_s.scale(decay);
      band_b.scale(decay);
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

  
  {
    Profiler::ScopedTimer st(tm_gen_left);
    auto lw = out_left.make_writer();
    while(lw)
    {
      float v = spectral_gen_s_->generate();
      lw << v;
    }
  }


  {
    Profiler::ScopedTimer st(tm_gen_right);
    auto rw = out_right.make_writer();
    while(rw)
    {
      if (block_out_read_idx_ == SpectralGen::block_size)
      {
        vessl::array<float> block(block_out_, SpectralGen::block_size);
        spectral_gen_b_->generate(block);
        block_out_read_idx_ = 0;
      }
      rw << block_out_[block_out_read_idx_++];
    }
  }
}

} // namespace vessicle_dsp
