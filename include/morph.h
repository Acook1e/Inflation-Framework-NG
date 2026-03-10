#pragma once

namespace Morph
{
enum class MorphType : std::uint8_t
{
  Belly,
  Pregnancy,
  Breasts,
  Butt,
};

struct MorphData
{
  std::string morphName;
  float min;
  float max;
};

std::unordered_map<std::uint32_t, MorphData>& Get();
void visit();

float GetMorphByName(RE::Actor* actor, std::string_view morphName);
float GetMorphByType(RE::Actor* actor, MorphType morphType);
float GetMorphByMap(RE::Actor* actor, std::string_view mapName);

void SetMorphByName(RE::Actor* actor, std::string_view morphName, float value);
void SetMorphByType(RE::Actor* actor, MorphType morphType, float value);
void SetMorphByMap(RE::Actor* actor, std::string_view mapName, float value);

bool HasMorph(RE::Actor* actor, std::string_view morphName);

void ApplyMorphs(RE::Actor* actor);

void Initialize();
}  // namespace Morph