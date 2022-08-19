#include "SoundEditorWidget.h"

#include "layers/UI.h"

namespace dorito {
  void SoundEditorWidget::Draw() {
    ImGui::Begin("Sound Editor", &UI::ShowSoundEditor);
    ImGui::End();
  }
} // dorito