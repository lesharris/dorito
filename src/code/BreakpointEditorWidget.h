#pragma once

#include "zep.h"

namespace dorito {

  class BreakpointWidget : public Zep::IWidget {
  public:
    BreakpointWidget(Zep::ZepEditor &editor) : m_editor(editor) {}

    Zep::NVec2f GetSize() const override;

    void MouseDown(const Zep::NVec2f &pos, Zep::ZepMouseButton button) override;

    void MouseUp(const Zep::NVec2f &pos, Zep::ZepMouseButton button) override;

    void MouseMove(const Zep::NVec2f &pos) override;

    void Draw(const Zep::ZepBuffer &buffer, const Zep::NVec2f &location) override;

    void DrawInline(const Zep::ZepBuffer &buffer, const Zep::NRectf &location) override;

    void Set(const Zep::NVec4f &value) override;

    const Zep::NVec4f &Get() const override;

  private:
    Zep::ZepEditor &m_editor;
    Zep::NVec4f m_value = Zep::NVec4f(0.0f, 0.0f, 0.0f, 0.0f);
    Zep::NVec4f m_DisabledColor = Zep::NVec4f(0.3f, 0.3f, 0.3f, 1.0f);
    Zep::NVec4f m_EnabledColor = Zep::NVec4f(0.6f, 0.6f, 0.6f, 1.0f);
    bool m_Enabled = false;
  };

} // dorito
