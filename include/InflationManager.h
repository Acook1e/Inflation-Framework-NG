#pragma once

class InflationManager
{
public:
  InflationManager& GetSingleton()
  {
    static InflationManager singleton;
    return singleton;
  }

  struct InflationData
  {
    std::uint32_t morphHash;
    float inflatedValue;
  };

  static void SaveData();
  static void LoadData();
  static void RevertData();

private:
  std::unordered_map<RE::FormID, std::vector<InflationData>> inflationDataMap;
  std::unordered_map<RE::FormID, std::vector<InflationData>> runtimeInflationDataMap;
};