#include "EditorWidget.h"

#include <fstream>
#include <nfd.h>

#include "layers/UI.h"

namespace dorito {
  void EditorWidget::Draw() {
    auto &bus = Bus::Get();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Code", &UI::ShowCodeEditor, ImGuiWindowFlags_MenuBar);

    ImGui::PopStyleVar();
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", nullptr)) {
          NewFile();
        }

        ImGui::Separator();

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

        if (ImGui::MenuItem("Save", nullptr)) {
          SaveFile();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Code")) {
        if (ImGui::MenuItem("Run", nullptr)) {
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

        if (ImGui::MenuItem("Compile", nullptr)) {
          if (SaveFile()) {
            if (Compile()) {
              SaveRom();
            }
          }
        }

        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    m_Editor.draw();

    if (m_PromptSave) {
      ImGui::OpenPopup("Save current file?");
    }

    ConfirmSave();

    if (m_Program && m_CompiledSuccessfully) {
      static MemoryEditor compiledRom;

      compiledRom.DrawWindow("Compiled Rom", m_Program->rom, 1024 * 64);
    }

    ImGui::End();
  }

  void EditorWidget::NewFile() {
    auto &editor = m_Editor.GetEditor();

    auto dirty = editor.GetActiveBuffer()->HasFileFlags(Zep::FileFlags::Dirty);

    if (dirty && !m_PromptSaveNew) {
      m_PromptSave = true;
      m_PromptSaveNew = true;
    } else {
      editor.RemoveBuffer(editor.GetActiveBuffer());
      m_Editor.GetEditor().InitWithText("Code.o8", "");
      m_Editor.GetEditor().GetActiveBuffer()->Clear();
      m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);

      if (m_Program) {
        octo_free_program(m_Program);
        m_Program = nullptr;
      }

      m_Path = "";
      m_PromptSave = false;
      m_PromptSaveNew = false;
    }
  }

  bool EditorWidget::OpenFile(const std::string &path) {
    auto doOpen = [&](const std::string &filepath) {
      m_Path = Zep::ZepPath{filepath};
      m_Editor.GetEditor().GetActiveBuffer()->Load(m_Path);
    };

    auto dirty = m_Editor.GetEditor().GetActiveBuffer()->HasFileFlags(Zep::FileFlags::Dirty);

    if (!m_PromptSave) {
      if (dirty && m_Editor.getText().size() > 0) {
        if (m_Path.empty()) {
          m_PromptSave = true;
          m_PromptSaveNew = false;
          return false;
        } else {
          SaveFile();
        }
      }
    } else {
      m_PromptSave = false;
      m_PromptSaveNew = false;
    }

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

          EventManager::Dispatcher().enqueue<Events::UIAddRecentSourceFile>(m_Path);
        }
          return true;

        case NFD_CANCEL:
          return false;

        case NFD_ERROR:
          spdlog::get("console")->error("{}", NFD_GetError());
          return false;
      }
    }

    return false;
  }

  bool EditorWidget::SaveFile() {

    auto doSave = [&](const char *outPath) {
      m_Path = Zep::ZepPath{outPath};
      auto &editor = m_Editor.GetEditor();
      auto buffer = editor.GetActiveBuffer();
      auto &fs = editor.GetFileSystem();
      auto workDir = m_Editor.GetEditor().GetFileSystem().GetWorkingDirectory();
      int64_t size;

      fs.SetWorkingDirectory(m_Path.parent_path());
      buffer->SetFilePath(m_Path.filename());
      buffer->Save(size);
      fs.SetWorkingDirectory(workDir);
    };

    if (m_Path.empty()) {
      nfdchar_t *outPath = nullptr;

      nfdresult_t result = NFD_SaveDialog("o8", nullptr, &outPath);

      switch (result) {
        case NFD_OKAY: {
          doSave(outPath);
          EventManager::Dispatcher().enqueue<Events::UIAddRecentSourceFile>(m_Path);
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
      doSave(m_Path.c_str());
      return true;
    }

    return false;
  }

  bool EditorWidget::SaveRom() {
    if (m_Path.empty() || !m_Program) {
      return false;
    }

    auto romPath = fmt::format("{}/{}.ch8", m_Path.parent_path().c_str(), m_Path.stem().c_str());

    std::ofstream stream(romPath.c_str(), std::ios::binary);

    if (!stream.good()) {
      return false;
    }

    stream.write(&m_Program->rom[0x200], m_Program->length);
    stream.close();

    return true;
  }

  bool EditorWidget::Compile() {
    if (m_Program) {
      octo_free_program(m_Program);
      m_Program = nullptr;
    }

    auto code = m_Editor.getText();
    m_Editor.GetEditor().GetActiveBuffer()->ClearRangeMarkers(Zep::RangeMarkerType::All);

    spdlog::get("console")->info("{}", code);
    // Freed in octo_free_program
    char *source = (char *) malloc(sizeof(char) * code.size());

    memcpy(source, code.c_str(), code.size());

    m_Program = octo_compile_str(source);

    if (m_Program->is_error) {
      auto &editor = m_Editor.GetEditor();
      auto buffer = editor.GetActiveBuffer();
      auto window = editor.GetActiveWindow();

      auto marker = std::make_shared<Zep::RangeMarker>(*m_Editor.GetEditor().GetActiveBuffer());

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

    m_CompiledSuccessfully = true;
    return true;
  }

  void EditorWidget::ConfirmSave() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save current file?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Do you want to save the current file?");
      ImGui::Separator();

      if (ImGui::Button("Yes", ImVec2(120, 0))) {
        SaveFile();

        if (!m_PromptSaveNew) {
          OpenFile();
        } else {
          NewFile();
        }

        ImGui::CloseCurrentPopup();
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();

      if (ImGui::Button("No", ImVec2(120, 0))) {
        if (!m_PromptSaveNew) {
          OpenFile();
        } else {
          NewFile();
        }

        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }

  }
} // dorito