#pragma once

#include "Widget.h"

namespace dorito {

  class MainMenuWidget : public Widget {
  public:
    void Draw() override;

    std::string Name() override {
      return "MainMenu";
    }

  private:
    void UpdateStatus();

    void DrawMenubar();

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
    void SetQuirkEntry(uint8_t quirkIndex, bool isSet);

    void EnablePalette(uint8_t pal);

  private:
    bool m_DoritoMuted = false;


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

    bool m_About = false;
  };

} // dorito
