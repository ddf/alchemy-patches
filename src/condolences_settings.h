#pragma once

#include "condolences_dsp.h"
#include "alchemy/surface/serializable.h"

namespace condolences
{
using namespace alchemy;

////////////////////////////////////////////////////////////////////////////////
// Settings
constexpr float band_density_min = GetDensityMin();
constexpr float band_density_max = GetDensityMax();
constexpr float sensi_min        = 0.1f;
constexpr float sensi_max        = 0.9f;
constexpr float dampi_min        = 0.8;
constexpr float dampi_max        = 0.999;
constexpr float motio_min        = 0.1f;
constexpr float motio_max        = 1.0f;
constexpr float smear_min        = 1.0f;
constexpr float smear_max        = 8.0f;

// absolute min/max values that ripple params are allowed to have
namespace ripple
{
constexpr float amount_min = 0.f;
constexpr float amount_max = 1.f;
constexpr float depth_min  = 0.0f;
constexpr float depth_max  = 1.0f;
constexpr float damp_reduct_min = 0.0f;
constexpr float damp_reduct_max = 1.0f;
constexpr float smear_boost_min = 0.f;
constexpr float smear_boost_max = 1.0f;
}

struct VibeSettings : alchemy::Serializable
{
  static constexpr int16_t page = 1;

  static constexpr float band_min_default = (192.f - band_density_min) / (band_density_max - band_density_min);
  static constexpr float band_max_default = (band_density_max - band_density_min) / (band_density_max - band_density_min);
  static constexpr float spread_min_dafault = 0.0f;
  static constexpr float spread_max_default = 1.0f;

  /* Normalized 0..1; the disp hint maps the readout to 0..2× gain. */
  float band_min = band_min_default;
  float band_max = band_max_default;
  float spread_min = spread_min_dafault;
  float spread_max = spread_max_default;

  size_t SerializedSize() const override { return 4u * sizeof(float); }

  void Serialize(uint8_t* out) const override
  {
    std::memcpy(out + 0, &band_min, 4);
    std::memcpy(out + 4, &band_max, 4);
    std::memcpy(out + 8, &spread_min, 4);
    std::memcpy(out + 12, &spread_max, 4);
  }

  bool Deserialize(const uint8_t* in) override
  {
    std::memcpy(&band_min, in + 0, 4);
    std::memcpy(&band_max, in + 4, 4);
    std::memcpy(&spread_min, in + 8, 4);
    std::memcpy(&spread_max, in + 12, 4);
    return true;
  }

  uint32_t SchemaHash() const override { return ('V'<<24) | ('I'<<16) | ('B'<<8) | ('A'); }

  bool Describe(hostlink::ComponentWriter& w) const override
  {
    w.Label("Vibe Settings");
    
    char band_disp_json[64];
    sprintf(band_disp_json, "{\"kind\":\"linear\",\"lo\":%d,\"hi\":%d}", 
      static_cast<int>(band_density_min), static_cast<int>(band_density_max));

    bool ok = w.Field("density.min", "Perception Bands Min", 0, hostlink::FieldType::F32, 
        band_min_default, 
        band_disp_json,
        0, // zones 
        page, 
        kPotTopLeft);

    ok &= w.Field("density.max", "Perception Bands Max", 4, hostlink::FieldType::F32, 
        band_max_default, 
        band_disp_json,
        0, // zones
        page,
        kPotTopRight
    );

    ok &= w.Field("spread.min", "Focus Min", 8, hostlink::FieldType::F32, 
        spread_min_dafault,
        nullptr,
        0, // zones
        page,
        kPotMiddleLeft
    );

    ok &= w.Field("spread.max", "Focus Max", 12, hostlink::FieldType::F32,
        spread_max_default,
        nullptr,
        0, // zones
        page,
        kPotMiddleRight
    );

    return ok;
  }
};

struct RizzSettings : alchemy::Serializable
{
  static constexpr int16_t page = 2;

  // range for the ripple (sizzle) knob
  float rippl_amount_max = 0.7f;
  // settings for fixed values
  float rippl_depth = 0.15f;
  float rippl_damp_reduct = 0.6f;
  float rippl_smear_boost = 0.20f;

  size_t SerializedSize() const override { return 4u * sizeof(float); }

  void Serialize(uint8_t* out) const override
  {
    std::memcpy(out + 0, &rippl_amount_max, 4);
    std::memcpy(out + 4, &rippl_depth, 4);
    std::memcpy(out + 8, &rippl_damp_reduct, 4);
    std::memcpy(out + 12, &rippl_smear_boost, 4);
  }

  bool Deserialize(const uint8_t* in) override
  {
    std::memcpy(&rippl_amount_max, in + 0, 4);
    std::memcpy(&rippl_depth, in + 4, 4);
    std::memcpy(&rippl_damp_reduct, in + 8, 4);
    std::memcpy(&rippl_smear_boost, in + 12, 4);
    return true;
  }

  uint32_t SchemaHash() const override { return ('R'<<24) | ('I'<<16) | ('Z'<<8) | ('A'); }

  bool Describe(hostlink::ComponentWriter& w) const override
  {
    w.Label("Rizz Settings");
    
    // char disp_json[128];

    // sprintf(disp_json, "{\"kind\":\"linear\",\"lo\":%.2f,\"hi\":%.2f}", ripple::amount_min, ripple::amount_max);
    bool ok = w.Field("ripple.amt", "Sizzle Amount Max", 0, hostlink::FieldType::F32, 
        rippl_amount_max,
        nullptr,
        0, // zones
        page,
        kPotTopLeft
    );

    // sprintf(disp_json, "{\"kind\":\"linear\",\"lo\":%.2f,\"hi\":%.2f}", ripple::depth_min, ripple::depth_max);
    ok &= w.Field("ripple.depth", "Sizzle LFO Depth", 4, hostlink::FieldType::F32, 
        rippl_depth,
        nullptr,
        0, // zones
        page,
        kPotTopRight
    );

    // sprintf(disp_json, "{\"kind\":\"linear\",\"lo\":%.2f,\"hi\":%.2f}", ripple::damp_reduct_min, ripple::damp_reduct_max);
    ok &= w.Field("ripple.damp", "Sympathy: Sizzle Reduction", 8, hostlink::FieldType::F32, 
        rippl_damp_reduct,
        nullptr,
        0, // zones
        page,
        kPotMiddleLeft
    );

    // sprintf(disp_json, "{\"kind\":\"linear\",\"lo\":%.2f,\"hi\":%.2f}", ripple::smear_boost_min, ripple::smear_boost_max);
    ok &= w.Field("ripple.boost", "Smear: Sizzle Boost", 12, hostlink::FieldType::F32, 
        rippl_smear_boost,
        nullptr,
        0, // zones
        page,
        kPotMiddleRight
    );

    return ok;
  }
};
} // namespace condolences