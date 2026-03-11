#include "Menu.h"

#include "inflationManager.h"
#include "morph.h"
#include "settings.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

namespace Localization
{
struct StringMap
{
  std::string label;
  std::string desc;
};

static std::unordered_map<std::uint32_t, StringMap> stringMaps;
static std::unordered_map<std::uint32_t, std::vector<StringMap>> comboMaps;

void LoadLocalization()
{
  constexpr std::string_view LocalizationPath = "Data/SKSE/Plugins/InflationFramework_Localization.json";

  nlohmann::json j;
  try {
    std::ifstream ifs(LocalizationPath.data());
    j = nlohmann::json::parse(ifs);
  } catch (const std::exception& e) {
    logger::error("[InflationFramework] Failed to load localization file: {}. Error: {}", LocalizationPath, e.what());
    return;
  }

  if (j.is_null() || !j.is_object()) {
    logger::error("[InflationFramework] Localization file is not a valid JSON object.");
    return;
  }

  auto buildComboMap = []<typename E>() {
    const auto& enum_name   = magic_enum::enum_type_name<E>();
    const auto& value_names = magic_enum::enum_names<E>();
    std::vector<StringMap> maps;
    for (const auto& name : value_names) {
      if (auto hashValue = hash(name); stringMaps.find(hashValue) != stringMaps.end())
        maps.push_back(stringMaps.at(hashValue));
    }
    comboMaps[hash(enum_name)] = std::move(maps);
  };

  stringMaps.reserve(j.size());
  for (const auto& [key, value] : j.items()) {
    if (value.empty() || !value.is_object())
      continue;

    auto label = value.value("label", "");
    auto desc  = value.value("desc", "");

    stringMaps[hash(key)] = {label, desc};
  }
  buildComboMap.operator()<Morph::MorphType>();
}
}  // namespace Localization

namespace ImGui
{
using StringMap = Localization::StringMap;
inline const StringMap& GetStringMap(std::uint32_t hash)
{
  static const StringMap unknown = {"", ""};
  auto it                        = Localization::stringMaps.find(hash);
  return (it != Localization::stringMaps.end()) ? it->second : unknown;
}
void SetSection(std::uint32_t hash)
{
  const StringMap& map = GetStringMap(hash);
  if (!map.label.empty())
    SKSEMenuFramework::SetSection(map.label.data());
  else
    SKSEMenuFramework::SetSection(GetStringMap("UnknownValue"_h).label.data());
}
void AddSectionItem(std::uint32_t hash, SKSEMenuFramework::Model::RenderFunction func)
{
  const StringMap& map = GetStringMap(hash);
  if (!map.label.empty())
    SKSEMenuFramework::AddSectionItem(map.label.data(), func);
  else
    SKSEMenuFramework::AddSectionItem(GetStringMap("UnknownValue"_h).label.data(), func);
}
void Checkbox(std::uint32_t hash, bool* v, std::function<void()> onChange = nullptr)
{
  const StringMap& map = GetStringMap(hash);
  if (ImGuiMCP::Checkbox(map.label.data(), v))
    if (onChange)
      onChange();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}
void DragInt(std::uint32_t hash, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d",
             std::function<void()> onChange = nullptr)
{
  const StringMap& map             = GetStringMap(hash);
  ImGuiMCP::ImGuiSliderFlags flags = ImGuiMCP::ImGuiSliderFlags_None;
  if (ImGuiMCP::DragInt(map.label.data(), v, v_speed, v_min, v_max, format, flags))
    if (onChange)
      onChange();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}
void DragFloat(std::uint32_t hash, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.1f",
               std::function<void()> onChange = nullptr)
{
  const StringMap& map             = GetStringMap(hash);
  ImGuiMCP::ImGuiSliderFlags flags = ImGuiMCP::ImGuiSliderFlags_None;
  if (ImGuiMCP::DragFloat(map.label.data(), v, v_speed, v_min, v_max, format, flags))
    if (onChange)
      onChange();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}
template <typename T>
void Combo(std::uint32_t hash, T* current_item, std::function<void()> onChange = nullptr)
{
  const StringMap& map = GetStringMap(hash);
  const auto& comboIt  = Localization::comboMaps.at(hash);
  if (ImGuiMCP::BeginCombo(map.label.data(), comboIt[*current_item].label.data())) {
    for (size_t n = 0; n < comboIt.size(); n++) {
      const bool is_selected = (*current_item == static_cast<int>(n));
      if (ImGuiMCP::Selectable(comboIt[n].label.data(), is_selected)) {
        *current_item = static_cast<int>(n);
        if (onChange)
          onChange();
      }
      if (ImGuiMCP::IsItemHovered() && !comboIt[n].desc.empty())
        ImGuiMCP::SetTooltip(comboIt[n].desc.data());
      if (is_selected)
        ImGuiMCP::SetItemDefaultFocus();
    }
    ImGuiMCP::EndCombo();
  }
}
}  // namespace ImGui

void Menu::Settings()
{
  ImGui::Checkbox("EnableMod"_h, &Settings::enable);
  ImGui::Checkbox("EnablePlayer"_h, &Settings::enablePlayer);
  ImGui::Checkbox("EnableNPC"_h, &Settings::enableNPC);
}

void Menu::Debug()
{
  for (const auto& [type, data] : Morph::GetMorphDataMap()) {
    auto value = InflationManager::GetInflation(RE::PlayerCharacter::GetSingleton(), type);
    ImGui::DragFloat(data.hash, &value, 0.01f, data.min, data.max, "%.2f", [&]() {
      InflationManager::SetInflation(RE::PlayerCharacter::GetSingleton(), type, value);
    });
  }
}

void __stdcall Menu::EventListener(SKSEMenuFramework::Model::EventType eventType)
{
  switch (eventType) {
  case SKSEMenuFramework::Model::EventType::kOpenMenu:
    break;
  case SKSEMenuFramework::Model::EventType::kCloseMenu:
    Settings::SettingsToJson();
    break;
  }
}

void Menu::InsertLocalization(std::string key, std::string label, std::string desc)
{
  Localization::stringMaps[hash(key)] = {std::move(label), std::move(desc)};
}

Menu::Menu()
{
  Localization::LoadLocalization();

  if (!SKSEMenuFramework::IsInstalled())
    return;

  ImGui::SetSection("InflationFramework"_h);

  ImGui::AddSectionItem("Settings"_h, Settings);
  ImGui::AddSectionItem("Debug"_h, Debug);

  // priority should be a individual value for each mod, here is nexus id of this mod
  event = new SKSEMenuFramework::Model::Event(EventListener, static_cast<float>("InflationFramework"_h));

  logger::info("[InflationFramework] Menu: SKSEMenuFramework v{} loaded.", SKSEMenuFramework::GetMenuFrameworkVersion());
}

Menu::~Menu()
{
  if (event)
    delete event;
}