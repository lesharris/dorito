#pragma once

#include "Widget.h"

namespace dorito {

  class SpriteEditorWidget : public Widget {
  public:
    std::string Name() override {
      return "SpriteEditor";
    }

    void Draw() override;

  private:
    void Toolbar();

    void ColorChooser();

    void SpriteCanvas();

    void ByteOutput();

  private:
    std::vector<uint8_t> ReadRow(uint8_t plane, uint8_t row);

    void WriteRow(uint8_t plane, uint8_t row, const std::vector<uint8_t> &data);

    std::vector<uint8_t> ReadColumn(uint8_t plane, uint8_t column);

    void WriteColumn(uint8_t plane, uint8_t column, const std::vector<uint8_t> &data);

  private:
    std::vector<std::vector<uint8_t>> bitplanes{
        std::vector<uint8_t>(16 * 16),
        std::vector<uint8_t>(16 * 16)
    };

    uint8_t spriteWidth = 16;
    uint8_t spriteHeight = 16;

    float scale = 24.0f;
    uint8_t color = 1;
    bool colorSprite = true;
    bool hiresSprite = true;
  };

} // dorito
