#pragma once

#include "Widget.h"

namespace dorito {

  class DisassemblyWidget : public Widget {
  public:
    std::string Name() override {
      return "Disassembly";
    }

    void Draw() override;
  };

} // dorito

