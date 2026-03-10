

void onPostLoad() {}

void onDataLoaded() {}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
  switch (a_msg->type) {
  case SKSE::MessagingInterface::kPostLoad:
    onPostLoad();
    break;
  case SKSE::MessagingInterface::kDataLoaded:
    onDataLoaded();
    break;
  case SKSE::MessagingInterface::kNewGame:
    break;
  case SKSE::MessagingInterface::kPreLoadGame:
    break;
  case SKSE::MessagingInterface::kPostLoadGame:
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