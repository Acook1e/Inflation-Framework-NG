#pragma once

#include "morph.h"

namespace InflationManager
{
float GetInflation(RE::Actor* actor, Morph::MorphType type);
void SetInflation(RE::Actor* actor, Morph::MorphType type, float value);

void SaveData();
void LoadData();
void RevertData();
};  // namespace InflationManager