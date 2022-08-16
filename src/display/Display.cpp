#include "Display.h"

#include <iostream>

namespace dorito {
  Display::Display() {
    EventManager::Get().Attach<
        Events::SetColor,
        &Display::HandleSetColor
    >(this);

    EventManager::Get().Attach<
        Events::SetPalette,
        &Display::HandleSetPalette
    >(this);

    Reset();
  }

  Display::~Display() {
    EventManager::Get().DetachAll(this);
  }

  void Display::Reset() {
    m_PlaneMask = 0x1;
    m_HighRes = false;
    memset(&m_Buffer[0][0], 0, 128 * 64);
    memset(&m_Buffer[1][0], 0, 128 * 64);
  }

  bool Display::Plot(uint8_t plane, uint8_t x, uint8_t y) {
    if (!m_HighRes) {
      x *= 2;
      y *= 2;
    }

    return SetPixel(plane, x, y);
  }

  void Display::Clear() {
    for (auto layer = 0; layer < 2; layer++) {
      if ((m_PlaneMask & (layer + 1)) == 0)
        continue;

      memset(&m_Buffer[layer][0], 0, 128 * 64);
    }
  }

  void Display::CopyRow(uint8_t source, uint8_t destination) {
    for (auto i = 0; i < 2; i++) {

      if ((m_PlaneMask & (i + 1)) == 0)
        continue;

      std::copy_n(m_Buffer[i].cbegin() + source * 128,
                  128,
                  m_Buffer[i].begin() + destination * 128);
    }
  }

  void Display::CopyColumn(uint8_t source, uint8_t destination) {
    for (auto i = 0; i < 2; i++) {

      if ((m_PlaneMask & (i + 1)) == 0)
        continue;

      for (int y = 0; y < 64; ++y) {
        auto rowOffset = y * 128;
        m_Buffer[i][destination + rowOffset] = m_Buffer[i][source + rowOffset];
      }
    }
  }

  void Display::ClearRow(uint8_t row) {
    for (auto i = 0; i < 2; i++) {

      if ((m_PlaneMask & (i + 1)) == 0)
        continue;

      std::fill_n(m_Buffer[i].begin() + row * 128, 128, 0);
    }
  }

  void Display::ClearColumn(uint8_t column) {
    for (auto i = 0; i < 2; i++) {

      if ((m_PlaneMask & (i + 1)) == 0)
        continue;

      for (int y = 0; y < 64; ++y) {
        m_Buffer[i][column + (y * 128)] = 0;
      }
    }
  }

  void Display::ScrollDown(uint8_t count) {
    for (int y = 64 - count - 1; y >= 0; --y) {
      CopyRow(y, y + count);
    }

    for (int y = 0; y < count; ++y) {
      ClearRow(y);
    }
  }

  void Display::ScrollUp(uint8_t count) {
    for (int y = 0; y < (64 - count); ++y) {
      CopyRow(y + count, y);
    }

    for (int y = 0; y < count; ++y) {
      ClearRow(64 - y - 1);
    }
  }

  void Display::ScrollLeft(uint8_t count) {
    for (int x = 0; x < (128 - count); ++x) {
      CopyColumn(x + count, x);
    }

    for (int x = 0; x < count; ++x) {
      ClearColumn(128 - x - 1);
    }
  }

  void Display::ScrollRight(uint8_t count) {
    for (int x = 128 - count - 1; x >= 0; --x) {
      CopyColumn(x, x + count);
    }

    for (int x = 0; x < count; ++x) {
      ClearColumn(x);
    }
  }

  bool Display::SetPixel(uint8_t plane, uint8_t x, uint8_t y) {
    bool result = false;

    x %= 128;
    y %= 64;

    uint16_t indexTL = (y * 128) + x;
    uint16_t indexTR = (y * 128) + x + 1;
    uint16_t indexBL = ((y + 1) * 128) + x;
    uint16_t indexBR = ((y + 1) * 128) + x + 1;

    if (y + 1 >= 64) {
      indexBR = indexTL;
      indexBL = indexTL;
    }

    if (x + 1 >= 128) {
      indexTR = indexTL;
      indexBR = indexTL;
    }

    if (m_Buffer[plane][indexTL]) {
      m_Buffer[plane][indexTL] = 0;
      result = true;
    } else {
      m_Buffer[plane][indexTL] = 1;
    }

    if (!m_HighRes) {
      m_Buffer[plane][indexTR] = m_Buffer[plane][indexTL];
      m_Buffer[plane][indexBL] = m_Buffer[plane][indexTL];
      m_Buffer[plane][indexBR] = m_Buffer[plane][indexTL];
    }

    return result;
  }

  void Display::HandleSetColor(const Events::SetColor &event) {
    m_Palette[event.index] = event.color;
  }

  void Display::HandleSetPalette(const Events::SetPalette &event) {
    m_Palette = event.palette;
  }

} // dorito