#include "InflationManager.h"

#include "magic_enum/magic_enum.hpp"

namespace InflationManager
{
static std::unordered_map<RE::FormID, std::unordered_map<std::uint32_t, float>> inflationDataMap;
static std::unordered_map<RE::Actor*, std::unordered_map<std::uint32_t, float>> runtimeInflationDataMap;

float GetInflation(RE::Actor* actor, Morph::MorphType type)
{
  if (!actor)
    return 0.0f;

  RE::FormID formID = actor->GetFormID();
  if (actor->GetActorBase()->IsUnique()) {
    const auto it = inflationDataMap.find(formID);
    if (it != inflationDataMap.end()) {
      const auto hashIt = it->second.find(Morph::GetHash(type));
      if (hashIt != it->second.end()) {
        return hashIt->second;
      }
    }
  } else {
    const auto it = runtimeInflationDataMap.find(actor);
    if (it != runtimeInflationDataMap.end()) {
      const auto hashIt = it->second.find(Morph::GetHash(type));
      if (hashIt != it->second.end()) {
        return hashIt->second;
      }
    }
  }
  return 0.0f;
}

void SetInflation(RE::Actor* actor, Morph::MorphType type, float value)
{
  if (!actor)
    return;

  RE::FormID formID = actor->GetFormID();
  if (actor->GetActorBase()->IsUnique())
    inflationDataMap[formID][Morph::GetHash(type)] = value;
  else
    runtimeInflationDataMap[actor][Morph::GetHash(type)] = value;
  Morph::SetMorphByType(actor, type, value);
  Morph::ApplyMorphs(actor);
}

void SaveData() {}
void LoadData() {}
void RevertData() {}

};  // namespace InflationManager