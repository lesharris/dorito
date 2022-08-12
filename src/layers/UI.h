#pragma once

#include <raylib.h>

#include "common/common.h"
#include "core/Dorito.h"

#include "core/events/EventManager.h"
#include "core/layers/GameLayer.h"

#include "external/glfw/include/GLFW/glfw3.h"

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui_extra/imgui_memory_editor.h"

namespace dorito {

  class UI : public GameLayer {
  public:
    void OnAttach() override;

    void OnDetach() override;

    void Render() override;

    std::string Name() const noexcept override;

    bool RenderTarget() override { return false; }

    void BeginFrame() override;

    void EndFrame() override;

  private:
    void Viewport();

  private:
    ImVec2 GetLargestSizeForViewport();

    ImVec2 GetCenteredPositionForViewport(ImVec2 &windowSize);

  private:
    void HandleKeyPress(Events::KeyPressed &event);

  private:
    Dorito &app = Dorito::Get();

    bool m_ShowDemo = false;
    bool m_ShowEmu = true;

    ImVec2 m_PreviousWindowSize{0, 0};
    ImVec2 m_PreviousMousePosition{0, 0};

    int32_t m_PrevScreenWidth;
    int32_t m_PrevScreenHeight;
  };

} // dorito

