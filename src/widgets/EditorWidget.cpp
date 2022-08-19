#include "EditorWidget.h"

#include <fstream>
#include <nfd.h>

#include "layers/UI.h"

namespace dorito {
  void EditorWidget::Draw() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Code", &UI::ShowCodeEditor, ImGuiWindowFlags_MenuBar);

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

        if (ImGui::MenuItem("Save", nullptr)) {
          SaveFile();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Code")) {
        if (ImGui::MenuItem("Run", nullptr)) {
          if (SaveFile()) {
            Compile();
            SaveRom();

            auto viewport = ImGui::FindWindowByName("Viewport");
            ImGui::FocusWindow(viewport);

            EventManager::Dispatcher().enqueue<Events::RunCode>(m_Program->rom);
          }
        }

        if (ImGui::MenuItem("Compile", nullptr)) {
          if (SaveFile()) {
            Compile();
            SaveRom();
          }
        }

        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    m_Editor.draw();

    if (m_Program) {
      static MemoryEditor compiledRom;

      compiledRom.DrawWindow("Compiled Rom", m_Program->rom, 1024 * 64);
    }

    ImGui::End();

    ImGui::PopStyleVar();
  }

  bool EditorWidget::OpenFile() {
    nfdchar_t *outPath = nullptr;

    nfdresult_t result = NFD_OpenDialog("o8", nullptr, &outPath);

    switch (result) {
      case NFD_OKAY: {
        m_Path = Zep::ZepPath{outPath};
        auto fileText = LoadFileText(outPath);
        std::string file{fileText};
        UnloadFileText(fileText);

        m_Editor.setText(file);
        m_Editor.GetEditor().GetActiveBuffer()->SetFilePath(m_Path);
        m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);

        delete outPath;
      }
        return true;

      case NFD_CANCEL:
        return false;

      case NFD_ERROR:
        spdlog::get("console")->error("{}", NFD_GetError());
        return false;
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

  void EditorWidget::Compile() {
    if (m_Program) {
      octo_free_program(m_Program);
      m_Program = nullptr;
    }

    auto code = m_Editor.getText();

    // Freed in octo_free_program
    char *source = (char *) malloc(sizeof(char) * code.size());

    memcpy(source, code.c_str(), code.size());

    m_Program = octo_compile_str(source);
  }
} // dorito