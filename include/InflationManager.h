#pragma once

#include "morph.h"

// MOD 已在 PCH.h 中定义

namespace InflationManager
{

void Initialize();

float GetInflation(RE::Actor* actor, Morph::MorphType type);
void SetInflation(RE::Actor* actor, Morph::MorphType type, float value);
void ModInflation(RE::Actor* actor, Morph::MorphType type, float value);

void SaveData(SKSE::SerializationInterface* serial);
void LoadData(SKSE::SerializationInterface* serial);
void RevertData(SKSE::SerializationInterface* serial);

}  // namespace InflationManager
