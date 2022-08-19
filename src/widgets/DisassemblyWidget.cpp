#include "DisassemblyWidget.h"

#include "layers/UI.h"

namespace dorito {
  void DisassemblyWidget::Draw() {
    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();

    if (!ImGui::Begin("Disassembly", &UI::ShowDisassembly)) {
      ImGui::End();
    } else {

      if (ImGui::BeginTable("disa", 3, ImGuiTableFlags_ScrollY |
                                       ImGuiTableFlags_BordersOuterH |
                                       ImGuiTableFlags_BordersOuterV |
                                       ImGuiTableFlags_RowBg)) {
        auto &lines = cpu.m_Disassembly;

        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthStretch);

        ImGuiListClipper clipper;
        size_t lineSize = lines.size();
        clipper.Begin((int) lineSize);

        auto lineByIndex = [lines](int index) -> Chip8::DisassemblyLine {
          for (const auto &[_, line]: lines) {
            if (line.index == index)
              return line;
          }

          return Chip8::DisassemblyLine{
              0xFFFF,
              0xFFFF,
              "",
              ""
          };
        };

        bool lineHighlite = false;

        while (clipper.Step()) {
          for (auto item = clipper.DisplayStart; item < clipper.DisplayEnd; item++) {
            auto line = lineByIndex(item);

            if (line.index == 0xFFFF)
              continue;

            ImGui::TableNextRow();

            if (!lineHighlite && (line.index == lines.size() - 1 ||
                                  line.addr == cpu.m_PrevPC)) {
              ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.18f, 0.47f, 0.59f, 0.65f));
              ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, row_bg_color);
              lineHighlite = true;
            }

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(fmt::format("${:04X}", line.addr).c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(fmt::format("{}", line.bytes).c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(line.text.c_str());
          }
        }

        if (cpu.m_PrevPC != UI::PrevPC) {
          auto addr = cpu.m_PrevPC;

          if (lines.contains(addr)) {
            auto size = ImGui::GetWindowSize();
            auto &line = lines[addr];
            ImGui::SetScrollY(
                (clipper.ItemsHeight * (float) line.index - (size.y / 2) -
                 (clipper.ItemsHeight / clipper.ItemsCount)));
            UI::PrevPC = addr;
          }
        }

        ImGui::EndTable();

      }

      ImGui::End();
    }
  }
} // dorito