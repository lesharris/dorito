#pragma once

#include <raylib.h>
#include <vector>

#include "common/common.h"
#include "core/Dorito.h"

#include "core/events/EventManager.h"
#include "core/layers/GameLayer.h"

#include "external/glfw/include/GLFW/glfw3.h"

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"
#include "external/imgui/backends/imgui_impl_glfw.h"

#include "system/Bus.h"

#include "widgets/AudioWidget.h"
#include "widgets/ColorEditorWidget.h"
#include "widgets/DisassemblyWidget.h"
#include "widgets/EditorWidget.h"
#include "widgets/MainMenuWidget.h"
#include "widgets/MemoryEditorWidget.h"
#include "widgets/RegistersWidget.h"
#include "widgets/ViewportWidget.h"
#include "widgets/SpriteEditorWidget.h"
#include "widgets/SoundEditorWidget.h"
#include "widgets/MonitorsWidget.h"
#include "widgets/BreakpointsWidget.h"

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

  public:
    static bool ShowDemo;
    static bool ShowEmu;
    static bool ShowMemory;
    static bool ShowRegisters;
    static bool ShowDisassembly;
    static bool ShowAudio;
    static bool ShowColorEditor;
    static bool ShowCodeEditor;
    static bool ShowSoundEditor;
    static bool ShowSpriteEditor;
    static bool ShowMonitors;
    static bool ShowBreakpoints;
    static uint16_t PrevPC;

  private:
    void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle);

    ImVec2 ImRotate(const ImVec2 &v, float cos_a, float sin_a);

  private:
    void HandleKeyPress(Events::KeyPressed &event);

    void HandleResetPC(Events::UIResetPC &event);

    AudioWidget m_Audio;
    ColorEditorWidget m_ColorEditor;
    DisassemblyWidget m_Disassembly;
    EditorWidget m_Editor;
    MainMenuWidget m_MainMenu;
    MemoryEditorWidget m_MemoryEditor;
    RegistersWidget m_Registers;
    ViewportWidget m_Viewport;
    SpriteEditorWidget m_SpriteEditor;
    SoundEditorWidget m_SoundEditor;
    MonitorsWidget m_Monitors;
    BreakpointsWidget m_Breakpoints;
  };

} // dorito

