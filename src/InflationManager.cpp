#include "InflationManager.h"

#include "magic_enum/magic_enum.hpp"

namespace InflationManager
{
static std::unordered_map<RE::FormID, std::unordered_map<std::uint32_t, float>> inflationDataMap;
static std::unordered_map<RE::Actor*, std::unordered_map<std::uint32_t, float>> runtimeInflationDataMap;

static std::mutex mtx;

std::uint32_t RegisterInflation(std::string_view morphName, float min, float max)
{
  auto hashValue = Morph::GetHash(morphName);
  if (Morph::GetHash(static_cast<Morph::MorphType>(hashValue)) == 0) {
    Morph::RegisterMorph(morphName, min, max);
    return hashValue;
  }
  logger::warn("[InflationManager] Inflation '{}' already registered.", morphName);
  return 0;
}

float GetInflation(RE::Actor* actor, Morph::MorphType type)
{
  if (!actor)
    return 0.0f;

  if (actor->GetActorBase()->IsUnique()) {
    RE::FormID formID = actor->GetFormID();
    const auto it     = inflationDataMap.find(formID);
    if (it != inflationDataMap.end()) {
      const auto hashIt = it->second.find(Morph::GetHash(type));
      if (hashIt != it->second.end()) {
        return hashIt->second;
      }
      auto morphValue                                = Morph::GetMorphByType(actor, type);
      inflationDataMap[formID][Morph::GetHash(type)] = morphValue;
    }
  } else {
    const auto it = runtimeInflationDataMap.find(actor);
    if (it != runtimeInflationDataMap.end()) {
      const auto hashIt = it->second.find(Morph::GetHash(type));
      if (hashIt != it->second.end()) {
        return hashIt->second;
      }
      auto morphValue                                      = Morph::GetMorphByType(actor, type);
      runtimeInflationDataMap[actor][Morph::GetHash(type)] = morphValue;
    }
  }
  return 0.0f;
}

void SetInflation(RE::Actor* actor, Morph::MorphType type, float value)
{
  if (!actor)
    return;

  if (actor->GetActorBase()->IsUnique()) {
    RE::FormID formID                              = actor->GetFormID();
    inflationDataMap[formID][Morph::GetHash(type)] = value;
  } else
    runtimeInflationDataMap[actor][Morph::GetHash(type)] = value;
  Morph::SetMorphByType(actor, type, value);
  Morph::ApplyMorphs(actor);
}

void ModInflation(RE::Actor* actor, Morph::MorphType type, float value)
{
  if (!actor)
    return;

  RE::FormID formID = actor->GetFormID();
  if (actor->GetActorBase()->IsUnique()) {
    const auto it = inflationDataMap.find(formID);
    if (it != inflationDataMap.end()) {
      const auto hashIt = it->second.find(Morph::GetHash(type));
      if (hashIt != it->second.end()) {
        hashIt->second += value;
        Morph::SetMorphByType(actor, type, hashIt->second);
        Morph::ApplyMorphs(actor);
      }
    }
  } else {
    const auto it = runtimeInflationDataMap.find(actor);
    if (it != runtimeInflationDataMap.end()) {
      const auto hashIt = it->second.find(Morph::GetHash(type));
      if (hashIt != it->second.end()) {
        hashIt->second += value;
        Morph::SetMorphByType(actor, type, hashIt->second);
        Morph::ApplyMorphs(actor);
      }
    }
  }
}

void SaveData(SKSE::SerializationInterface* serial)
{
  std::lock_guard<std::mutex> lock(mtx);
  serial->WriteRecordData(static_cast<std::size_t>(inflationDataMap.size()));
  for (const auto& [formID, morphMap] : inflationDataMap) {
    serial->WriteRecordData(formID);
    serial->WriteRecordData(static_cast<std::size_t>(morphMap.size()));
    for (const auto& [hash, value] : morphMap) {
      serial->WriteRecordData(hash);
      serial->WriteRecordData(value);
    }
  }
}
void LoadData(SKSE::SerializationInterface* serial)
{
  std::lock_guard<std::mutex> lock(mtx);
  inflationDataMap.clear();
  runtimeInflationDataMap.clear();

  std::size_t actorCount = 0;
  serial->ReadRecordData(actorCount);
  for (std::size_t i = 0; i < actorCount; ++i) {
    RE::FormID formID;
    serial->ReadRecordData(formID);

    std::size_t morphCount = 0;
    serial->ReadRecordData(morphCount);

    std::unordered_map<std::uint32_t, float> morphMap;
    for (std::size_t j = 0; j < morphCount; ++j) {
      std::uint32_t hash;
      float value;
      serial->ReadRecordData(hash);
      serial->ReadRecordData(value);
      morphMap[hash] = value;
    }
    inflationDataMap[formID] = std::move(morphMap);
  }
}
void RevertData(SKSE::SerializationInterface* serial)
{
  std::lock_guard<std::mutex> lock(mtx);
  inflationDataMap.clear();
  runtimeInflationDataMap.clear();
}

void Initialize()
{
  auto serial = SKSE::GetSerializationInterface();
  if (!serial) {
    logger::critical("[Inflation Framework] Failed to get SerializationInterface");
    return;
  }
  serial->SetUniqueID(MOD);
  serial->SetSaveCallback(SaveData);
  serial->SetLoadCallback(LoadData);
  serial->SetRevertCallback(RevertData);
  logger::info("[Inflation Framework] InflationManager initialized");
}
}  // namespace InflationManager