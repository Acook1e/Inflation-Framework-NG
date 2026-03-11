#include "InflationFrameworkAPI.h"
#include "InflationFrameworkAPI_Impl.h"
#include "InflationManager.h"
#include "menu.h"
#include "morph.h"
#include "papyrus.h"
#include "settings.h"

// =========================================================================
//  SKSE Message API 处理
//  其他 DLL (如 FertilityMode) 发送 kExchangeInterface 消息,
//  我们填充 interfacePtr 返回 API 单例
// =========================================================================
void APIMessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
  if (a_msg->type == InflationFrameworkAPI::InterfaceExchangeMessage::kExchangeInterface) {
    auto* msg = static_cast<InflationFrameworkAPI::InterfaceExchangeMessage*>(a_msg->data);
    if (msg) {
      msg->interfacePtr = InflationFrameworkAPI::InflationFrameworkInterfaceImpl::GetSingleton();
      logger::info("[InflationFramework] API interface dispatched to {}", a_msg->sender ? a_msg->sender : "unknown");
    }
  }
}

// =========================================================================
//  生命周期
// =========================================================================

inline void onPostLoad()
{
  Settings::JsonToSettings();
  InflationManager::Initialize();

  // 注册为消息监听器, 接收其他 DLL 的 API 请求
  auto* messaging = SKSE::GetMessagingInterface();
  if (messaging) {
    messaging->RegisterListener("InflationFramework", APIMessageHandler);
  }
}

inline void onPostPostLoad()
{
  Morph::Initialize();
}

inline void onDataLoaded()
{
  Menu::GetSingleton();
  InflationManager::RegisterInflation("BreastHeight", 0.0f, 2.0f);
  Menu::InsertLocalization("BreastHeight", "Breast Height", "Adjust the height of the breasts.");
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

  auto* messaging = SKSE::GetMessagingInterface();
  if (!messaging->RegisterListener(MessageHandler)) {
    return false;
  }

  // 注册 Papyrus Native 函数
  auto* papyrus = SKSE::GetPapyrusInterface();
  if (papyrus) {
    papyrus->Register(Papyrus::RegisterFunctions);
  }

  return true;
}
