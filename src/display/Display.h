#pragma once

#include <raylib.h>
#include <cstdint>
#include <vector>

#include "core/events/EventManager.h"

namespace dorito {

  class Display {
  public:
    Display();

    ~Display();

    void Reset();

    void HighRes(bool isHighRes) {
      m_HighRes = isHighRes;
    }

    void PlaneMask(uint8_t mask) {
      m_PlaneMask = mask & 0x3;
    }

    uint8_t PlaneMask() { return m_PlaneMask; }

    void Clear();

    bool Plot(uint8_t plane, uint8_t x, uint8_t y);

    void ScrollDown(uint8_t count);

    void ScrollUp(uint8_t count);

    void ScrollLeft(uint8_t count);

    void ScrollRight(uint8_t count);

  public:
    [[nodiscard]] bool HighRes() const {
      return m_HighRes;
    }

    [[nodiscard]] uint8_t Height() const {
      return m_HighRes ? 64 : 32;
    }

    [[nodiscard]] uint8_t Width() const {
      return m_HighRes ? 128 : 64;
    }

    [[nodiscard]] const std::vector<Color> &Palette() const {
      return m_Palette;
    }

    [[nodiscard]] const std::vector<std::vector<uint8_t>> &Buffers() const {
      return m_Buffer;
    }

  private:
    bool SetPixel(uint8_t plane, uint8_t x, uint8_t y);

    void CopyRow(uint8_t source, uint8_t destination);

    void CopyColumn(uint8_t source, uint8_t destination);

    void ClearRow(uint8_t row);

    void ClearColumn(uint8_t column);

  private:
    void HandleSetColor(const Events::SetColor &event);

    void HandleSetPalette(const Events::SetPalette &event);

  private:
    uint8_t m_PlaneMask = 0x1;

    bool m_HighRes = false;

    std::vector<std::vector<uint8_t>> m_Buffer = {
        std::vector<uint8_t>(128 * 64),
        std::vector<uint8_t>(128 * 64)
    };

    std::vector<Color> m_Palette{
        {0x99, 0x66, 0x00, 0xFF},
        {0xFF, 0xCC, 0x00, 0xFF},
        {0xFF, 0x66, 0x00, 0xFF},
        {0x66, 0x22, 0x00, 0xFF}
    };
  };

} // dorito

