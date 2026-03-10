#pragma once

namespace Morph
{
enum class MorphType
{
  Belly,
  Breasts,
  Butt,
};
struct MorphData
{
  std::string morphName;
  std::string morphKey;
  MorphType morphType;
};

void SetMorphByName(RE::Actor* actor, const char* morphName, const char* morphKey, float value);
void SetMorphByHash(RE::Actor* actor, std::uint32_t morphHash, float value);
void SetMorphByMap(RE::Actor* actor, const char* mapName, float value);

void Initialize();
}  // namespace Morph