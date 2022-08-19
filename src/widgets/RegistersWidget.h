#pragma once

#include "Widget.h"

namespace dorito {

  class RegistersWidget : public Widget {
  public:
    std::string Name() override {
      return "Registers";
    }

    void Draw() override;
  };

} // dorito
