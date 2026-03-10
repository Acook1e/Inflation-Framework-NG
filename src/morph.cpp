#include "morph.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

#include "API/SKEE.h"

namespace Morph
{
static SKEE::IBodyMorphInterface* bodyMorphInterface = nullptr;
static std::unordered_map<std::uint32_t, MorphData> morphDataMap;

std::unordered_map<std::uint32_t, MorphData>& Get()
{
  return morphDataMap;
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
    auto name     = magic_enum::enum_name<MorphType>(morphType);
    const auto it = morphDataMap.find(hash(name));
    if (it != morphDataMap.end()) {
      return bodyMorphInterface->GetMorph(actor, it->second.morphName.data(), PLUGIN_NAME.data());
    }
  }
  return 0.0f;
}

float GetMorphByMap(RE::Actor* actor, std::string_view mapName)
{
  if (bodyMorphInterface && actor->Is3DLoaded()) {
    const auto it = morphDataMap.find(hash(mapName.data(), mapName.size()));
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
    auto name     = magic_enum::enum_name<MorphType>(morphType);
    const auto it = morphDataMap.find(hash(name));
    if (it != morphDataMap.end()) {
      bodyMorphInterface->SetMorph(actor, it->second.morphName.data(), PLUGIN_NAME.data(), value);
    }
  }
}

void SetMorphByMap(RE::Actor* actor, std::string_view mapName, float value)
{
  if (bodyMorphInterface && actor->Is3DLoaded()) {
    const auto it = morphDataMap.find(hash(mapName.data(), mapName.size()));
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

class VisitorImpl : public SKEE::IBodyMorphInterface::MorphValueVisitor
{
  void Visit(RE::TESObjectREFR* actor, const char* morphName, const char* morphKey, float value) override
  {
    logger::info("Morph {} {} Value {}", morphName, morphKey, value);
  }
};

void visit()
{
  VisitorImpl visitor;
  bodyMorphInterface->VisitMorphValues(RE::PlayerCharacter::GetSingleton(), visitor);
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