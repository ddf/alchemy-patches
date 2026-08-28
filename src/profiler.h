#pragma once

#include "alchemy/hw/alchemy_lab.h"
#include "alchemy/surface/serializable.h"

namespace alchemy
{
namespace Profiler
{
  void Init(float sampleRateInHz, int blockSizeInSamples, daisy::AudioHandle::AudioCallback user_audio_callback);
  void Process(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t block_size);

  static constexpr size_t kMaxTimers = 32;

  class Timer
  {
  public:
    explicit Timer(const char* name);
    void Start();
    void Stop();

  private:
    friend class SettingsPage;
    const char* name_;
    uint16_t idx_;
    uint32_t begin_;
  };

  class ScopedTimer
  {
  public:
    explicit ScopedTimer(Timer& th) : th_(&th) { th.Start(); }
    ~ScopedTimer() { th_->Stop(); }
  private:
    Timer* th_;
  };

  class SettingsPage : public Serializable
  {
  public:
    size_t SerializedSize() const override { return sizeof(float)*3 + sizeof(float)*kMaxTimers; }
    void Serialize(uint8_t* out) const override;
    bool Deserialize(const uint8_t* in) override { return true; } 
    uint32_t SchemaHash() const override;
    bool Describe(class hostlink::ComponentWriter& w) const override;
  };
} // namespace Profiler
} // namespace alchemy