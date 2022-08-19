#pragma once

#include "Widget.h"

namespace dorito {

  class AudioWidget : public Widget {
  public:
    std::string Name() override {
      return "Audio";
    }

    void Draw() override;
  };

} // dorito
