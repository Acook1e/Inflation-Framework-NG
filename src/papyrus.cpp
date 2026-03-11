#include "papyrus.h"
#include "InflationManager.h"
#include "morph.h"

namespace Papyrus
{

static constexpr const char* SCRIPT_NAME = "InflationFrameworkNative";

// =========================================================================
//  MorphType 枚举转换
//  Papyrus 传 int, C++ 转 Morph::MorphType
//  0=Belly, 1=BellyMid, 2=BellyUnder, 3=BellyPregnancy, 4=Breasts, 5=Butt
// =========================================================================

static bool IsValidMorphType(int type)
{
  return type >= 0 && type <= static_cast<int>(Morph::MorphType::Butt);
}

// =========================================================================
//  Papyrus Native 函数
// =========================================================================

// --- 按 MorphType 枚举操作 ---

static float GetInflation(RE::StaticFunctionTag*, RE::Actor* actor, int morphType)
{
  if (!actor || !IsValidMorphType(morphType))
    return 0.0f;
  return InflationManager::GetInflation(actor, static_cast<Morph::MorphType>(morphType));
}

static void SetInflation(RE::StaticFunctionTag*, RE::Actor* actor, int morphType, float value)
{
  if (!actor || !IsValidMorphType(morphType))
    return;
  InflationManager::SetInflation(actor, static_cast<Morph::MorphType>(morphType), value);
}

static void ModInflation(RE::StaticFunctionTag*, RE::Actor* actor, int morphType, float value)
{
  if (!actor || !IsValidMorphType(morphType))
    return;
  InflationManager::ModInflation(actor, static_cast<Morph::MorphType>(morphType), value);
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
  if (!IsValidMorphType(morphType))
    return 0.0f;
  return Morph::GetMinValue(static_cast<Morph::MorphType>(morphType));
}

static float GetMorphMaxValue(RE::StaticFunctionTag*, int morphType)
{
  if (!IsValidMorphType(morphType))
    return 1.0f;
  return Morph::GetMaxValue(static_cast<Morph::MorphType>(morphType));
}

// 获取 MorphType 对应的 SKEE slider 名字
static RE::BSFixedString GetMorphSliderName(RE::StaticFunctionTag*, int morphType)
{
  if (!IsValidMorphType(morphType))
    return "";
  return RE::BSFixedString(Morph::GetMorphName(static_cast<Morph::MorphType>(morphType)));
}

// =========================================================================
//  注册
// =========================================================================

bool RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
{
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
  vm->RegisterFunction("GetMorphSliderName", SCRIPT_NAME, GetMorphSliderName);

  logger::info("[InflationFramework] Papyrus functions registered on {}", SCRIPT_NAME);
  return true;
}

}  // namespace Papyrus
