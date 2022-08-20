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
          m_Editor.setText("");

          if (m_Program) {
            octo_free_program(m_Program);
            m_Program = nullptr;
          }
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

    if (m_Program && m_CompiledSuccessfully) {
      static MemoryEditor compiledRom;

      compiledRom.DrawWindow("Compiled Rom", m_Program->rom, 1024 * 64);
    }

    ImGui::End();
  }

  bool EditorWidget::OpenFile(const std::string &path) {
    auto doOpen = [&](const std::string &filepath) {
      m_Path = Zep::ZepPath{filepath};
      auto fileText = LoadFileText(filepath.c_str());
      std::string file{fileText};
      UnloadFileText(fileText);

      m_Editor.setText(file);
      m_Editor.GetEditor().GetActiveBuffer()->SetFilePath(m_Path);
      m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);
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
    if (m_Path.empty()) {
      nfdchar_t *outPath = nullptr;

      nfdresult_t result = NFD_SaveDialog("o8", nullptr, &outPath);

      switch (result) {
        case NFD_OKAY: {
          m_Path = Zep::ZepPath{outPath};

          SaveFileText(m_Path.c_str(), (char *) m_Editor.getText().c_str());

          m_Editor.GetEditor().GetActiveBuffer()->SetFilePath(m_Path);
          m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);

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
      if (SaveFileText(m_Path.c_str(), (char *) m_Editor.getText().c_str())) {
        m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);
        return true;
      }
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

    // Freed in octo_free_program
    char *source = (char *) malloc(sizeof(char) * code.size());

    memcpy(source, code.c_str(), code.size());

    m_Program = octo_compile_str(source);

    if (m_Program->is_error) {
      auto &editor = m_Editor.GetEditor();
      auto buffer = editor.GetActiveBuffer();
      auto window = editor.GetActiveWindow();

      buffer->ClearRangeMarkers(Zep::RangeMarkerType::All);

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
} // dorito