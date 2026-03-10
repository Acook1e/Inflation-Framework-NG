#include "morph.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

#include "API/SKEE.h"

namespace Morph
{
static SKEE::IBodyMorphInterface* bodyMorphInterface = nullptr;
static std::unordered_map<MorphType, std::uint32_t> hashMap;
static std::unordered_map<std::uint32_t, MorphData> morphDataMap;

std::uint32_t GetHash(MorphType type)
{
  if (hashMap.empty())
    for (const auto& value : magic_enum::enum_values<MorphType>())
      hashMap[value] = hash(magic_enum::enum_name<MorphType>(value));

  return hashMap[type];
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
  }
}
}  // namespace Morph