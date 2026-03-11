#pragma once

#include "API/SKSEMenuFramework.h"

class Menu
{
public:
  Menu();
  ~Menu();

  static Menu& GetSingleton()
  {
    static Menu singleton;
    return singleton;
  }

  static void InsertLocalization(std::string key, std::string label, std::string desc);

  static void Settings();
  static void Debug();

  static void __stdcall EventListener(SKSEMenuFramework::Model::EventType eventType);

private:
  SKSEMenuFramework::Model::Event* event = nullptr;
};
