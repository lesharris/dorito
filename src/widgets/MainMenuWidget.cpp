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
        if (ImGui::MenuItem(ICON_FA_RIGHT_FROM_BRACKET " Quit", nullptr)) {
          EventManager::Dispatcher().trigger<Events::WantQuit>();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {

        if (ImGui::MenuItem(bus.Running() ? ICON_FA_PAUSE " Pause" : ICON_FA_PLAY " Run")) {
          EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({!bus.Running()});
        }

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_VOLUME_XMARK " Mute Dorito", nullptr, &m_DoritoMuted)) {
          EventManager::Dispatcher().enqueue(Events::SetMute(m_DoritoMuted));
        }

        ImGui::Separator();

        if (ImGui::BeginMenu(ICON_FA_GAUGE_HIGH " Speed")) {
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
            EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("Color Editor");
            EnablePalette(8);
          }

          ImGui::EndMenu();
        }

        ImGui::EndMenu();
      }

      auto &app = Dorito::Get();
      UI *ui = (UI *) app.GetLayer("ui").get();
      auto status = ui->GetWidgetStatus();

      if (ImGui::BeginMenu("Development")) {
        if (ImGui::MenuItem(ICON_FA_CODE " Code Editor", nullptr, status["Editor"])) {
          EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("Editor");
        }
        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_PAINTBRUSH " Sprite Editor", nullptr, status["SpriteEditor"])) {
          EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("SpriteEditor");
        }

        if (ImGui::MenuItem(ICON_FA_MUSIC " Sound Editor", nullptr, status["SoundEditor"])) {
          EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("SoundEditor");
        }

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_MAGNIFYING_GLASS " Monitors", nullptr, status["MonitorsWidget"])) {
          EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("MonitorsWidget");
        }

        if (ImGui::MenuItem(ICON_FA_CIRCLE " Breakpoints", nullptr, status["Breakpoints"])) {
          EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("Breakpoints");
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {
        if (ImGui::BeginMenu("CPU")) {
          if (ImGui::MenuItem(ICON_FA_MICROCHIP " Registers", nullptr, status["Registers"])) {
            EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("Registers");
          }
          if (ImGui::MenuItem(ICON_FA_BARS " Disassembly", nullptr, status["Disassembly"])) {
            EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("Disassembly");
          }
          ImGui::EndMenu();
        }

        if (ImGui::MenuItem(ICON_FA_MEMORY " Memory Viewer", nullptr, status["MemoryEditor"])) {
          EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("MemoryEditor");
        }

        if (ImGui::MenuItem(ICON_FA_HEADPHONES " Audio Viewer", nullptr, status["Audio"])) {
          EventManager::Dispatcher().enqueue<Events::UIToggleEnabled>("Audio");
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem(ICON_FA_BOLT " About...", nullptr)) {
          m_About = true;
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    if (m_About) {
      ImGui::OpenPopup("About Dorito...");
      ImGui::SetNextWindowSize({370.0f, 350.0f}, ImGuiCond_Appearing);
    }

    std::string name = fmt::format(ICON_FA_MICROCHIP " Dorito v{}", DORITO_VERSION);
    std::string repo = fmt::format("https://github.com/lesharris/dorito");

    if (ImGui::BeginPopupModal("About Dorito...", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {

      ImGui::PushFont(MonoFont);

      auto &app = Dorito::Get();
      auto &logo = app.GetLogoTexture();

      auto region = ImGui::GetContentRegionAvail();
      auto textSize = ImGui::CalcTextSize(name.c_str());
      auto repoSize = ImGui::CalcTextSize(repo.c_str());

      ImGui::SetCursorPosX((region.x - (logo.width * 0.5f)) * 0.5);

      ImGui::Image(reinterpret_cast<ImTextureID>((uint64_t) logo.id),
                   {static_cast<float>(logo.width) * 0.5f, static_cast<float>(logo.height) * 0.5f},
                   {0, 0}, {1, 1});


      ImGui::Dummy({0.0f, 30.0f});
      ImGui::SetCursorPosX((region.x - textSize.x) * 0.5);
      ImGui::Text("%s", name.c_str());
      ImGui::Dummy({0.0f, 10.0f});
      ImGui::SetCursorPosX((region.x - repoSize.x) * 0.5);
      ImGui::Text("%s", repo.c_str());
      ImGui::Dummy({0.0f, 10.0f});
      ImGui::Separator();
      ImGui::Dummy({0.0f, 10.0f});

      ImGui::SetCursorPosX((region.x - 120.0f) * 0.5);

      ImGui::SetItemDefaultFocus();
      if (ImGui::Button(ICON_FA_THUMBS_UP " Sweet!", ImVec2(120.0f, 0))) {
        ImGui::CloseCurrentPopup();
        m_About = false;
      }

      ImGui::Dummy({0.0f, 10.0f});

      ImGui::PopFont();
      ImGui::EndPopup();
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