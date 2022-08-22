#pragma once

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <raylib.h>

#include "common/common.h"

#include "external/imgui/imgui.h"
#include "external/imgui_extra/imgui_memory_editor.h"
#include "imgui_internal.h"

#include "system/Bus.h"
#include "external/octo_compiler.h"
#include "external/IconsFontAwesome5.h"

namespace dorito {
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