#include "EditorWidget.h"

#include <fstream>
#include <nfd.h>

#include "layers/UI.h"

#include "code/ZepImGuiExCommand.h"
#include "code/BreakpointEditorWidget.h"

#include "external/imgui_colored_button.h"

namespace dorito {
  EditorWidget::EditorWidget() {
    EventManager::Get().Attach<
        Events::StepCPU,
        &EditorWidget::HandleStepCPU
    >(this);

    m_Editor.BuildFonts();

    auto &editor = m_Editor.GetEditor();
    editor.RegisterExCommand(std::make_shared<ZepImGuiExCommand>(editor));
  }

  EditorWidget::~EditorWidget() {
    EventManager::Get().DetachAll(this);
  }

  void EditorWidget::Draw() {
    auto &bus = Bus::Get();

    bool wasEnabled = m_Enabled;

    ImGui::SetNextWindowSize({400, 350}, ImGuiCond_FirstUseEver);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (!ImGui::Begin(ICON_FA_CODE " Code", &m_Enabled, ImGuiWindowFlags_MenuBar)) {
      ImGui::PopStyleVar();
      ImGui::End();
    } else {

      ImGui::PopStyleVar();
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem(ICON_FA_FILE " New", nullptr)) {
            NewFile();
          }

          if (ImGui::MenuItem("Open...", nullptr)) {
            OpenFile();
          }

          auto &paths = bus.RecentSourceFiles();

          if (ImGui::BeginMenu("Open Recent...", paths.size() != 0)) {

            for (const auto &path: paths) {
              auto zpath = Zep::ZepPath{path};
              auto filepath = fmt::format("{}{}", zpath.stem().c_str(), zpath.extension().c_str());

              if (ImGui::MenuItem(filepath.c_str(), nullptr)) {
                OpenFile(path);
              }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Clear Recents", nullptr)) {
              EventManager::Dispatcher().enqueue<Events::UIClearRecentSources>();
            }

            ImGui::EndMenu();
          }

          ImGui::Separator();

          if (ImGui::MenuItem("Close", nullptr, false, HasActiveBuffer())) {
            auto &editor = m_Editor.GetEditor();
            auto dirty = editor.GetActiveBuffer()->HasFileFlags(Zep::FileFlags::Dirty);

            if (dirty) {
              m_PromptSave = true;
              m_PromptSaveAction = PromptSaveAction::Close;
            } else {
              CloseTabWindow();
            }
          }

          if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Save", nullptr, false, HasActiveBuffer())) {
            SaveFile();
          }

          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Code")) {
          if (ImGui::MenuItem(ICON_FA_PLAY " Run", nullptr, false, HasActiveBuffer())) {
            if (SaveFile()) {
              if (Compile()) {
                SaveRom();
                auto viewport = ImGui::FindWindowByName("Viewport");
                ImGui::FocusWindow(viewport);
                EventManager::Dispatcher().enqueue<Events::RunCode>(m_Program->rom);
              }
            }
          }

          ImGui::Separator();

          if (ImGui::MenuItem(ICON_FA_GEAR " Compile", nullptr, false, HasActiveBuffer())) {
            if (SaveFile()) {
              if (Compile()) {
                EventManager::Dispatcher().enqueue<Events::LoadCode>(m_Program->rom);
                SaveRom();
              }
            }
          }

          ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
      }

      if (HasActiveBuffer()) {
        ImGui::Dummy({0.0f, 10.0f});
        ImGui::Dummy({10.0f, 0.0});
        ImGui::SameLine();

        if (ImGui::ColoredButtonV1(bus.Running() ? ICON_FA_PAUSE " Pause " : ICON_FA_PLAY " Run ", ImVec2(0.0f, 0.0f),
                                   IM_COL32(255, 255, 255, 255),
                                   bus.Running() ? IM_COL32(200, 60, 60, 255) : IM_COL32(50, 220, 60, 255),
                                   bus.Running() ? IM_COL32(180, 40, 90, 255) : IM_COL32(69, 150, 70, 255))) {

          if (!bus.Running()) {
            if (SaveFile()) {
              if (Compile()) {
                SaveRom();
                auto viewport = ImGui::FindWindowByName("Viewport");
                ImGui::FocusWindow(viewport);
                EventManager::Dispatcher().enqueue<Events::RunCode>(m_Program->rom);
                EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({!bus.Running()});
              }
            }
          } else {
            EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({!bus.Running()});
          }
        }

        ImGui::SameLine();

        if (!m_CompiledSuccessfully) {
          ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
          ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (!bus.Running()) {
          if (ImGui::Button(ICON_FA_FORWARD_STEP " Step ")) {
            EventManager::Dispatcher().enqueue<Events::StepCPU>({});
          }
        }

        if (!m_CompiledSuccessfully) {
          ImGui::PopItemFlag();
          ImGui::PopStyleVar();
        }

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        if (!m_CompiledSuccessfully) {
          ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
          ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        if (ImGui::Button(ICON_FA_BACKWARD_STEP " Reset ")) {
          m_Stepped = false;
          m_LineTarget = 0;
          m_Editor.GetEditor().GetActiveBuffer()->ClearRangeMarkers(Zep::RangeMarkerType::All);
          EventManager::Dispatcher().enqueue<Events::Reset>();
        }

        if (!m_CompiledSuccessfully) {
          ImGui::PopItemFlag();
          ImGui::PopStyleVar();
        }

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_GEAR " Compile ")) {
          if (SaveFile()) {
            if (Compile()) {
              m_Stepped = false;
              m_LineTarget = 0;
              EventManager::Dispatcher().enqueue<Events::LoadCode>(m_Program->rom);
              SaveRom();
            }
          }
        }

        //ImGui::SameLine();
        //ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        //ImGui::SameLine();

        /*  ImGui::Text("Run to line: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(50.0f);
            ImGui::InputScalar("##", ImGuiDataType_U16, &m_LineTarget, nullptr, nullptr, "%d");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(" Go ")) {
              if (m_LineTarget != 0) {
                m_LineTargetEnabled = true;
                EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({true});
              }
            }*/

        ImGui::Dummy({0.0f, 10.0f});

        HighlightCurrent();
        CheckLineTarget();
      }

      m_Editor.Draw();

      if (m_PromptSave) {
        ImGui::OpenPopup("Save current file?");
      }

      ConfirmSave();

      if (!m_Enabled && wasEnabled) {
        EventManager::Dispatcher().enqueue<Events::SaveAppPrefs>();
      }

      ImGui::End();
    }
  }

  void EditorWidget::NewFile() {
    auto &editor = m_Editor.GetEditor();
    auto tabWindow = editor.AddTabWindow();

    editor.SetCurrentTabWindow(tabWindow);
  }

  bool EditorWidget::OpenFile(const std::string &path) {
    auto doOpen = [&](const std::string &filepath) {
      auto &editor = m_Editor.GetEditor();
      auto tabWindow = editor.AddTabWindow();
      editor.SetCurrentTabWindow(tabWindow);
      auto buffer = editor.GetActiveBuffer();
      buffer->Load(Zep::ZepPath{filepath});
    };

    if (path.size() != 0) {
      doOpen(path);
    } else {
      nfdchar_t *outPath = nullptr;
      nfdresult_t result = NFD_OpenDialog("o8", nullptr, &outPath);

      switch (result) {
        case NFD_OKAY: {

          std::string filepath{outPath};
          doOpen(filepath);

          delete outPath;

          EventManager::Dispatcher().enqueue<Events::UIAddRecentSourceFile>(filepath);
        }
          return true;

        case NFD_CANCEL:
          return false;

        case NFD_ERROR:
          spdlog::get("console")->error("{}", NFD_GetError());
          return false;
      }
    }

    auto &editor = m_Editor.GetEditor();
    auto buffer = editor.GetActiveBuffer();

    auto lines = buffer->GetLineCount();

    for (auto i = 0l; i < lines; i++) {
      Zep::ByteRange range;
      buffer->GetLineOffsets(i, range);

      auto picker = std::make_shared<BreakpointWidget>(editor);
      auto marker = std::make_shared<Zep::RangeMarker>(*buffer);

      marker->SetRange({range.first, range.first + 1});
      marker->markerType = Zep::RangeMarkerType::Widget;
      marker->displayType = Zep::RangeMarkerDisplayType::Background;
      marker->spWidget = picker;

      buffer->AddRangeMarker(marker);

    }

    return false;
  }

  bool EditorWidget::SaveFile() {

    auto doSave = [&](const char *outPath) {
      Zep::ZepPath path = outPath;

      auto &editor = m_Editor.GetEditor();
      auto buffer = editor.GetActiveBuffer();
      auto &fs = editor.GetFileSystem();
      auto workDir = editor.GetFileSystem().GetWorkingDirectory();
      int64_t size;

      if (!buffer) {
        return;
      }

      fs.SetWorkingDirectory(path.parent_path());
      buffer->SetFilePath(path.filename());
      buffer->Save(size);
      fs.SetWorkingDirectory(workDir);
    };

    auto &editor = m_Editor.GetEditor();
    auto buffer = editor.GetActiveBuffer();
    auto path = buffer->GetFilePath();

    if (path.empty()) {
      nfdchar_t *outPath = nullptr;

      nfdresult_t result = NFD_SaveDialog("o8", nullptr, &outPath);

      switch (result) {
        case NFD_OKAY: {
          doSave(outPath);
          EventManager::Dispatcher().enqueue<Events::UIAddRecentSourceFile>(outPath);
          delete outPath;

          return true;
        }

        case NFD_CANCEL:
          return false;

        case NFD_ERROR:
          spdlog::get("console")->error("{}", NFD_GetError());
          return false;
      }

    } else {
      doSave(path.c_str());
      return true;
    }

    return false;
  }

  bool EditorWidget::SaveRom() {
    if (!m_Program) {
      return false;
    }

    auto &editor = m_Editor.GetEditor();
    auto buffer = editor.GetActiveBuffer();
    auto path = buffer->GetFilePath();

    auto romPath = fmt::format("{}/{}.ch8", path.parent_path().c_str(), path.stem().c_str());

    std::ofstream stream(romPath.c_str(), std::ios::binary);

    if (!stream.good()) {
      return false;
    }

    stream.write(&m_Program->rom[0x200], m_Program->length);
    stream.close();

    return true;
  }

  bool EditorWidget::Compile() {
    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();

    auto &editor = m_Editor.GetEditor();
    auto buffer = editor.GetActiveBuffer();
    auto window = editor.GetActiveWindow();

    cpu.ClearBreakpoints();
    DeleteProgram();

    auto code = buffer->GetBufferText(buffer->Begin(), buffer->End());
    buffer->ClearRangeMarkers(Zep::RangeMarkerType::All);

    // Freed in octo_free_program
    char *source = (char *) malloc(sizeof(char) * code.size());
    memcpy(source, code.c_str(), code.size());

    m_Program = octo_compile_str(source);

    if (m_Program->is_error) {
      auto marker = std::make_shared<Zep::RangeMarker>(*buffer);

      Zep::ByteRange range;
      buffer->GetLineOffsets(m_Program->error_line, range);

      marker->SetHighlightColor(Zep::ThemeColor::Error);
      marker->SetEnabled(true);
      marker->SetDescription(m_Program->error);
      marker->SetName("Compilation Error");
      marker->SetRange({range.first + m_Program->error_pos, range.first + m_Program->error_pos + 1});

      buffer->AddRangeMarker(marker);

      auto pos = Zep::GlyphIterator{buffer, (unsigned long) range.first + m_Program->error_pos + 1};
      window->SetBufferCursor(pos);

      Zep::GlyphRange glyphRange{buffer, range};
      buffer->BeginFlash(1.0f, Zep::FlashType::Flash, glyphRange);

      m_CompiledSuccessfully = false;

      return false;
    }

    if (m_Program->monitors.keys.count > 0) {
      for (auto i = 0; i < m_Program->monitors.keys.count; i++) {
        auto key = (char *) m_Program->monitors.keys.data[i];
        auto monitor = (octo_mon *) m_Program->monitors.values.data[i];

        EventManager::Dispatcher().enqueue<Events::UIAddMonitor>({
                                                                     monitor->type,
                                                                     monitor->base,
                                                                     monitor->len,
                                                                     monitor->format,
                                                                     key
                                                                 });
      }
    }

    for (uint16_t i = 0; i < 1024 * 64 - 1; i++) {
      if (!m_Program->breakpoints[i])
        continue;

      std::string label = m_Program->breakpoints[i];
      cpu.AddBreakpoint({label, i, true});
    }

    m_CompiledSuccessfully = true;

    return true;
  }

  void EditorWidget::ConfirmSave() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    auto ExecutePromptAction = [&]() {
      switch (m_PromptSaveAction) {
        case PromptSaveAction::None:
          break;
        case PromptSaveAction::Close:
          CloseTabWindow();
          break;
      }

      m_PromptSave = false;
      m_PromptSaveAction = PromptSaveAction::None;
    };

    if (ImGui::BeginPopupModal("Save current file?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Do you want to save the current file?");
      ImGui::Separator();

      if (ImGui::Button("Yes", ImVec2(120, 0))) {
        if (SaveFile()) {
          ExecutePromptAction();
          ImGui::CloseCurrentPopup();
        }
      }

      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();

      if (ImGui::Button("No", ImVec2(120, 0))) {
        ExecutePromptAction();
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }
  }

  void EditorWidget::DeleteProgram() {
    if (m_Program) {
      m_CompiledSuccessfully = false;
      EventManager::Dispatcher().trigger<Events::UIClearMonitors>();
      octo_free_program(m_Program);
      m_Program = nullptr;
    }
  }

  void EditorWidget::HighlightCurrent() {
    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();
    auto &editor = m_Editor.GetEditor();
    auto buffer = editor.GetActiveBuffer();
    auto window = editor.GetActiveWindow();

    if (!m_CompiledSuccessfully) {
      return;
    }

    if (cpu.m_Halted && !m_Stepped) {
      return;
    }

    if (!cpu.m_Halted) {
      m_Stepped = false;
    }

    if (m_Program->listing->contains(cpu.regs.pc)) {
      const auto &item = m_Program->listing->at(cpu.regs.pc);

      buffer->ClearRangeMarkers(Zep::RangeMarkerType::All);

      auto marker = std::make_shared<Zep::RangeMarker>(*buffer);
      auto line = item.line - 1;

      // jump
      if ((item.data & 0xF000) == 0x1000) {
        line--;
      }

      // return
      if ((item.data & 0x00FF) == 0x00EE) {
        line--;
      }

      auto &lines = cpu.m_Disassembly;
      Chip8::DisassemblyLine dline;
      bool haveDLine = false;

      if (lines.contains(cpu.regs.pc)) {
        haveDLine = true;
        dline = lines.at(cpu.regs.pc);
      }

      Zep::ByteRange range;
      buffer->GetLineOffsets(line, range);

      marker->SetHighlightColor(Zep::ThemeColor::Info);
      marker->SetEnabled(true);

      if (haveDLine) {
        marker->SetDescription(fmt::format("0x{:04X} | {} | {}", dline.addr, dline.bytes, dline.text));
      } else {
        marker->SetDescription(fmt::format("0x{:04X}", cpu.regs.pc));
      }

      marker->SetRange({range.first, range.second});

      buffer->AddRangeMarker(marker);

      auto pos = Zep::GlyphIterator{buffer, (unsigned long) range.second - 1};
      window->SetBufferCursor(pos);
    }
  }

  void EditorWidget::HandleStepCPU(const Events::StepCPU &) {
    m_Stepped = true;
  }

  void EditorWidget::CheckLineTarget() {
    if (m_LineTarget == 0 || !m_LineTargetEnabled) {
      return;
    }

    if (!m_CompiledSuccessfully) {
      return;
    }

    auto &bus = Bus::Get();
    auto &cpu = bus.GetCpu();

    if (m_Program->listing->contains(cpu.regs.pc)) {
      const auto &item = m_Program->listing->at(cpu.regs.pc);

      if (item.line == m_LineTarget) {
        m_LineTargetEnabled = false;
        EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({false});
      }
    }
  }

  bool EditorWidget::HasActiveBuffer() {
    auto &editor = m_Editor.GetEditor();

    return editor.GetActiveBuffer() != nullptr;
  }

  void EditorWidget::CloseTabWindow() {
    auto &editor = m_Editor.GetEditor();

    if (editor.GetActiveTabWindow()) {
      editor.RemoveTabWindow(editor.GetActiveTabWindow());
    }
  }

} // dorito