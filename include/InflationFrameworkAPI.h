#pragma once

// =========================================================================
//  InflationFramework Public API
//
//  其他 DLL (如 FertilityMode) include 此头文件,
//  通过 SKSE MessagingInterface 调用 InflationFramework 功能.
//
//  用法 (消费端):
//    #include "InflationFrameworkAPI.h"
//
//    // 在 kPostPostLoad 之后:
//    auto* api = InflationFrameworkAPI::GetAPI();
//    if (api) {
//        api->SetInflation(actor, InflationFrameworkAPI::MorphType::Belly, 0.5f);
//        float v = api->GetInflation(actor, InflationFrameworkAPI::MorphType::BellyPregnancy);
//    }
// =========================================================================

#include <cstdint>

namespace InflationFrameworkAPI
{

// 版本
inline constexpr std::uint32_t kAPIVersion = 1;

// Morph 类型 — 必须与 InflationFramework 内部 Morph::MorphType 一致
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

// ─── API 接口 (虚函数表, 跨 DLL 安全) ───
class IInflationFrameworkInterface
{
public:
  virtual ~IInflationFrameworkInterface() = default;

  virtual std::uint32_t GetVersion() const = 0;

  // 根据滑条名获取 MorphType, 没有则返回 0 (注意 MorphType::Total 之前的值是保留的)
  virtual MorphType GetMorphType(const char* morphName) const = 0;

  // 注册自定义 Morph, 供 GetMorphTypeByName 和后续的 Get/SetInflation 使用
  virtual std::uint32_t RegisterInflation(const char* morphName, float min, float max) = 0;

  // 注册本地化字符串 (morphName 需要与 RegisterInflation 时一致)
  virtual void RegisterLocalization(const char* morphName, const char* label, const char* desc) = 0;

  // 获取 actor 某 morph 当前值
  virtual float GetInflation(RE::Actor* actor, MorphType type) const = 0;

  // 设置 actor 某 morph 绝对值 (立即应用 morph)
  virtual void SetInflation(RE::Actor* actor, MorphType type, float value) = 0;

  // 增量修改 actor 某 morph (立即应用 morph)
  virtual void ModInflation(RE::Actor* actor, MorphType type, float value) = 0;

  // 按 morph 名字操作 (不走 MorphType 枚举, 直接操作 SKEE slider)
  virtual float GetMorphByName(RE::Actor* actor, const char* morphName) const       = 0;
  virtual void SetMorphByName(RE::Actor* actor, const char* morphName, float value) = 0;

  // 应用所有 morph 变更
  virtual void ApplyMorphs(RE::Actor* actor) = 0;
};

// ─── SKSE Message 交换协议 ───

// 消费端发送此消息到 "InflationFramework" 插件
// InflationFramework 收到后填充 interfacePtr
struct InterfaceExchangeMessage
{
  enum : std::uint32_t
  {
    kExchangeInterface = 'IFXC'
  };

  IInflationFrameworkInterface* interfacePtr = nullptr;
};

// ─── 便捷获取函数 ───

// 在 kPostLoad / kPostPostLoad / kDataLoaded 之后调用
// 返回 nullptr 表示 InflationFramework 未安装或版本不匹配
inline IInflationFrameworkInterface* GetAPI()
{
  InterfaceExchangeMessage msg;
  const auto* messaging = SKSE::GetMessagingInterface();
  if (!messaging)
    return nullptr;

  // 向 InflationFramework 发送接口请求
  messaging->Dispatch(InterfaceExchangeMessage::kExchangeInterface, &msg, sizeof(InterfaceExchangeMessage), "InflationFramework");

  return msg.interfacePtr;
}

}  // namespace InflationFrameworkAPI
