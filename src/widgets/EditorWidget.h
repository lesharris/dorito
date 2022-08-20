#pragma once

#include <string>

#include "Widget.h"
#include "external/zep/ZepEditor.h"

namespace dorito {

  class EditorWidget : public Widget {
  public:
    void Draw() override;

    std::string Name() override { return "Editor"; };

  private:
    bool OpenFile(const std::string &path = "");

    bool SaveFile();

    void NewFile();

    bool SaveRom();

    bool Compile();

    void ConfirmSave();

    void DeleteProgram();

  private:
    CodeEditor m_Editor{"Code.o8"};
    Zep::ZepPath m_Path;
    octo_program *m_Program = nullptr;
    bool m_CompiledSuccessfully = false;
    bool m_PromptSave = false;
    bool m_PromptSaveNew = false;
  };

} // dorito
