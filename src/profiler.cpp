#include "profiler.h"
#include "alchemy/host_link/component_writer.h"
#include "util/CpuLoadMeter.h"
#include "sys/system.h"
#include <stdio.h>
#include <cstring>

static volatile float s_cpu_min = 0; 
static volatile float s_cpu_max = 0;
static volatile float s_cpu_avg = 0;

// percentage of one audio block each timer takes
static volatile float s_timer_cpu[alchemy::Profiler::kMaxTimers];
static alchemy::Profiler::Timer* s_timers[alchemy::Profiler::kMaxTimers];
static size_t s_timer_count = 0;

static daisy::AudioHandle::AudioCallback audio_callback;
static daisy::CpuLoadMeter cpu_load_meter;
static float us_per_block_inv; // for timers

  
void alchemy::Profiler::Init(float sampleRateInHz, int blockSizeInSamples, daisy::AudioHandle::AudioCallback user_audio_callback)
{
  s_cpu_min  = 0;
  s_cpu_max  = 0;
  s_cpu_avg  = 0;
  audio_callback = user_audio_callback;
  cpu_load_meter.Init(sampleRateInHz, blockSizeInSamples);

  const float seconds_per_block = float(blockSizeInSamples) / sampleRateInHz;
  const float us_per_second     = 1000 * 1000;
  us_per_block_inv              = 1.0f / (us_per_second * seconds_per_block);
}

void alchemy::Profiler::Process(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t block_size)
{
  cpu_load_meter.OnBlockStart();
  audio_callback(in, out, block_size);
  cpu_load_meter.OnBlockEnd();
  s_cpu_min = cpu_load_meter.GetMinCpuLoad();
  s_cpu_max = cpu_load_meter.GetMaxCpuLoad();
  s_cpu_avg = cpu_load_meter.GetAvgCpuLoad();
}

namespace alchemy
{
  Profiler::Timer::Timer(const char* name) : name_(name)
  {
    idx_ = s_timer_count++;
    s_timers[idx_] = this;
    s_timer_cpu[idx_] = 0.f;
  }

  void Profiler::Timer::Start()
  {
    begin_ = daisy::System::GetUs();
  }

  void Profiler::Timer::Stop()
  {
    uint32_t end = daisy::System::GetUs();
    if (idx_ < kMaxTimers)
    {
      s_timer_cpu[idx_] = (end-begin_)*us_per_block_inv;
    }
  }

  void Profiler::SettingsPage::Serialize(uint8_t *out) const
  {
    float cpu_min = s_cpu_min;
    float cpu_max = s_cpu_max;
    float cpu_avg = s_cpu_avg;
    std::memcpy(out+0, &cpu_min, 4);
    std::memcpy(out+4, &cpu_max, 4);
    std::memcpy(out+8, &cpu_avg, 4);
    for(size_t i = 0; i < s_timer_count; ++i)
    {
      float tim = s_timer_cpu[i];
      std::memcpy(out+12+(i*4), &tim, 4);
    }
  }

uint32_t Profiler::SettingsPage::SchemaHash() const
{
  return 0xDEADBEEFu;
}

bool Profiler::SettingsPage::Describe(hostlink::ComponentWriter &w) const
{
  static char timer_id[8];

  w.Label("Profiler");

  bool ok = w.Field("cpu.min", "CPU Min Load", 0, hostlink::FieldType::F32, 0);
      ok &= w.Field("cpu.max", "CPU Max Load", 4, hostlink::FieldType::F32, 4);
      ok &= w.Field("cpu.avg", "CPU Avg Load", 8, hostlink::FieldType::F32, 8);

  for(size_t i = 0; i < s_timer_count; ++i)
  {
    const Timer* desc = s_timers[i];
    sprintf(timer_id, "tm.%d", i);
    ok &= w.Field(timer_id, desc->name_, 12 + i*sizeof(float), alchemy::hostlink::FieldType::F32, 0);
  }
  return ok;
}
} // namespace alchemy