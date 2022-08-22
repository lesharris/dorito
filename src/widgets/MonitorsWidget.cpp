#include "MonitorsWidget.h"

#include "layers/UI.h"

#include <regex>

namespace dorito {

  MonitorsWidget::MonitorsWidget() {
    EventManager::Get().Attach<
        Events::UIClearMonitors,
        &MonitorsWidget::HandleClearMonitors
    >(this);

    EventManager::Get().Attach<
        Events::UIAddMonitor,
        &MonitorsWidget::HandleAddMonitor
    >(this);
  }

  void MonitorsWidget::Draw() {
    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();
    auto &ram = bus.GetRam().GetMemory();

    bool wasEnabled = m_Enabled;

    ImGui::SetNextWindowSize({400, 350}, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(ICON_FA_SEARCH " Monitors", &m_Enabled)) {
      ImGui::End();
    } else {

      ImGui::BeginTable("monitors", 2, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 100.0f);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      for (const auto &item: m_Monitors) {
        ImGui::TableNextRow();

        switch (item.type) {
          case Type::Register: {
            std::string name = fmt::format("v{:X} - v{:X}", item.base, item.length);
            std::string values;

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", name.c_str());

            ImGui::TableSetColumnIndex(1);

            for (auto i = item.base; i <= item.length; i++) {
              if (i > 0 && i % 4 == 0)
                values += "\n";

              values += fmt::format("0x{:02X} ", cpu.regs.v[i]);
            }

            ImGui::Text("%s", values.c_str());
          }
            break;
          case Type::Memory: {
            std::string name = fmt::format("{}", item.name);
            std::string values;

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", name.c_str());

            ImGui::TableSetColumnIndex(1);

            if (item.length == -1 && item.format.size() == 0) {
              ImGui::Text("<Invalid>");
              continue;
            }

            if (item.format.size() == 0) {
              for (auto i = item.base; i <= item.base + item.length; i++) {
                if (i > item.base && i % 4 == 0)
                  values += "\n";

                values += fmt::format("0x{:02X} ", ram[i]);
              }

              ImGui::Text("%s", values.c_str());
            } else {
              auto parsed = ParseFormat(item);
              ImGui::Text("%s", parsed.c_str());
            }
          }
            break;
        }
      }

      ImGui::EndTable();

      if (!m_Enabled && wasEnabled) {
        EventManager::Dispatcher().enqueue<Events::SaveAppPrefs>();
      }

      ImGui::End();
    }
  }

  void MonitorsWidget::HandleClearMonitors(const Events::UIClearMonitors &) {
    m_Monitors.clear();
  }

  void MonitorsWidget::HandleAddMonitor(const Events::UIAddMonitor &event) {
    MonitorItem item;

    item.format = event.format ? std::string{event.format} : "";
    item.name = event.name;
    item.base = event.base;
    item.type = static_cast<Type>(event.type);
    item.length = event.len;

    m_Monitors.push_back(item);
  }

  std::string MonitorsWidget::ParseFormat(const MonitorsWidget::MonitorItem &item) {
    auto &bus = Bus::Get();
    auto &ram = bus.GetRam().GetMemory();

    std::string format = item.format;
    std::regex formatPattern{"%([0-9]+)?([bBiIxXcC]){1}"};
    std::vector<FormatItem> items;

    std::sregex_iterator rend{};
    std::sregex_iterator iter{format.begin(), format.end(), formatPattern};

    while (iter != std::sregex_iterator()) {
      auto &results = *iter;

      if (!results.empty()) {
        std::string fstring = results[1].matched
                              ? fmt::format("{}{}{}", *results[0].first, *results[1].first, *results[2].first)
                              : fmt::format("{}{}", *results[0].first, *results[2].first);

        uint32_t byteCount = 1;

        if (results[1].matched) {
          auto lstring = fmt::format("{}", *results[1].first);
          byteCount = std::stoi(lstring);
        }

        std::string type = fmt::format("{}", *results[2].first);

        std::string value;

        switch (type[0]) {
          case 'b':
          case 'B':
            for (uint32_t i = 0; i < byteCount; i++) {
              if (i != 0)
                value += " ";
              value += fmt::format("0b{:08b}", ram[item.base + i]);
            }
            break;

          case 'i':
          case 'I':
            for (uint32_t i = 0; i < byteCount; i++) {
              if (i != 0)
                value += " ";

              value += fmt::format("{}", ram[item.base + i]);
            }
            break;

          case 'x':
          case 'X':
            for (uint32_t i = 0; i < byteCount; i++) {
              if (i != 0)
                value += " ";
              value += fmt::format("0x{:02X}", ram[item.base + i]);
            }
            break;

          case 'c':
          case 'C':
            for (uint32_t i = 0; i < byteCount; i++) {
              if (i != 0)
                value += " ";
              value += (char) ram[item.base + i];
            }
            break;
        }

        items.push_back({fstring, value});
      }

      iter++;
    }

    for (const auto &fitem: items) {
      std::regex itemPattern{fitem.format};
      std::smatch match;

      if (std::regex_search(format, match, itemPattern)) {
        format.replace(match[0].first, match[0].second, fitem.value);
      }
    }

    return format;
  }

} // dorito