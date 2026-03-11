#pragma once

namespace Settings
{
inline bool enable       = true;
inline bool enablePlayer = true;
inline bool enableNPC    = true;

void JsonToSettings();
void SettingsToJson();
}  // namespace Settings