#pragma once

#include "Widget.h"

namespace dorito {

  class MemoryEditorWidget : public Widget {
  public:
    std::string Name() override {
      return "MemoryEditor";
    }

    void Draw() override;
  };

} // dorito

