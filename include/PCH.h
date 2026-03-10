#pragma once

#include "RE/Skyrim.h"
#include "REL/REL.h"
#include "REX/REX.h"
#include "SKSE/SKSE.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <format>
#include <mutex>
#include <ranges>
#include <set>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr std::string_view PLUGIN_NAME = "InflationFramework";
constexpr std::uint32_t MOD            = 'IFNG';

constexpr std::uint32_t hash(const char* data, size_t const size) noexcept
{
  uint32_t hash = MOD;
  for (const char* c = data; c < data + size; ++c) {
    hash = ((hash << 5) + hash) + (unsigned char)*c;
  }
  return hash;
}
constexpr std::uint32_t hash(std::string_view str) noexcept
{
  return hash(str.data(), str.size());
}
constexpr std::uint32_t operator""_h(const char* str, size_t size) noexcept
{
  return hash(str, size);
}

namespace logger = SKSE::log;