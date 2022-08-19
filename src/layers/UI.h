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
#include "external/imgui_extra/imgui_memory_editor.h"

#include "external/zep/ZepEditor.h"

#include "system/Bus.h"

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

    void Code();

  private:
    void DrawMenubar();

  private:
    ImVec2 GetLargestSizeForViewport();

    ImVec2 GetCenteredPositionForViewport(ImVec2 &windowSize);

    void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle);

    ImVec2 ImRotate(const ImVec2 &v, float cos_a, float sin_a);

    void EnablePalette(uint8_t pal);

    void SetQuirkEntry(uint8_t quirkIndex, bool isSet);

    void UpdateMenuStatus();

  private:
    void HandleKeyPress(Events::KeyPressed &event);

  private:
    struct PaletteEntry {
      std::string name;
      std::vector<Color> palette;
    };

    struct QuirkEntry {
      std::string label;
      Chip8::Quirk quirk;
      bool set;
    };

    struct CycleEntry {
      std::string label;
      uint32_t cycles;
      bool set;
    };

    struct ProfileEntry {
      std::string label;
      Bus::CompatProfile profile;
      bool set;
    };

  private:
    bool m_DoritoMuted = false;

    bool m_ShowDemo = false;
    bool m_ShowEmu = true;
    bool m_ShowMemory = true;
    bool m_ShowRegisters = true;
    bool m_ShowDisassembly = true;
    bool m_ShowAudio = true;
    bool m_ShowColorEditor = false;
    bool m_ShowCodeEditor = true;

    std::vector<uint16_t> m_CycleSet{7, 15, 20, 30, 100, 200, 500, 1000, 10000};

    std::vector<CycleEntry> m_CycleEntries{
        {
            "7 Cycles/Frame",
            7,
            false
        },
        {
            "15 Cycles/Frame",
            15,
            false
        },
        {
            "20 Cycles/Frame",
            20,
            false
        },
        {
            "30 Cycles/Frame",
            30,
            false
        },
        {
            "100 Cycles/Frame",
            100,
            true
        },
        {
            "200 Cycles/Frame",
            200,
            false
        },
        {
            "500 Cycles/Frame",
            500,
            false
        },
        {
            "1000 Cycles/Frame",
            1000,
            false
        },
        {
            "sep",
            0,
            false
        },
        {
            "Warp Factor 10k",
            10000,
            false
        }
    };

    std::vector<QuirkEntry> m_QuirkEntries{
        {
            "<<= and >>= modify vx in place and ignore vy",
            Chip8::Quirk::Shift,
            false
        },
        {
            "load and store operations leave i unchanged",
            Chip8::Quirk::LoadStore,
            false
        },
        {
            "load and store operations leave i unchanged",
            Chip8::Quirk::LoadStore,
            false
        },
        {
            "4 high bits of target address determines the offset register of jump0 instead of v0.",
            Chip8::Quirk::Jump,
            false
        },
        {
            "clear vF after vx |= vy, vx &= vy, and vx ^= vy",
            Chip8::Quirk::Logic,
            false
        },
        {
            "clip sprites at screen edges instead of wrapping",
            Chip8::Quirk::Clip,
            false
        },
        {
            "render sprites only in vblank",
            Chip8::Quirk::VBlank,
            false
        },
        {
            "render height 0 sprites as 8x16 sprites in lores mode",
            Chip8::Quirk::LoresSprites,
            false
        },
        {
            "set carry if I overflows, clear otherwise",
            Chip8::Quirk::IRegCarry,
            false
        }
    };

    std::vector<PaletteEntry> m_PaletteEntries{
        {
            "Greyscale",
            {
                {0xAA, 0xAA, 0xAA, 0xFF},
                BLACK,
                WHITE,
                {0x66, 0x66, 0x66, 0xFF}
            }
        },
        {
            "Octo",
            {
                {0x99, 0x66, 0x00, 0xFF},
                {0xFF, 0xCC, 0x00, 0xFF},
                {0xFF, 0x66, 0x00, 0xFF},
                {0x66, 0x22, 0x00, 0xFF}
            }
        },
        {
            "Neatboy",
            {
                {0x10, 0x00, 0x10, 0xFF},
                {0xE6, 0xE6, 0xFA, 0xFF},
                {0xFF, 0x14, 0x93, 0xFF},
                {0xFF, 0x14, 0x93, 0xFF}
            }
        },
        {
            "Kesha",
            {
                {0x28, 0x35, 0x93, 0xFF},
                {0xB2, 0x2D, 0x10, 0xFF},
                {0x10, 0xB2, 0x58, 0xFF},
                {0xFF, 0xFF, 0xFF, 0xFF}
            }
        },
        {
            "LCD",
            {
                {0xF9, 0xFF, 0xB3, 0xFF},
                {0x3D, 0x80, 0x26, 0xFF},
                {0xAB, 0xCC, 0x47, 0xFF},
                {0x00, 0x13, 0x1A, 0xFF}
            }

        },
        {
            "Hotdog",
            {
                BLACK,
                {0xFF, 0x00, 0x00, 0xFF},
                {0xFF, 0xFF, 0x00, 0xFF},
                WHITE
            }
        },
        {
            "CGA0",
            {
                BLACK,
                {0x00, 0xFF, 0x00, 0xFF},
                {0xFF, 0x00, 0x00, 0xFF},
                {0xFF, 0xFF, 0x00, 0xFF}
            }
        },
        {
            "CGA1",
            {
                BLACK,
                {0xFF, 0x00, 0xFF, 0xFF},
                {0x00, 0xFF, 0xFF, 0xFF},
                WHITE
            }
        }
    };

    bool m_PaletteState[9]{
        false, true, false, false,
        false, false, false, false,
        false
    };

    std::vector<ProfileEntry> m_ProfileEntries{
        {
            "VIP",
            Bus::CompatProfile::VIP,
            false
        },
        {
            "SCHIP",
            Bus::CompatProfile::SCHIP,
            false
        },
        {
            "XO-Chip",
            Bus::CompatProfile::XOChip,
            true
        }
    };

    ImVec2 m_PreviousWindowSize{0, 0};
    ImVec2 m_PreviousMousePosition{0, 0};

    int32_t m_PrevScreenWidth;
    int32_t m_PrevScreenHeight;

    uint16_t m_PrevPC = 0;

    CodeEditor m_Editor{"Code.o8"};
  };

} // dorito

