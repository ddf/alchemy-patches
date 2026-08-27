#pragma once

#include <stdio.h>
#include <cstring>
#include <vector>
#include "alchemy/surface/serializable.h"
#include "alchemy/host_link/component_writer.h"

using namespace alchemy;

class Debugger : public Serializable
{
public:
  static constexpr size_t kMaxVars = 32;

  class VarDesc
  {
  public:
    VarDesc& Range(float lo, float hi)
    {
      min = lo;
      max = hi;
      return *this;
    }

    VarDesc& Kind(const char* kind)
    {
      this->kind = kind;
      return *this;
    }

    VarDesc& Unit(const char* unit)
    {
      this->unit = unit;
      return *this;
    }

    bool IsValid() const
    {
      return id != nullptr && name != nullptr;
    }

  private:
    // need min and max because we have to serialize/deserialze normalized
    float def = 0.f;
    float min = 0.f;
    float max = 1.f;
    const char* id = nullptr;
    const char* name = nullptr;
    const char* kind = "norm"; // or "linear" or "exp"
    const char* unit = "%"; // or other engineering unit like "ms" or "s" or "dB"

    friend class Debugger;
  };

  Debugger() : var_count_(0)
  {
    memset(data_, 0, sizeof(volatile float*)*kMaxVars);
  }

  // register a float32 variable with the debugger,
  // set the Range if var is not a normalized value.
  VarDesc& Var(volatile float& var, const char* id, const char* name)
  {
    static VarDesc none;
    if( var_count_ < kMaxVars )
    {
      data_[var_count_] = &var;
      VarDesc& desc = desc_[var_count_++];
      desc.def = var;
      desc.id  = id;
      desc.name = name;
      return desc;
    }
    return none;
  }

  size_t SerializedSize() const override { return sizeof(float)*kMaxVars; }

  void Serialize(uint8_t* out) const override
  {
    for(size_t i = 0; i < kMaxVars; ++i)
    {
      float val = 0;
      if (data_[i])
      {
        const VarDesc& desc = desc_[i];
        val = (*data_[i] - desc.min) / (desc.max - desc.min);
      }
      std::memcpy(out + i*sizeof(float), &val, sizeof(float));
    }
  }

  bool Deserialize(const uint8_t* in) override
  {
    for(size_t i = 0; i < kMaxVars; ++i)
    {
      float val;
      std::memcpy(&val, in + i*sizeof(float), sizeof(float));
      if (data_[i])
      {
        const VarDesc& desc = desc_[i];
        *data_[i] = desc.min + val * (desc.max - desc.min);
      }
    }
    return true;
  }

  uint32_t SchemaHash() const override 
  { 
    return 0xCDEB6032u;
  }

  bool Describe(hostlink::ComponentWriter& w) const override
  {
    static char disp_json[64];

    w.Label("Debugger");

    bool ok = true;
    for(size_t i = 0; i < var_count_; ++i)
    {
      const VarDesc& desc = desc_[i];
      sprintf(disp_json, 
        "{\"kind\":\"%s\",\"lo\":%d,\"hi\":%d,\"unit\":\"%s\"}",
        desc.kind, static_cast<int>(desc.min), static_cast<int>(desc.max), desc.unit
      );
      ok &= w.Field(desc.id, desc.name, i*sizeof(float), alchemy::hostlink::FieldType::F32, desc.def, disp_json);
    }
    return ok;
  }

private:
  volatile float* data_[kMaxVars];
  VarDesc desc_[kMaxVars];
  size_t var_count_;
};