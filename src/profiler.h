#pragma once

#include "alchemy/hw/alchemy_lab.h"
#include "alchemy/surface/serializable.h"
#include "alchemy/host_link/component_writer.h"
#include "util/CpuLoadMeter.h"
#include <stdio.h>
#include <cstring>

namespace alchemy
{

namespace Profiler
{
  static volatile float s_cpu_min = 0; 
  static volatile float s_cpu_max = 0;
  static volatile float s_cpu_avg = 0;

  daisy::AudioHandle::AudioCallback audio_callback;
  daisy::CpuLoadMeter cpu_load_meter;

  void Init(float sampleRateInHz, int blockSizeInSamples, daisy::AudioHandle::AudioCallback user_audio_callback)
  {
    s_cpu_min  = 0;
    s_cpu_max  = 0;
    s_cpu_avg  = 0;
    audio_callback = user_audio_callback;
    cpu_load_meter.Init(sampleRateInHz, blockSizeInSamples);
  }

  static void Process(
    daisy::AudioHandle::InputBuffer  in,
    daisy::AudioHandle::OutputBuffer out,
    size_t                           block_size)
  {
    cpu_load_meter.OnBlockStart();
    audio_callback(in, out, block_size);
    cpu_load_meter.OnBlockEnd();
    s_cpu_min = cpu_load_meter.GetMinCpuLoad();
    s_cpu_max = cpu_load_meter.GetMaxCpuLoad();
    s_cpu_avg = cpu_load_meter.GetAvgCpuLoad();
  }

  class SettingsPage : public Serializable
  {
  public:
    size_t SerializedSize() const override 
    { 
      // cpu load meter
      return sizeof(float)*3; 
    }

    void Serialize(uint8_t* out) const override
    {
      float cpu_min = s_cpu_min;
      float cpu_max = s_cpu_max;
      float cpu_avg = s_cpu_avg;
      std::memcpy(out+0, &cpu_min, 4);
      std::memcpy(out+4, &cpu_max, 4);
      std::memcpy(out+8, &cpu_avg, 4);
    }

    bool Deserialize(const uint8_t* in) override
    {
      return true;
    }

    uint32_t SchemaHash() const override 
    { 
      return 0xDEADBEEFu;
    }

    bool Describe(hostlink::ComponentWriter& w) const override
    {
      static char disp_json[64];

      w.Label("Profiler");

      bool ok = w.Field("cpu.min", "CPU Min Load", 0, hostlink::FieldType::F32, 0);
      ok &= w.Field("cpu.max", "CPU Max Load", 4, hostlink::FieldType::F32, 4);
      ok &= w.Field("cpu.avg", "CPU Avg Load", 8, hostlink::FieldType::F32, 8);

      // for(size_t i = 0; i < var_count_; ++i)
      // {
      //   const VarDesc& desc = desc_[i];
      //   sprintf(disp_json, 
      //     "{\"kind\":\"%s\",\"lo\":%d,\"hi\":%d,\"unit\":\"%s\"}",
      //     desc.kind, static_cast<int>(desc.min), static_cast<int>(desc.max), desc.unit
      //   );
      //   ok &= w.Field(desc.id, desc.name, i*sizeof(float), alchemy::hostlink::FieldType::F32, desc.def, disp_json);
      // }
      return ok;
    }
  };
} // namespace Profiler
} // namespace alchemy