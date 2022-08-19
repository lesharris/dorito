#include "SpriteEditorWidget.h"

#include "layers/UI.h"

namespace dorito {
  void SpriteEditorWidget::Draw() {
    ImGui::Begin("Sprite Editor", &UI::ShowSpriteEditor);
    ImGui::End();
  }
} // dorito