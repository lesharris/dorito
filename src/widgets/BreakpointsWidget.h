#pragma once

#include "Widget.h"

namespace dorito {

  class BreakpointsWidget : public Widget {
  public:
    std::string Name() override {
      return "Breakpoints";
    }

    void Draw() override;
  };

} // dorito

