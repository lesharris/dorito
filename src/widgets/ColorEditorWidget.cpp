#include "ColorEditorWidget.h"

#include "layers/UI.h"

namespace dorito {
  void ColorEditorWidget::Draw() {
    auto &bus = Bus::Get();

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    auto &palette = bus.GetDisplay().Palette();

    auto colorToImvec = [](const Color &color) {
      return ImVec4{
          color.r / 255.0f,
          color.g / 255.0f,
          color.b / 255.0f,
          color.a / 255.0f
      };
    };

    auto ImvecToColor = [](const ImVec4 &vec) {
      return Color{
          static_cast<unsigned char>(vec.x * 255),
          static_cast<unsigned char>(vec.y * 255),
          static_cast<unsigned char>(vec.z * 255),
          static_cast<unsigned char>(vec.w * 255)
      };
    };

    ImVec4 bgColor = colorToImvec(palette[0]);
    ImVec4 fc1Color = colorToImvec(palette[1]);
    ImVec4 fc2Color = colorToImvec(palette[2]);
    ImVec4 blColor = colorToImvec(palette[3]);

    if (!ImGui::Begin("Edit Colors", &UI::ShowColorEditor)) {
      ImGui::End();
    } else {

      ImGui::BeginTable("colors", 4, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Background Color", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Fill Color 1", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Fill Color 2", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Blended Color", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      if (ImGui::ColorPicker3("##bgcolor", (float *) &bgColor, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(bgColor);
        EventManager::Dispatcher().trigger<Events::SetColor>({0, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }

      ImGui::TableSetColumnIndex(1);
      if (ImGui::ColorPicker3("##fc1color", (float *) &fc1Color, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(fc1Color);
        EventManager::Dispatcher().trigger<Events::SetColor>({1, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }

      ImGui::TableSetColumnIndex(2);
      if (ImGui::ColorPicker3("##fc2color", (float *) &fc2Color, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(fc2Color);
        EventManager::Dispatcher().trigger<Events::SetColor>({2, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }

      ImGui::TableSetColumnIndex(3);
      if (ImGui::ColorPicker3("##blcolor", (float *) &blColor, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(blColor);
        EventManager::Dispatcher().trigger<Events::SetColor>({3, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }
      ImGui::EndTable();

      ImGui::End();
    }
  }
} // dorito