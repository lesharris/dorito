#include "AudioWidget.h"

#include "layers/UI.h"

namespace dorito {
  void AudioWidget::Draw() {
    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();
    auto &buffer = bus.GetRam().GetAudioBuffer();

    bool wasEnabled = m_Enabled;

    ImGui::SetNextWindowSize({400, 350}, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(ICON_FA_HEADPHONES " Audio", &m_Enabled)) {
      ImGui::End();
    } else {
      ImGui::BeginTable("pitch", 1, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Pitch", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%gHz", cpu.regs.pitch);

      ImGui::EndTable();

      ImGui::Separator();

      ImGui::BeginTable("audiobuffer", 1, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Audio Buffer", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);

      for (auto n = 0; n < 16; n++) {
        ImGui::Text("0x%02X ", buffer[n]);

        if (n != 7)
          ImGui::SameLine();
      }

      ImGui::EndTable();

      ImGui::Separator();

      auto patternToBits = [](const std::vector<uint8_t> &pattern) {
        std::vector<float> bits = std::vector<float>(512);
        auto i = 0;

        for (const uint8_t byte: pattern) {
          uint8_t mask = 128;

          while (mask != 0) {
            const float val = (float) ((byte & mask) != 0 ? 1 : 0);
            bits[i++] = val;
            bits[i++] = val;
            bits[i++] = val;
            bits[i++] = val;
            mask >>= 1;
          }
        }

        return bits;
      };

      auto bits = patternToBits(buffer);

      ImGui::PlotLines("Waveform",
                       &bits[0], 512,
                       0, nullptr, -0.2f, 1.2f,
                       ImVec2(0, 100.0f));


      if (!m_Enabled && wasEnabled) {
        EventManager::Dispatcher().enqueue<Events::SaveAppPrefs>();
      }

      ImGui::End();
    }
  }
} // dorito