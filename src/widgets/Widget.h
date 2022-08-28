#pragma once

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <raylib.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "common/common.h"

#include "external/imgui_extra/imgui_memory_editor.h"

#include "system/Bus.h"
#include "external/octo_compiler.h"
#include "external/IconsFontAwesome6.h"

namespace dorito {
  extern ImFont *MainFont;
  extern ImFont *MonoFont;
  
  class Widget {
  public:
    Widget() = default;

  public:
    virtual ~Widget() = default;

    virtual void Draw() = 0;

    virtual std::string Name() { return "Widget"; }

    virtual bool Enabled() const { return m_Enabled; }

    virtual void Enabled(bool isEnabled) { m_Enabled = isEnabled; }

  public:
    template<class T>
    static Ref<T> Create() {
      return CreateRef<T>();
    }

  protected:
    bool m_Enabled = false;
  };
}