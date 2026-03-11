#pragma once

#include "morph.h"

// MOD 已在 PCH.h 中定义

namespace InflationManager
{

void Initialize();

// 注册一个新的 inflation morph，返回值为该 morph 的 hash
// 返回 0 证明注册失败（已存在同名 morph）
std::uint32_t RegisterInflation(std::string morphName, float min = 0.0f, float max = 1.0f);

float GetInflation(RE::Actor* actor, Morph::MorphType type);
void SetInflation(RE::Actor* actor, Morph::MorphType type, float value);
void ModInflation(RE::Actor* actor, Morph::MorphType type, float value);

void SaveData(SKSE::SerializationInterface* serial);
void LoadData(SKSE::SerializationInterface* serial);
void RevertData(SKSE::SerializationInterface* serial);

}  // namespace InflationManager
