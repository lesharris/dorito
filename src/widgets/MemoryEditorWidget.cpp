#include "MemoryEditorWidget.h"

#include "layers/UI.h"

namespace dorito {
  void MemoryEditorWidget::Draw() {
    auto &bus = Bus::Get();

    static MemoryEditor memoryViewer;

    memoryViewer.DrawWindow("Memory", &bus.GetRam().GetMemory()[0], 1024 * 64);
  }
} // dorito