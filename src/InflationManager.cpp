#include "InflationManager.h"
#include "menu.h"

#include "magic_enum/magic_enum.hpp"

namespace InflationManager
{
static std::unordered_map<RE::FormID, std::unordered_map<Morph::MorphType, float>> inflationDataMap;
static std::unordered_map<RE::Actor*, std::unordered_map<Morph::MorphType, float>> runtimeInflationDataMap;

static std::mutex mtx;

constexpr std::uint32_t SerializationVersion = 1;
constexpr std::uint32_t InflationData        = 'INFD';

std::uint32_t RegisterInflation(std::string morphName, float min, float max)
{
  auto type = Morph::GetType(morphName);
  if (static_cast<std::uint32_t>(type) == 0)
    return 0;

  if (Morph::GetMorphData(type).morphName.empty()) {
    Morph::RegisterMorph(morphName, min, max);
    Menu::InsertLocalization(morphName, morphName, "");
    return static_cast<std::uint32_t>(type);
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
    if (auto it = inflationDataMap.find(formID); it != inflationDataMap.end())
      if (auto typeIt = it->second.find(type); typeIt != it->second.end())
        return typeIt->second;

    auto morphValue                = Morph::GetMorphByType(actor, type);
    inflationDataMap[formID][type] = morphValue;
  } else {
    if (auto it = runtimeInflationDataMap.find(actor); it != runtimeInflationDataMap.end())
      if (auto typeIt = it->second.find(type); typeIt != it->second.end())
        return typeIt->second;

    auto morphValue                      = Morph::GetMorphByType(actor, type);
    runtimeInflationDataMap[actor][type] = morphValue;
  }
  return 0.0f;
}

void SetInflation(RE::Actor* actor, Morph::MorphType type, float value)
{
  if (!actor)
    return;

  auto clampedValue = std::clamp(value, Morph::GetMinValue(type), Morph::GetMorphData(type).max);

  if (actor->GetActorBase()->IsUnique()) {
    RE::FormID formID              = actor->GetFormID();
    inflationDataMap[formID][type] = clampedValue;
  } else
    runtimeInflationDataMap[actor][type] = clampedValue;
  Morph::SetMorphByType(actor, type, clampedValue);
  Morph::ApplyMorphs(actor);
}

void ModInflation(RE::Actor* actor, Morph::MorphType type, float value)
{
  if (!actor)
    return;

  RE::FormID formID = actor->GetFormID();
  if (actor->GetActorBase()->IsUnique()) {
    if (auto it = inflationDataMap.find(formID); it != inflationDataMap.end()) {
      if (auto typeIt = it->second.find(type); typeIt != it->second.end()) {
        typeIt->second += value;
        typeIt->second = std::clamp(typeIt->second, Morph::GetMinValue(type), Morph::GetMorphData(type).max);
        Morph::SetMorphByType(actor, type, typeIt->second);
        Morph::ApplyMorphs(actor);
      }
    }
  } else {
    if (auto it = runtimeInflationDataMap.find(actor); it != runtimeInflationDataMap.end()) {
      if (auto typeIt = it->second.find(type); typeIt != it->second.end()) {
        typeIt->second += value;
        typeIt->second = std::clamp(typeIt->second, Morph::GetMinValue(type), Morph::GetMorphData(type).max);
        Morph::SetMorphByType(actor, type, typeIt->second);
        Morph::ApplyMorphs(actor);
      }
    }
  }
}

void SaveData(SKSE::SerializationInterface* serial)
{
  std::lock_guard<std::mutex> lock(mtx);

  // 清理不存在对应 MorphData 的数据，避免保存无效数据
  for (auto& [_, morphMap] : inflationDataMap) {
    for (auto it = morphMap.begin(); it != morphMap.end();) {
      if (Morph::GetMorphData(it->first).morphName.empty())
        it = morphMap.erase(it);
      else
        ++it;
    }
  }

  if (serial->OpenRecord(InflationData, SerializationVersion)) {
    serial->WriteRecordData(static_cast<std::size_t>(inflationDataMap.size()));
    for (const auto& [formID, morphMap] : inflationDataMap) {
      serial->WriteRecordData(formID);
      serial->WriteRecordData(static_cast<std::size_t>(morphMap.size()));
      for (const auto& [type, value] : morphMap) {
        serial->WriteRecordData(type);
        serial->WriteRecordData(value);
      }
    }
  }
}
void LoadData(SKSE::SerializationInterface* serial)
{
  std::lock_guard<std::mutex> lock(mtx);

  inflationDataMap.clear();
  runtimeInflationDataMap.clear();

  std::uint32_t type, version, length;
  while (serial->GetNextRecordInfo(type, version, length)) {
    if (version != SerializationVersion) {
      logger::warn("[InflationManager] Serialization version mismatch: got {}, expected {}", version, SerializationVersion);
      continue;
    }

    switch (type) {
    case InflationData: {
      std::size_t actorCount = 0;
      serial->ReadRecordData(actorCount);
      for (std::size_t i = 0; i < actorCount; ++i) {
        RE::FormID formID;
        serial->ReadRecordData(formID);

        std::size_t morphCount = 0;
        serial->ReadRecordData(morphCount);

        RE::Actor* actor = RE::Actor::LookupByID<RE::Actor>(formID);
        std::unordered_map<Morph::MorphType, float> morphMap;
        for (std::size_t j = 0; j < morphCount; ++j) {
          Morph::MorphType type;
          float value;
          serial->ReadRecordData(type);
          serial->ReadRecordData(value);
          morphMap[type] = value;
          if (actor)
            SetInflation(actor, type, value);
        }
        inflationDataMap[formID] = std::move(morphMap);
      }
    } break;
    default:
      logger::warn("[InflationManager] Unknown serialization record type {:08X}", type);
      break;
    }
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