#pragma once

namespace Morph
{
enum class MorphType : std::uint8_t
{
  Belly,           // 腹部整体
  BellyMid,        // 腹部中间
  BellyUnder,      // 下腹部
  BellyPregnancy,  // 怀孕腹部
  Breasts,         // 胸部整体
  Butt,            // 臀部整体
};

struct MorphData
{
  std::string morphName;
  float min;
  float max;
};

std::uint32_t GetHash(MorphType type);

float GetMinValue(MorphType type);
float GetMaxValue(MorphType type);

float GetMorphByName(RE::Actor* actor, std::string_view morphName);
float GetMorphByType(RE::Actor* actor, MorphType morphType);

void SetMorphByName(RE::Actor* actor, std::string_view morphName, float value);
void SetMorphByType(RE::Actor* actor, MorphType morphType, float value);

bool HasMorph(RE::Actor* actor, std::string_view morphName);

void ApplyMorphs(RE::Actor* actor);

void Initialize();
}  // namespace Morph