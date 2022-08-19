#pragma once

#include "Widget.h"

namespace dorito {

  class SoundEditorWidget : public Widget {
  public:
    std::string Name() override {
      return "SoundEditor";
    }

    void Draw() override;
  };

} // dorito
