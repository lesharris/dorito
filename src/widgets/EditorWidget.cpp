#include "EditorWidget.h"

#include <nfd.h>

#include "layers/UI.h"

namespace dorito {
  void EditorWidget::Draw() {
    static octo_program *program = nullptr;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Code", &UI::ShowCodeEditor, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", nullptr)) {
          m_Editor.setText("");
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Open...", nullptr)) {
          nfdchar_t *outPath = nullptr;

          nfdresult_t result = NFD_OpenDialog("o8", nullptr, &outPath);

          switch (result) {
            case NFD_OKAY: {
              std::string path{outPath};
              m_SourceFile = path;
              auto fileText = LoadFileText(outPath);
              std::string file{fileText};
              UnloadFileText(fileText);

              m_Editor.setText(file);
              m_Editor.GetEditor().GetActiveBuffer()->SetFilePath(path);
              m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);

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

        if (ImGui::MenuItem("Save", nullptr)) {
          if (m_SourceFile.empty()) {
            nfdchar_t *outPath = nullptr;

            nfdresult_t result = NFD_SaveDialog("o8", nullptr, &outPath);

            switch (result) {
              case NFD_OKAY: {
                std::string path{outPath};
                m_SourceFile = path;

                SaveFileText(m_SourceFile.c_str(), (char *) m_Editor.getText().c_str());

                m_Editor.GetEditor().GetActiveBuffer()->SetFilePath(path);
                m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);

                delete outPath;
              }
                break;
              case NFD_CANCEL:
                break;
              case NFD_ERROR:
                spdlog::get("console")->error("{}", NFD_GetError());
                break;
            }

          } else {
            SaveFileText(m_SourceFile.c_str(), (char *) m_Editor.getText().c_str());
            m_Editor.GetEditor().GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);
          }
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Code")) {
        if (ImGui::MenuItem("Run", nullptr)) {
          if (program) {
            octo_free_program(program);
            program = nullptr;
          }

          auto code = m_Editor.getText();
          char *source = (char *) malloc(sizeof(char) * code.size());
          memcpy(source, code.c_str(), code.size());

          program = octo_compile_str(source);

          auto viewport = ImGui::FindWindowByName("Viewport");
          ImGui::FocusWindow(viewport);

          EventManager::Dispatcher().enqueue<Events::RunCode>(program->rom);
        }

        if (ImGui::MenuItem("Compile", nullptr)) {
          if (program) {
            octo_free_program(program);
            program = nullptr;
          }

          auto code = m_Editor.getText();
          char *source = (char *) malloc(sizeof(char) * code.size());
          memcpy(source, code.c_str(), code.size());

          program = octo_compile_str(source);
        }

        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    m_Editor.draw();

    if (program) {
      static MemoryEditor compiledRom;

      compiledRom.DrawWindow("Compiled Rom", program->rom, 1024 * 64);
    }

    ImGui::End();

    ImGui::PopStyleVar();
  }
} // dorito