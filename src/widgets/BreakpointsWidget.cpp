#include "BreakpointsWidget.h"

#include "layers/UI.h"

namespace dorito {
  void BreakpointsWidget::Draw() {
    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();
    auto &breakpoints = cpu.Breakpoints();

    if (!ImGui::Begin(ICON_FA_CIRCLE " Breakpoints", &UI::ShowBreakpoints)) {
      ImGui::End();
    } else {
      ImGui::BeginTable("breakpoints", 4, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("##1", ImGuiTableColumnFlags_WidthFixed, 30.0f);
      ImGui::TableSetupColumn("Name##2", ImGuiTableColumnFlags_WidthFixed, 50.0f);
      ImGui::TableSetupColumn("Address##3", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("##4", ImGuiTableColumnFlags_WidthFixed, 60.0f);
      ImGui::TableHeadersRow();

      int32_t count = 0;
      for (const auto &bp: breakpoints) {
        auto checkId = fmt::format("##{}", count++);
        bool enabled = bp.enabled;
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Checkbox(checkId.c_str(), &enabled)) {
          cpu.ToggleBreakpoint(bp);
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", bp.label.c_str());

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%s", fmt::format("0x{:04X}", bp.addr).c_str());

        ImGui::TableSetColumnIndex(3);
        if (ImGui::Button("Remove")) {
          cpu.RemoveBreakpoint(bp);
          break;
        }
      }

      ImGui::EndTable();

      ImGui::End();
    }
  }
} // dorito