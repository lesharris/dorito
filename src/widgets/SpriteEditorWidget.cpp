#include "SpriteEditorWidget.h"

#include "layers/UI.h"

namespace dorito {
  void SpriteEditorWidget::Draw() {
    auto &bus = Bus::Get();

    if (!ImGui::Begin(ICON_FA_PAINT_BRUSH " Sprite Editor", &UI::ShowSpriteEditor)) {
      ImGui::End();
    } else {

      Toolbar();
      ColorChooser();

      ImGui::Separator();
      ImGui::Dummy({0, 5.0f});

      ImGui::BeginTable("seditortable", 2, ImGuiTableFlags_None);
      ImGui::TableSetupColumn("left", ImGuiTableColumnFlags_WidthFixed, spriteWidth * scale + 2.0f);
      ImGui::TableSetupColumn("right", ImGuiTableColumnFlags_None);
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      SpriteCanvas();

      ImGui::TableSetColumnIndex(1);
      ByteOutput();

      ImGui::EndTable();
      ImGui::End();
    }
  }

  void SpriteEditorWidget::Toolbar() {
    if (ImGui::Checkbox("16x16 Sprite", &hiresSprite)) {
      if (hiresSprite) {
        spriteWidth = 16;
        spriteHeight = 16;
      } else {
        spriteWidth = 8;
        spriteHeight = 16;
      }
    }

    ImGui::SameLine();

    if (ImGui::Checkbox("Color Sprite", &colorSprite)) {
      if (!colorSprite)
        color = 1;
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_LEFT)) {
      for (auto layer = 0; layer < 2; layer++) {
        auto firstColumn = ReadColumn(layer, 0);

        for (auto x = 1; x < (hiresSprite ? 16 : 8); x++) {
          auto col = ReadColumn(layer, x);
          WriteColumn(layer, x - 1, col);
        }

        WriteColumn(layer, hiresSprite ? 15 : 7, firstColumn);
      }
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_RIGHT)) {
      for (auto layer = 0; layer < 2; layer++) {
        auto lastColumn = ReadColumn(layer, hiresSprite ? 15 : 7);

        for (auto x = (hiresSprite ? 15 : 7); x >= 0; x--) {
          auto col = ReadColumn(layer, x - 1);
          WriteColumn(layer, x, col);
        }

        WriteColumn(layer, 0, lastColumn);
      }
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_UP)) {
      for (auto layer = 0; layer < 2; layer++) {
        auto firstRow = ReadRow(layer, 0);

        for (auto y = 1; y < 16; y++) {
          auto row = ReadRow(layer, y);
          WriteRow(layer, y - 1, row);
        }

        WriteRow(layer, 15, firstRow);
      }
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_DOWN)) {
      for (auto layer = 0; layer < 2; layer++) {
        auto lastRow = ReadRow(layer, 15);

        for (auto y = 15; y >= 0; y--) {
          auto row = ReadRow(layer, y - 1);
          WriteRow(layer, y, row);
        }

        WriteRow(layer, 0, lastRow);
      }
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_TRASH)) {
      memset(&bitplanes[0][0], 0, 16 * 16);
      memset(&bitplanes[1][0], 0, 16 * 16);
    }
  }

  void SpriteEditorWidget::ColorChooser() {
    auto &bus = Bus::Get();
    auto &palette = bus.GetDisplay().Palette();

    if (colorSprite) {
      ImGui::Separator();
      ImGui::Dummy({0, 5.0f});

      float sz = ImGui::GetTextLineHeight();

      sz += 10.0f;

      std::vector<std::string> names{
          "Background",
          "Foreground 1",
          "Foreground 2",
          "Blended"
      };

      for (auto i = 0; i < 4; i++) {
        ImVec2 p = ImGui::GetCursorScreenPos();

        ImGui::GetWindowDrawList()->AddRectFilled(p,
                                                  ImVec2(p.x + sz, p.y + sz),
                                                  IM_COL32(
                                                      palette[i].r,
                                                      palette[i].g,
                                                      palette[i].b,
                                                      palette[i].a
                                                  ));

        if (color == i) {
          ImGui::GetWindowDrawList()->AddRect({p.x - 1, p.y - 1}, ImVec2(p.x + sz + 1, p.y + sz + 1), IM_COL32_WHITE,
                                              0, 0, 2.0f);
        }

        auto id = fmt::format("color##{}", i);
        if (ImGui::InvisibleButton(id.c_str(), ImVec2(sz, sz))) {
          color = i;
        }
        ImGui::SameLine();

        if (ImGui::Button(names[i].c_str())) {
          color = i;
        }

        if (i != 3)
          ImGui::SameLine();
      }
    }

  }

  std::vector<uint8_t> SpriteEditorWidget::ReadRow(uint8_t plane, uint8_t row) {
    std::vector<uint8_t> result;

    for (auto x = 0; x < 16; x++) {
      result.push_back(bitplanes[plane][row * spriteWidth + x]);
    }

    return result;
  }

  void SpriteEditorWidget::WriteRow(uint8_t plane, uint8_t row, const std::vector<uint8_t> &data) {
    for (auto x = 0; x < 16; x++) {
      bitplanes[plane][row * spriteWidth + x] = data[x];
    }
  }

  std::vector<uint8_t> SpriteEditorWidget::ReadColumn(uint8_t plane, uint8_t column) {
    std::vector<uint8_t> result;

    for (auto y = 0; y < 16; y++) {
      result.push_back(bitplanes[plane][y * spriteWidth + column]);
    }

    return result;
  }

  void SpriteEditorWidget::WriteColumn(uint8_t plane, uint8_t column, const std::vector<uint8_t> &data) {
    for (auto y = 0; y < 16; y++) {
      bitplanes[plane][y * spriteWidth + column] = data[y];
    }
  }

  void SpriteEditorWidget::SpriteCanvas() {
    auto &bus = Bus::Get();
    auto &palette = bus.GetDisplay().Palette();

    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

    canvas_sz.x = spriteWidth * scale;
    canvas_sz.y = spriteHeight * scale;

    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(palette[0].r, palette[0].g, palette[0].b, palette[0].a));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

    ImGui::InvisibleButton("spritecanvas", canvas_sz,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    const bool is_hovered = ImGui::IsItemHovered();
    const ImVec2 origin(canvas_p0.x, canvas_p0.y);
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    if (is_hovered && (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
      bool leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
      bool rightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

      uint8_t tx = static_cast<uint8_t>(mouse_pos_in_canvas.x / scale);
      uint8_t ty = static_cast<uint8_t>(mouse_pos_in_canvas.y / scale);

      if (colorSprite) {
        switch (color) {
          case 0:
            bitplanes[0][ty * spriteWidth + tx] = 0;
            bitplanes[1][ty * spriteWidth + tx] = 0;
            break;

          case 1:
            bitplanes[0][ty * spriteWidth + tx] = 1;
            bitplanes[1][ty * spriteWidth + tx] = 0;
            break;

          case 2:
            bitplanes[0][ty * spriteWidth + tx] = 0;
            bitplanes[1][ty * spriteWidth + tx] = 1;
            break;

          case 3:
            bitplanes[0][ty * spriteWidth + tx] = 1;
            bitplanes[1][ty * spriteWidth + tx] = 1;
            break;

          default:
            break;
        }
      } else {
        if (leftDown) {
          bitplanes[0][ty * spriteWidth + tx] = 1;
          bitplanes[1][ty * spriteWidth + tx] = 0;
        }

        if (rightDown) {
          bitplanes[0][ty * spriteWidth + tx] = 0;
          bitplanes[1][ty * spriteWidth + tx] = 0;
        }
      }
    }

    const float GRID_STEP = scale;
    for (float x = fmodf(0.0f, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
      draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y),
                         IM_COL32(200, 200, 200, 40));
    for (float y = fmodf(0.0f, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
      draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y),
                         IM_COL32(200, 200, 200, 40));

    for (auto y = 0; y < spriteHeight; y++) {
      for (auto x = 0; x < spriteWidth; x++) {
        uint8_t p1 = bitplanes[0][y * spriteWidth + x];
        uint8_t p2 = bitplanes[1][y * spriteWidth + x];
        uint8_t entry = p2 << 1 | p1;
        auto col = IM_COL32_BLACK;

        if (!colorSprite) {
          if (p1) {
            col = IM_COL32(palette[1].r, palette[1].g, palette[1].b, palette[1].a);
          } else {
            col = IM_COL32(palette[0].r, palette[0].g, palette[0].b, palette[0].a);
          }
        } else {
          col = IM_COL32(palette[entry].r, palette[entry].g, palette[entry].b, palette[entry].a);
        }

        ImVec2 min{
            origin.x + x * scale,
            origin.y + y * scale
        };

        ImVec2 max{
            min.x + scale,
            min.y + scale
        };

        if (entry != 0)
          draw_list->AddRectFilled(min, max, col);
      }
    }
  }

  void SpriteEditorWidget::ByteOutput() {
    std::string output;

    auto getNibble = [&](const std::vector<uint8_t> &data, uint16_t &index) {
      uint8_t nibble = 0;

      for (auto i = 0; i < 4; i++) {
        uint8_t bit = (3 - i);
        uint8_t byte = data[index++];

        nibble |= (byte << bit);

      }
      return nibble;
    };

    for (auto plane = 0; plane < (colorSprite ? 2 : 1); plane++) {
      uint16_t index = 0;
      uint16_t count = 0;

      do {
        uint8_t high = getNibble(bitplanes[plane], index);
        uint8_t low = getNibble(bitplanes[plane], index);

        uint8_t packed = (high << 4) | low;

        output += fmt::format("0x{:02X} ", packed);

        count++;
        if ((count + (plane * 2) + (!hiresSprite * (plane * 4))) % 10 == 0) {
          output += "\n";
        }

      } while (count < (hiresSprite ? 32 : 16));
    }


    if (ImGui::Button(ICON_FA_COPY " Copy", {100, 30})) {
      ImGui::SetClipboardText(output.c_str());
    }

    ImGui::InputTextMultiline("##source", (char *) output.c_str(), output.size(),
                              ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 20),
                              ImGuiInputTextFlags_ReadOnly);
  }
} // dorito