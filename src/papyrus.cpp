#include "papyrus.h"
#include "InflationFrameworkAPI.h"
#include "InflationManager.h"
#include "menu.h"
#include "morph.h"

namespace Papyrus
{

static constexpr const char* SCRIPT_NAME         = "InflationFrameworkNative";
static constexpr std::int32_t INVALID_MORPH_TYPE = -1;

// =========================================================================
//  MorphType 枚举转换
//  Papyrus 传 int, C++ 转 Morph::MorphType
//  0=Belly, 1=BellyMid, 2=BellyUnder, 3=BellyPregnancy, 4=Breasts, 5=Butt
// =========================================================================

static Morph::MorphType ToMorphType(std::int32_t morphType)
{
  return static_cast<Morph::MorphType>(static_cast<std::uint32_t>(morphType));
}

static std::int32_t ToPapyrusMorphType(Morph::MorphType morphType)
{
  return static_cast<std::int32_t>(static_cast<std::uint32_t>(morphType));
}

static bool TryResolveMorphType(std::int32_t morphType, Morph::MorphType& resolvedType)
{
  resolvedType = ToMorphType(morphType);
  return !Morph::GetMorphData(resolvedType).morphName.empty();
}

static bool TryFindMorphType(RE::BSFixedString morphName, Morph::MorphType& resolvedType)
{
  if (morphName.empty())
    return false;

  const std::string_view target = morphName.c_str();
  for (const auto& [type, data] : Morph::GetMorphDataMap()) {
    if (data.morphName == target) {
      resolvedType = type;
      return true;
    }
  }

  return false;
}

// =========================================================================
//  Papyrus Native 函数
// =========================================================================

static std::int32_t GetAPIVersion(RE::StaticFunctionTag*)
{
  return static_cast<std::int32_t>(InflationFrameworkAPI::kAPIVersion);
}

static std::int32_t GetMorphType(RE::StaticFunctionTag*, RE::BSFixedString morphName)
{
  Morph::MorphType morphType;
  return TryFindMorphType(morphName, morphType) ? ToPapyrusMorphType(morphType) : INVALID_MORPH_TYPE;
}

static std::int32_t RegisterInflation(RE::StaticFunctionTag*, RE::BSFixedString morphName, float minValue, float maxValue)
{
  if (morphName.empty())
    return INVALID_MORPH_TYPE;

  const auto morphType = InflationManager::RegisterInflation(morphName.c_str(), minValue, maxValue);
  return morphType == 0 ? INVALID_MORPH_TYPE : static_cast<std::int32_t>(morphType);
}

static void RegisterLocalization(RE::StaticFunctionTag*, RE::BSFixedString morphName, RE::BSFixedString label, RE::BSFixedString desc)
{
  if (morphName.empty())
    return;

  Menu::InsertLocalization(morphName.c_str(), label.empty() ? morphName.c_str() : label.c_str(), desc.c_str());
}

// --- 按 MorphType 枚举操作 ---

static float GetInflation(RE::StaticFunctionTag*, RE::Actor* actor, int morphType)
{
  Morph::MorphType resolvedType;
  if (!actor || !TryResolveMorphType(morphType, resolvedType))
    return 0.0f;
  return InflationManager::GetInflation(actor, resolvedType);
}

static void SetInflation(RE::StaticFunctionTag*, RE::Actor* actor, int morphType, float value)
{
  Morph::MorphType resolvedType;
  if (!actor || !TryResolveMorphType(morphType, resolvedType))
    return;
  InflationManager::SetInflation(actor, resolvedType, value);
}

static void ModInflation(RE::StaticFunctionTag*, RE::Actor* actor, int morphType, float value)
{
  Morph::MorphType resolvedType;
  if (!actor || !TryResolveMorphType(morphType, resolvedType))
    return;
  InflationManager::ModInflation(actor, resolvedType, value);
}

// --- 按 morph 名字操作 (直接操作 SKEE slider, 不走 MorphType) ---

static float GetMorphByName(RE::StaticFunctionTag*, RE::Actor* actor, RE::BSFixedString morphName)
{
  if (!actor || morphName.empty())
    return 0.0f;
  return Morph::GetMorphByName(actor, morphName.c_str());
}

static void SetMorphByName(RE::StaticFunctionTag*, RE::Actor* actor, RE::BSFixedString morphName, float value)
{
  if (!actor || morphName.empty())
    return;
  Morph::SetMorphByName(actor, morphName.c_str(), value);
  Morph::ApplyMorphs(actor);
}

// --- 应用 morph ---

static void ApplyMorphs(RE::StaticFunctionTag*, RE::Actor* actor)
{
  if (!actor)
    return;
  Morph::ApplyMorphs(actor);
}

// --- 查询 ---

// 获取某 MorphType 的 min/max (MCM / Papyrus 可用)
static float GetMorphMinValue(RE::StaticFunctionTag*, int morphType)
{
  Morph::MorphType resolvedType;
  return TryResolveMorphType(morphType, resolvedType) ? Morph::GetMinValue(resolvedType) : 0.0f;
}

static float GetMorphMaxValue(RE::StaticFunctionTag*, int morphType)
{
  Morph::MorphType resolvedType;
  return TryResolveMorphType(morphType, resolvedType) ? Morph::GetMaxValue(resolvedType) : 1.0f;
}

static void SetMorphMinValue(RE::StaticFunctionTag*, int morphType, float value)
{
  Morph::MorphType resolvedType;
  if (!TryResolveMorphType(morphType, resolvedType))
    return;

  auto& data = Morph::GetMorphData(resolvedType);
  if (!data.morphName.empty())
    data.min = value > data.max ? data.max : value;
}

static void SetMorphMaxValue(RE::StaticFunctionTag*, int morphType, float value)
{
  Morph::MorphType resolvedType;
  if (!TryResolveMorphType(morphType, resolvedType))
    return;

  auto& data = Morph::GetMorphData(resolvedType);
  if (!data.morphName.empty())
    data.max = value < data.min ? data.min : value;
}

// 获取 MorphType 对应的 SKEE slider 名字
static RE::BSFixedString GetMorphSliderName(RE::StaticFunctionTag*, int morphType)
{
  Morph::MorphType resolvedType;
  return TryResolveMorphType(morphType, resolvedType) ? RE::BSFixedString(Morph::GetMorphName(resolvedType)) : RE::BSFixedString("");
}

// =========================================================================
//  注册
// =========================================================================

bool RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
{
  vm->RegisterFunction("GetAPIVersion", SCRIPT_NAME, GetAPIVersion);
  vm->RegisterFunction("GetMorphType", SCRIPT_NAME, GetMorphType);
  vm->RegisterFunction("RegisterInflation", SCRIPT_NAME, RegisterInflation);
  vm->RegisterFunction("RegisterLocalization", SCRIPT_NAME, RegisterLocalization);

  // 按 MorphType 枚举
  vm->RegisterFunction("GetInflation", SCRIPT_NAME, GetInflation);
  vm->RegisterFunction("SetInflation", SCRIPT_NAME, SetInflation);
  vm->RegisterFunction("ModInflation", SCRIPT_NAME, ModInflation);

  // 按 morph 名字
  vm->RegisterFunction("GetMorphByName", SCRIPT_NAME, GetMorphByName);
  vm->RegisterFunction("SetMorphByName", SCRIPT_NAME, SetMorphByName);

  // 应用
  vm->RegisterFunction("ApplyMorphs", SCRIPT_NAME, ApplyMorphs);

  // 查询
  vm->RegisterFunction("GetMorphMinValue", SCRIPT_NAME, GetMorphMinValue);
  vm->RegisterFunction("GetMorphMaxValue", SCRIPT_NAME, GetMorphMaxValue);
  vm->RegisterFunction("SetMorphMinValue", SCRIPT_NAME, SetMorphMinValue);
  vm->RegisterFunction("SetMorphMaxValue", SCRIPT_NAME, SetMorphMaxValue);
  vm->RegisterFunction("GetMorphSliderName", SCRIPT_NAME, GetMorphSliderName);

  logger::info("[InflationFramework] Papyrus functions registered on {}", SCRIPT_NAME);
  return true;
}

}  // namespace Papyrus
