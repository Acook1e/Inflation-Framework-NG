#pragma once

#include "API/SKSEMenuFramework.h"

class Menu
{
public:
  Menu();
  ~Menu() { delete event; }

  static Menu& GetSingleton()
  {
    static Menu singleton;
    return singleton;
  }

  static void Settings();

  static void __stdcall EventListener(SKSEMenuFramework::Model::EventType eventType);

private:
  SKSEMenuFramework::Model::Event* event;
};
