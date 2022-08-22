#include "MemoryEditorWidget.h"

#include "layers/UI.h"

namespace dorito {
  void MemoryEditorWidget::Draw() {
    auto &bus = Bus::Get();

    if (m_Enabled) {
      static MemoryEditor memoryViewer;

      memoryViewer.DrawWindow(ICON_FA_MEMORY " Memory", &bus.GetRam().GetMemory()[0], 1024 * 64);
    }
  }
} // dorito