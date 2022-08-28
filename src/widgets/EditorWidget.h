#pragma once

#include <string>

#include "Widget.h"
#include "external/zep/ZepEditor.h"

namespace dorito {

  class EditorWidget : public Widget {
    enum class PromptSaveAction {
      None,
      Close
    };

  public:
    EditorWidget();

    virtual ~EditorWidget();

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

    void HighlightCurrent();

    void CheckLineTarget();

    bool HasActiveBuffer();

    void CloseTabWindow();

  private:
    void HandleStepCPU(const Events::StepCPU &event);

  private:
    CodeEditor m_Editor;

    bool m_PromptSave = false;
    PromptSaveAction m_PromptSaveAction = PromptSaveAction::None;

    octo_program *m_Program = nullptr;
    bool m_CompiledSuccessfully = false;
    bool m_Stepped = false;

    uint16_t m_LineTarget = 0;
    bool m_LineTargetEnabled = false;

  };

} // dorito
