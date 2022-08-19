#pragma once

#include "Widget.h"

namespace dorito {

  class SpriteEditorWidget : public Widget {
  public:
    std::string Name() override {
      return "SpriteEditor";
    }

    void Draw() override;
  };

} // dorito
