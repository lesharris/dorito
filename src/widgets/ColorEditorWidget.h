#pragma once

#include "Widget.h"

namespace dorito {

  class ColorEditorWidget : public Widget {
  public:
    std::string Name() override {
      return "Color Editor";
    }

    void Draw() override;
  };

} // dorito

