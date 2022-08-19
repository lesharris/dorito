#pragma once

#include "Widget.h"

namespace dorito {

  class MemoryEditorWidget : public Widget {
  public:
    std::string Name() override {
      return Widget::Name();
    }

    void Draw() override;
  };

} // dorito

