#pragma once

#include <string_view>
#include <unordered_map>

namespace Morph
{

// ─── MorphType ───
// 枚举名必须与 InflationFramework_Morphs.json 的 key 完全一致
// magic_enum 依赖枚举名做 JSON key → enum 映射
enum class MorphType : std::uint8_t
{
  Belly          = 0,
  BellyMid       = 1,
  BellyUnder     = 2,
  BellyPregnancy = 3,
  Breasts        = 4,
  Butt           = 5,
};

struct MorphData
{
  std::string morphName;
  float min;
  float max;
};

void Initialize();

std::uint32_t GetHash(MorphType type);
std::string_view GetMorphName(MorphType type);
float GetMinValue(MorphType type);
float GetMaxValue(MorphType type);

float GetMorphByName(RE::Actor* actor, std::string_view morphName);
float GetMorphByType(RE::Actor* actor, MorphType morphType);

void SetMorphByName(RE::Actor* actor, std::string_view morphName, float value);
void SetMorphByType(RE::Actor* actor, MorphType morphType, float value);

bool HasMorph(RE::Actor* actor, std::string_view morphName);

void ApplyMorphs(RE::Actor* actor);

}  // namespace Morph
