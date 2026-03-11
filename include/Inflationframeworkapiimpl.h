#pragma once

#include "InflationFrameworkAPI.h"
#include "InflationManager.h"
#include "morph.h"

namespace InflationFrameworkAPI
{

// =========================================================================
//  API 实现 (单例, 内部使用 InflationManager + Morph)
// =========================================================================
class InflationFrameworkInterfaceImpl final : public IInflationFrameworkInterface
{
public:
  static InflationFrameworkInterfaceImpl* GetSingleton()
  {
    static InflationFrameworkInterfaceImpl instance;
    return &instance;
  }

  std::uint32_t GetVersion() const override { return kAPIVersion; }

  float GetInflation(RE::Actor* actor, MorphType type) const override
  {
    return InflationManager::GetInflation(actor, static_cast<Morph::MorphType>(type));
  }

  void SetInflation(RE::Actor* actor, MorphType type, float value) override
  {
    InflationManager::SetInflation(actor, static_cast<Morph::MorphType>(type), value);
  }

  void ModInflation(RE::Actor* actor, MorphType type, float value) override
  {
    InflationManager::ModInflation(actor, static_cast<Morph::MorphType>(type), value);
  }

  float GetMorphByName(RE::Actor* actor, const char* morphName) const override { return Morph::GetMorphByName(actor, morphName); }

  void SetMorphByName(RE::Actor* actor, const char* morphName, float value) override { Morph::SetMorphByName(actor, morphName, value); }

  void ApplyMorphs(RE::Actor* actor) override { Morph::ApplyMorphs(actor); }

private:
  InflationFrameworkInterfaceImpl() = default;
};

}  // namespace InflationFrameworkAPI
