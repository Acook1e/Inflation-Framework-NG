#include "morph.h"

#include "API/SKEE.h"

namespace Morph
{
static IBodyMorphInterface* bodyMorphInterface = nullptr;
static std::unordered_map<std::uint32_t, MorphData> morphDataMap;

void SetMorphByName(RE::Actor* actor, const char* morphName, const char* morphKey, float value)
{
  if (bodyMorphInterface)
    bodyMorphInterface->SetMorph(actor->As<TESObjectREFR>(), morphName, morphKey, value);
}

void Initialize()
{
  InterfaceExchangeMessage msg;
  const auto* const intfc{SKSE::GetMessagingInterface()};
  intfc->Dispatch(InterfaceExchangeMessage::kMessage_ExchangeInterface, &msg, sizeof(InterfaceExchangeMessage*), "skee");
  if (!msg.interfaceMap) {
    logger::critical("[Inflation Framework] Morph::Initialize : Couldn't get interface map!");
    return;
  }
  bodyMorphInterface = static_cast<IBodyMorphInterface*>(msg.interfaceMap->QueryInterface("BodyMorph"));
  if (!bodyMorphInterface) {
    logger::critical("[Inflation Framework] Morph::Initialize : Couldn't get body morph interface!");
    return;
  }
}
}  // namespace Morph