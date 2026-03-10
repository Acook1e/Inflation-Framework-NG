#include "InflationManager.h"
#include "menu.h"
#include "morph.h"
#include "settings.h"

inline void onPostLoad()
{
  Settings::JsonToSettings();
  InflationManager::Initialize();
}

inline void onPostPostLoad()
{
  Morph::Initialize();
}

inline void onDataLoaded()
{
  Menu::GetSingleton();
}

inline void onEnterGame() {}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
  switch (a_msg->type) {
  case SKSE::MessagingInterface::kPostLoad:
    onPostLoad();
    break;
  case SKSE::MessagingInterface::kPostPostLoad:
    onPostPostLoad();
    break;
  case SKSE::MessagingInterface::kDataLoaded:
    onDataLoaded();
    break;
  case SKSE::MessagingInterface::kNewGame:
    onEnterGame();
    break;
  case SKSE::MessagingInterface::kPreLoadGame:
    break;
  case SKSE::MessagingInterface::kPostLoadGame:
    onEnterGame();
    break;
  }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
  SKSE::Init(skse, true);

  logger::info("Runtime version: {}", skse->RuntimeVersion());

  auto messaging = SKSE::GetMessagingInterface();
  if (!messaging->RegisterListener(MessageHandler)) {
    return false;
  }

  return true;
}