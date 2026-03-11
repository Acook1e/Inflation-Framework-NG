#include "morph.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

#include "API/SKEE.h"

namespace Morph
{
static SKEE::IBodyMorphInterface* bodyMorphInterface = nullptr;
static std::unordered_map<MorphType, std::uint32_t> hashMap;
static std::unordered_map<std::uint32_t, MorphData> morphDataMap;

std::unordered_map<MorphType, std::uint32_t>& GetHashMap()
{
  return hashMap;
}

MorphData& GetMorphData(std::uint32_t hash)
{
  if (auto it = morphDataMap.find(hash); it != morphDataMap.end())
    return it->second;

  static MorphData emptyData{"", 0.0f, 1.0f};
  return emptyData;
}

std::uint32_t GetHash(MorphType type)
{
  if (auto it = hashMap.find(type); it != hashMap.end())
    return it->second;
  return 0;
}

std::uint32_t GetType(std::string_view morphName)
{
  auto hashValue = hash(morphName) + static_cast<uint32_t>(MorphType::Total);
  if (hashValue < static_cast<uint32_t>(MorphType::Total)) {
    logger::error("[Morph] Hash collision detected for morph '{}'", morphName);
    return 0;
  }
  return hashValue;
}

void RegisterMorph(std::string morphName, float min, float max)
{
  std::uint32_t hashValue                    = GetType(morphName);
  morphDataMap[hashValue]                    = {morphName, min, max};
  hashMap[static_cast<MorphType>(hashValue)] = hash(morphName);
}

void VisitMorphs(std::function<void(MorphType type, std::string_view typeName, float min, float max)> visitor)
{
  for (const auto& [type, hash] : hashMap) {
    const auto& data = morphDataMap.at(hash);
    if (type < MorphType::Total) {
      visitor(type, magic_enum::enum_name(type), data.min, data.max);
    } else
      visitor(type, data.morphName, data.min, data.max);
  }
}

void VisitCustomMorphs(std::function<void(std::string_view morphName, float min, float max)> visitor)
{
  for (const auto& [type, hash] : hashMap) {
    const auto& data = morphDataMap.at(hash);
    if (type >= MorphType::Total) {
      visitor(data.morphName, data.min, data.max);
    }
  }
}

std::string_view GetMorphName(MorphType type)
{
  const auto it = morphDataMap.find(GetHash(type));
  if (it != morphDataMap.end()) {
    return it->second.morphName;
  }
  return "";
}

float GetMinValue(MorphType type)
{
  const auto it = morphDataMap.find(GetHash(type));
  if (it != morphDataMap.end()) {
    return it->second.min;
  }
  return 0.0f;
}

float GetMaxValue(MorphType type)
{
  const auto it = morphDataMap.find(GetHash(type));
  if (it != morphDataMap.end()) {
    return it->second.max;
  }
  return 1.0f;
}

float GetMorphByName(RE::Actor* actor, std::string_view morphName)
{
  if (bodyMorphInterface && actor->Is3DLoaded())
    return bodyMorphInterface->GetMorph(actor, morphName.data(), PLUGIN_NAME.data());
  return 0.0f;
}

float GetMorphByType(RE::Actor* actor, MorphType morphType)
{
  if (bodyMorphInterface && actor->Is3DLoaded()) {
    const auto it = morphDataMap.find(GetHash(morphType));
    if (it != morphDataMap.end()) {
      return bodyMorphInterface->GetMorph(actor, it->second.morphName.data(), PLUGIN_NAME.data());
    }
  }
  return 0.0f;
}

void SetMorphByName(RE::Actor* actor, std::string_view morphName, float value)
{
  if (bodyMorphInterface && actor->Is3DLoaded())
    bodyMorphInterface->SetMorph(actor, morphName.data(), PLUGIN_NAME.data(), value);
}

void SetMorphByType(RE::Actor* actor, MorphType morphType, float value)
{
  if (bodyMorphInterface && actor->Is3DLoaded()) {
    const auto it = morphDataMap.find(GetHash(morphType));
    if (it != morphDataMap.end()) {
      bodyMorphInterface->SetMorph(actor, it->second.morphName.data(), PLUGIN_NAME.data(), value);
    }
  }
}

bool HasMorph(RE::Actor* actor, std::string_view morphName)
{
  if (bodyMorphInterface && actor->Is3DLoaded()) {
    return bodyMorphInterface->HasBodyMorph(actor, morphName.data(), PLUGIN_NAME.data());
  }
  return false;
}

void ApplyMorphs(RE::Actor* actor)
{
  if (bodyMorphInterface && actor->Is3DLoaded())
    bodyMorphInterface->ApplyBodyMorphs(actor);
}

void Initialize()
{
  SKEE::InterfaceExchangeMessage msg;
  const auto* const intfc{SKSE::GetMessagingInterface()};
  intfc->Dispatch(SKEE::InterfaceExchangeMessage::kExchangeInterface, &msg, sizeof(SKEE::InterfaceExchangeMessage*), "skee");
  if (!msg.interfaceMap) {
    logger::critical("[Inflation Framework] Morph::Initialize : Couldn't get interface map!");
    return;
  }
  bodyMorphInterface = static_cast<SKEE::IBodyMorphInterface*>(msg.interfaceMap->QueryInterface("BodyMorph"));
  if (!bodyMorphInterface) {
    logger::critical("[Inflation Framework] Morph::Initialize : Couldn't get body morph interface!");
    return;
  }
  logger::info("[Inflation Framework] Morph::Initialize : Body morph interface acquired.");

  constexpr std::string_view filePath = "Data/SKSE/Plugins/InflationFramework_Morphs.json";
  nlohmann::json j;
  std::fstream f(filePath.data());
  if (!f.is_open()) {
    logger::error("[Inflation Framework] Morph::Initialize : Couldn't open morph mapping file: {}", filePath);
    return;
  }
  f >> j;
  f.close();
  for (const auto& [key, value] : j.items()) {
    auto slider = value.value("SliderName", "");
    logger::info("[Inflation Framework] Morph::Initialize : Loaded morph mapping: {} -> {}", key, slider);
    auto min = value.value("Min", 0.0f);
    auto max = value.value("Max", 1.0f);
    morphDataMap.emplace(hash(key), MorphData{slider, min, max});
    if (auto typeOpt = magic_enum::enum_cast<MorphType>(key); typeOpt.has_value()) {
      hashMap[typeOpt.value()] = hash(key);
    } else {
      if (auto type = GetType(slider); type != 0)
        hashMap[static_cast<MorphType>(type)] = hash(key);
    }
  }
}
}  // namespace Morph