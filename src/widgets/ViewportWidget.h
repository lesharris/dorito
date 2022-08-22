#pragma once

#include "Widget.h"

namespace dorito {

  class ViewportWidget : public Widget {
  public:
    std::string Name() override {
      return "Viewport";
    }

    void Draw() override;

    bool Enabled() const override {
      // Viewport Always Enabled
      return true;
    }

    void Enabled(bool) override {}

  private:
    ImVec2 GetCenteredPositionForViewport(ImVec2 &aspectSize);

    ImVec2 GetLargestSizeForViewport();

  private:
    ImVec2 m_PreviousWindowSize{0, 0};
    ImVec2 m_PreviousMousePosition{0, 0};

    int32_t m_PrevScreenWidth;
    int32_t m_PrevScreenHeight;

    bool m_Enabled = true;
  };

} // dorito

