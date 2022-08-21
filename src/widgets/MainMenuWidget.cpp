#include "MainMenuWidget.h"

#include <nfd.h>

#include "layers/UI.h"

namespace dorito {
  void MainMenuWidget::Draw() {
    UpdateStatus();
    DrawMenubar();
  }

  void MainMenuWidget::UpdateStatus() {
    auto &bus = Bus::Get();

    auto cycles = bus.CyclesPerFrame();

    uint8_t index = 0;
    for (const auto &val: m_CycleSet) {
      if (m_CycleEntries[index].cycles == 0) {
        index++;
      }

      m_CycleEntries[index++].set = (cycles == val);
    }

    auto quirks = bus.Quirks();

    for (const auto &entry: m_QuirkEntries) {
      uint8_t quirkIndex = static_cast<uint8_t>(entry.quirk);
      SetQuirkEntry(quirkIndex, quirks[quirkIndex]);
    }

    auto profile = bus.GetCompatProfile();

    m_ProfileEntries[0].set = profile == Bus::CompatProfile::VIP;
    m_ProfileEntries[1].set = profile == Bus::CompatProfile::SCHIP;
    m_ProfileEntries[2].set = profile == Bus::CompatProfile::XOChip;

    m_DoritoMuted = bus.Muted();
  }

  void MainMenuWidget::DrawMenubar() {
    auto &bus = Bus::Get();

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open ROM...", nullptr)) {
          nfdchar_t *outPath = nullptr;

          EventManager::Dispatcher().trigger<Events::Reset>({});

          nfdresult_t result = NFD_OpenDialog("ch8,c8", nullptr, &outPath);

          switch (result) {
            case NFD_OKAY: {
              std::string path{outPath};
              EventManager::Dispatcher().trigger<Events::UIResetPC>();
              EventManager::Dispatcher().trigger(Events::LoadROM{path});
              delete outPath;
            }
              break;
            case NFD_CANCEL:
              break;
            case NFD_ERROR:
              spdlog::get("console")->error("{}", NFD_GetError());
              break;
          }
        }

        auto &paths = bus.RecentRoms();

        if (ImGui::BeginMenu("Open Recent...", paths.size() != 0)) {
          for (auto it = paths.rbegin(); it != paths.rend(); it++) {
            auto romPath = *it;

            Zep::ZepPath path{romPath};

            auto name = fmt::format("{}{}", path.stem().c_str(), path.extension().c_str());

            if (ImGui::MenuItem(name.c_str(), nullptr)) {
              EventManager::Dispatcher().enqueue<Events::UIResetPC>();
              EventManager::Dispatcher().enqueue<Events::LoadROM>({path});
            }
          }

          ImGui::Separator();

          if (ImGui::MenuItem("Clear Recents")) {
            EventManager::Dispatcher().trigger<Events::UIClearRecents>();
          }


          ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Unload ROM")) {
          EventManager::Dispatcher().trigger<Events::UIResetPC>();
          EventManager::Dispatcher().trigger(Events::UnloadROM{});
        }

        if (ImGui::MenuItem("Reset")) {
          EventManager::Dispatcher().trigger<Events::UIResetPC>();
          EventManager::Dispatcher().trigger(Events::Reset{});
        }

        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Quit", nullptr)) {
          EventManager::Dispatcher().trigger<Events::WantQuit>();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem(ICON_FA_VOLUME_MUTE " Mute Dorito", nullptr, &m_DoritoMuted)) {
          EventManager::Dispatcher().enqueue(Events::SetMute(m_DoritoMuted));
        }

        ImGui::Separator();

        if (ImGui::BeginMenu(ICON_FA_TACHOMETER_ALT " Speed")) {
          for (auto &entry: m_CycleEntries) {
            if (entry.cycles == 0) {
              ImGui::Separator();
              continue;
            }

            if (ImGui::MenuItem(entry.label.c_str(), nullptr, &(entry.set))) {
              EventManager::Dispatcher().enqueue(Events::SetCycles(entry.cycles));
              EventManager::Dispatcher().enqueue(Events::SavePrefs());
            }
          }
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_LIST " Compatibility")) {
          for (auto &entry: m_ProfileEntries) {
            if (ImGui::MenuItem(entry.label.c_str(), nullptr, &(entry.set))) {

              switch (entry.profile) {
                case Bus::CompatProfile::VIP:
                  EventManager::Dispatcher().enqueue<Events::VIPCompat>({});
                  break;
                case Bus::CompatProfile::SCHIP:
                  EventManager::Dispatcher().enqueue<Events::SCHIPCompat>({});
                  break;
                case Bus::CompatProfile::XOChip:
                  EventManager::Dispatcher().enqueue<Events::XOCompat>({});
                  break;
              }

              EventManager::Dispatcher().enqueue(Events::SavePrefs());
            }
          }

          ImGui::Separator();

          if (ImGui::BeginMenu("Quirks")) {
            for (auto &entry: m_QuirkEntries) {
              if (ImGui::MenuItem(entry.label.c_str(), nullptr, &(entry.set))) {
                EventManager::Dispatcher().enqueue<Events::SetQuirk>({entry.quirk, entry.set});
                EventManager::Dispatcher().enqueue(Events::SavePrefs());
              }
            }

            ImGui::EndMenu();
          }
          ImGui::EndMenu();
        }

        auto drawPalette = [](const std::vector<Color> &pal) {
          float sz = ImGui::GetTextLineHeight();
          ImVec2 p = ImGui::GetCursorScreenPos();

          for (auto i = 1; i <= 4; i++) {
            ImGui::GetWindowDrawList()->AddRectFilled(p,
                                                      ImVec2(p.x + sz, p.y + sz),
                                                      IM_COL32(
                                                          pal[i - 1].r,
                                                          pal[i - 1].g,
                                                          pal[i - 1].b,
                                                          pal[i - 1].a
                                                      ));

            p.x += sz + 4.0f;
          }
          ImGui::Dummy(ImVec2(sz * 5, sz));
          ImGui::SameLine();
        };

        auto &currentPalette = bus.GetDisplay().Palette();

        if (ImGui::BeginMenu(ICON_FA_PALETTE " Colors")) {
          uint8_t index = 0;

          for (const auto &entry: m_PaletteEntries) {
            drawPalette(entry.palette);
            if (ImGui::MenuItem(entry.name.c_str(), nullptr, m_PaletteState[index++])) {
              EnablePalette(index - 1);
              EventManager::Dispatcher().trigger<Events::SetPalette>(Events::SetPalette{entry.palette});
              EventManager::Dispatcher().enqueue(Events::SavePrefs());
            }
          }

          ImGui::Separator();

          drawPalette(currentPalette);
          if (ImGui::MenuItem("Set Colors...", nullptr, m_PaletteState[8])) {
            UI::ShowColorEditor = true;
            EnablePalette(8);
          }

          ImGui::EndMenu();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Development")) {
        ImGui::MenuItem(ICON_FA_CODE " Code Editor", nullptr, &UI::ShowCodeEditor);
        ImGui::Separator();
        ImGui::MenuItem(ICON_FA_PAINT_BRUSH " Sprite Editor", nullptr, &UI::ShowSpriteEditor);
        ImGui::MenuItem(ICON_FA_MUSIC " Sound Editor", nullptr, &UI::ShowSoundEditor);
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {
        if (ImGui::BeginMenu("CPU")) {
          ImGui::MenuItem(ICON_FA_MICROCHIP " Registers", nullptr, &UI::ShowRegisters);
          ImGui::MenuItem(ICON_FA_BARS " Disassembly", nullptr, &UI::ShowDisassembly);
          ImGui::EndMenu();
        }
        ImGui::MenuItem(ICON_FA_MEMORY " Memory Viewer", nullptr, &UI::ShowMemory);
        ImGui::MenuItem(ICON_FA_HEADPHONES " Audio Viewer", nullptr, &UI::ShowAudio);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, &UI::ShowDemo);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
  }

  void MainMenuWidget::SetQuirkEntry(uint8_t quirkIndex, bool isSet) {
    auto quirk = static_cast<Chip8::Quirk>(quirkIndex);
    auto it = std::find_if(
        m_QuirkEntries.begin(),
        m_QuirkEntries.end(),
        [quirk](const QuirkEntry &entry) {
          return quirk == entry.quirk;
        });

    if (it != std::end(m_QuirkEntries)) {
      it->set = isSet;
    }
  }

  void MainMenuWidget::EnablePalette(uint8_t pal) {
    memset(m_PaletteState, 0, 9);
    m_PaletteState[pal] = true;
  }
} // dorito