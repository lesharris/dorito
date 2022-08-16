#pragma once

#include <raylib.h>
#include <zep.h>
#include <vector>

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

    void Registers();

    void Disassembly();

    void Audio();

    void ColorEditor();

    void CodeEditor();

  private:
    ImVec2 GetLargestSizeForViewport();

    ImVec2 GetCenteredPositionForViewport(ImVec2 &windowSize);

  private:
    void HandleKeyPress(Events::KeyPressed &event);

  private:
    bool m_ShowDemo = false;
    bool m_ShowEmu = true;
    bool m_ShowMemory = true;
    bool m_ShowRegisters = true;
    bool m_ShowDisassembly = true;
    bool m_ShowAudio = true;
    bool m_ShowColorEditor = false;
    bool m_ShowCodeEditor = true;

    bool m_7Cycles = false;
    bool m_15Cycles = false;
    bool m_20Cycles = false;
    bool m_30Cycles = false;
    bool m_100Cycles = false;
    bool m_200Cycles = false;
    bool m_500Cycles = false;
    bool m_1000Cycles = false;
    bool m_10000Cycles = false;

    bool m_ShiftQuirk = false;
    bool m_LoadStoreQuirk = false;
    bool m_JumpQuirk = false;
    bool m_LogicQuirk = false;
    bool m_ClipQuirk = false;
    bool m_LoresQuirk = false;
    bool m_VBlankQuirk = false;
    bool m_IRegCarryQuirk = false;

    bool m_VIPMode = false;
    bool m_SCHIPMode = false;
    bool m_XOMode = false;

    bool m_PalGrey = false;
    bool m_PalOcto = true;
    bool m_PalNeat = false;
    bool m_PalKesh = false;

    std::vector<Color> m_OctoPalette{
        {0x99, 0x66, 0x00, 0xFF},
        {0xFF, 0xCC, 0x00, 0xFF},
        {0xFF, 0x66, 0x00, 0xFF},
        {0x66, 0x22, 0x00, 0xFF}
    };

    std::vector<Color> m_KeshaPalette = {
        {0x28, 0x35, 0x93, 0xFF},
        {0xB2, 0x2D, 0x10, 0xFF},
        {0x10, 0xB2, 0x58, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF}
    };

    std::vector<Color> m_NeatPalette = {
        {0x10, 0,    0x10, 0xFF},
        {0xE6, 0xE6, 0xFA, 0xFF},
        {0xFF, 0x14, 0x93, 0xFF},
        {0xFF, 0x14, 0x93, 0xFF}
    };

    std::vector<Color> m_GreyPalette = {
        BLACK,
        WHITE,
        {0xAA, 0xAA, 0xAA, 0xFF},
        {0x55, 0x55, 0x55, 0xFF}
    };

    ImVec2 m_PreviousWindowSize{0, 0};
    ImVec2 m_PreviousMousePosition{0, 0};

    int32_t m_PrevScreenWidth;
    int32_t m_PrevScreenHeight;

    uint16_t m_PrevPC = 0;

    Scope<Zep::ZepEditor_ImGui> m_Editor;
    Zep::ZepBuffer *m_CodeBuffer;
  };

} // dorito

