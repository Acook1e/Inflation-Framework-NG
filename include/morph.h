#pragma once

#include <string_view>
#include <unordered_map>

namespace Morph
{

// ─── MorphType ───
// 枚举名必须与 InflationFramework_Morphs.json 的 key 完全一致
// magic_enum 依赖枚举名做 JSON key → enum 映射
enum class MorphType : std::uint32_t
{
  Belly          = 0,
  BellyMid       = 1,
  BellyUnder     = 2,
  BellyPregnancy = 3,
  Breasts        = 4,
  Butt           = 5,
  Total
};

struct MorphData
{
  std::string morphName;
  std::uint32_t hash;
  float min;
  float max;
};

void Initialize();

// 所有 MorphType 的访问接口，用于内部调用
std::unordered_map<MorphType, MorphData>& GetMorphDataMap();
MorphData& GetMorphData(MorphType type);

// 对于用户自定义的 Morph，使用 morphName 的 hash 作为 MorphType
// 返回值为 0 证明 hash 冲突 但概率极低 1e-9 级别，基本可以忽略
MorphType GetType(std::string_view morphName);
void RegisterMorph(std::string morphName, float min = 0.0f, float max = 1.0f);

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
