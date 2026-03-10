#pragma once

#include "morph.h"

namespace InflationManager
{
float GetInflation(RE::Actor* actor, Morph::MorphType type);
void SetInflation(RE::Actor* actor, Morph::MorphType type, float value);

void ModInflation(RE::Actor* actor, Morph::MorphType type, float value);

void Initialize();
};  // namespace InflationManager