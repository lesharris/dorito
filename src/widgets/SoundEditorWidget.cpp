#include "SoundEditorWidget.h"

#include "layers/UI.h"

namespace dorito {
  void SoundEditorWidget::Draw() {
    if (!ImGui::Begin(ICON_FA_MUSIC " Sound Editor", &UI::ShowSoundEditor)) {
      ImGui::End();
    } else {
      ImGui::End();
    }
  }
} // dorito