#include "BreakpointEditorWidget.h"

namespace dorito {
  Zep::NVec2f BreakpointWidget::GetSize() const {
    return Zep::NVec2f(0.0f, 0.0f);
  }

  void BreakpointWidget::MouseDown(const Zep::NVec2f &pos, Zep::ZepMouseButton button) {
    ZEP_UNUSED(pos);

    if (button == Zep::ZepMouseButton::Left) {
      m_Enabled = !m_Enabled;
    }
  }

  void BreakpointWidget::MouseUp(const Zep::NVec2f &pos, Zep::ZepMouseButton button) {
    ZEP_UNUSED(pos);
    ZEP_UNUSED(button);
  }

  void BreakpointWidget::MouseMove(const Zep::NVec2f &pos) {
    ZEP_UNUSED(pos);
  }

  void BreakpointWidget::Draw(const Zep::ZepBuffer &buffer, const Zep::NVec2f &location) {
    ZEP_UNUSED(buffer);
    ZEP_UNUSED(location);
  }

  void BreakpointWidget::DrawInline(const Zep::ZepBuffer &buffer, const Zep::NRectf &location) {
    auto &display = m_editor.GetDisplay();
    display.DrawRectFilled(location, m_Enabled ? m_EnabledColor : m_DisabledColor);
  }

  void BreakpointWidget::Set(const Zep::NVec4f &value) {
    ZEP_UNUSED(value);
  }

  const Zep::NVec4f &BreakpointWidget::Get() const {
    return m_value;
  }
} // dorito