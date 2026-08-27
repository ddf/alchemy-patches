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

  // register a float32 variable with the debugger
  Debugger& Var(float def, const char* id, const char * name, const char* disp_json = nullptr)
  {
    if( desc_.size() < kMaxVars )
    {
      data_[desc_.size()] = def;
      VarDesc desc = { def, id, name, disp_json };
      desc_.push_back(desc);
    }
    return *this;
  }

  void SetVar(const char* id, float val)
  {
    for(size_t i = 0; i < desc_.size(); ++i)
    {
      if (strcmp(desc_[i].id, id)==0)
      {
        data_[i] = val;
        break;
      }
    }
  }

  size_t SerializedSize() const override { return sizeof(float)*kMaxVars; }

  void Serialize(uint8_t* out) const override
  {
    std::memcpy(out, data_, SerializedSize());
  }

  bool Deserialize(const uint8_t* in) override
  {
    std::memcpy(data_, in, SerializedSize());
    return true;
  }

  uint32_t SchemaHash() const override 
  { 
    return 0xADEB6032u;
  }

  bool Describe(hostlink::ComponentWriter& w) const override
  {
    w.Label("Debugger");

    bool ok = true;
    size_t off = 0;
    for(const VarDesc& desc : desc_)
    {
      ok &= w.Field(desc.id, desc.name, off, alchemy::hostlink::FieldType::F32, desc.def, desc.disp_json);
    }
    return ok;
  }

private:
  struct VarDesc
  {
    float def;
    const char* id;
    const char* name;
    const char* disp_json;
  };

  float data_[kMaxVars];
  std::vector<VarDesc> desc_;
};