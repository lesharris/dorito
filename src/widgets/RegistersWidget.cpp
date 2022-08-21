#include "RegistersWidget.h"

#include "layers/UI.h"

namespace dorito {
  void RegistersWidget::Draw() {
    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();

    if (!ImGui::Begin(ICON_FA_MICROCHIP " Registers", &UI::ShowRegisters)) {
      ImGui::End();
    } else {

      ImGui::BeginTable("flagsbtnscycles", 2);
      ImGui::TableSetupColumn("btns");
      ImGui::TableSetupColumn("cycles");

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      if (ImGui::Button(bus.Running() ? ICON_FA_PAUSE " Pause" : ICON_FA_PLAY " Run")) {
        EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({!bus.Running()});
      }

      ImGui::SameLine();

      if (!bus.Running()) {
        if (ImGui::Button(ICON_FA_STEP_FORWARD " Step")) {
          EventManager::Dispatcher().enqueue<Events::StepCPU>({});
        }
      }

      ImGui::TableSetColumnIndex(1);
      auto cycleInfo = fmt::format("Total Cycles: {}", cpu.m_Cycles);
      auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(cycleInfo.c_str()).x
                   - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
      if (posX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(posX);
      ImGui::TextUnformatted(cycleInfo.c_str());

      ImGui::EndTable();

      ImGui::Separator();

      if (!bus.Running()) {
        static uint16_t addr = 0x200;

        ImGui::Text("Run to Address:");
        ImGui::SameLine();
        ImGui::InputScalar("##", ImGuiDataType_U16, &addr, nullptr, nullptr, "0x%04X");
        if (ImGui::Button("Go...")) {
          EventManager::Dispatcher().enqueue<Events::ExecuteUntil>({addr});
        }
      }

      ImGui::Separator();

      ImGui::BeginTable("pci", 3, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("I", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Latch", ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(fmt::format("0x{:04X}", cpu.regs.pc).c_str());

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(fmt::format("0x{:04X}", cpu.regs.i).c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(fmt::format("0x{:04X}", cpu.regs.latch).c_str());


      ImGui::EndTable();

      ImGui::BeginTable("dtst", 2, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Delay Timer", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Sound Timer", ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.dt).c_str());

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.st).c_str());

      ImGui::EndTable();

      ImGui::BeginTable("regs07", 8, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("V0", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V1", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V2", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V3", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V4", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V5", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V6", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V7", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      for (auto r = 0; r < 8; r++) {
        ImGui::TableSetColumnIndex(r);
        ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.v[r]).c_str());
      }

      ImGui::EndTable();

      ImGui::BeginTable("regs8F", 8, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("V8", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V9", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VA", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VB", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VC", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VD", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VE", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VF", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      for (auto r = 8; r < 16; r++) {
        ImGui::TableSetColumnIndex(r - 8);
        ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.v[r]).c_str());
      }

      ImGui::EndTable();

      ImGui::Separator();

      ImGui::BeginTable("opsstack", 2, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Operands", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Stack", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      ImGui::BeginTable("ops", 2, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("OpType", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("OpValue", ImGuiTableColumnFlags_None);

      if (cpu.m_CurrentInstruction) {
        for (auto i = 0; i < cpu.m_CurrentInstruction->operand_count; i++) {
          ImGui::TableNextRow();
          auto &op = cpu.mOperands[i];
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%s", cpu.OperandTypeName(op.type).c_str());
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("0x%X (%d)", op.value, op.value);
        }
      }

      ImGui::EndTable();

      ImGui::TableSetColumnIndex(1);

      ImGui::BeginTable("stack", 1, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("address", ImGuiTableColumnFlags_None);

      for (const auto &item: bus.GetRam().GetStack()) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("0x%X", item);
      }

      ImGui::EndTable();


      ImGui::EndTable();


      ImGui::End();
    }
  }
} // dorito