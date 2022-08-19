#pragma once

#include <optional>

#include "zep/mode_repl.h"
#include "zep/editor.h"
#include "zep/filesystem.h"
#include "zep/imgui/display_imgui.h"
#include "zep/mode_standard.h"
#include "zep/mode_vim.h"
#include "zep/tab_window.h"
#include "zep/theme.h"
#include "zep/window.h"

namespace dorito {
  class CodeEditor final : public Zep::IZepComponent {
  public:
    CodeEditor(const std::string &name);

    virtual ~CodeEditor() {};

    void Destroy() {
      m_editor->UnRegisterCallback(this);
      m_editor.reset();
    }

    void draw();

    virtual Zep::ZepEditor &GetEditor() const override final { return *m_editor; }

    void setText(const std::string &str) {
      auto buffer = m_editor->GetMRUBuffer();
      buffer->SetText(str);
    }

    std::string getText() {
      auto buffer = m_editor->GetMRUBuffer();
      return buffer->GetBufferText(buffer->Begin(), buffer->End());
    }

    bool hasTextChanged() {
      auto currentTime = m_editor->GetMRUBuffer()->GetLastUpdateTime();
      if (m_lastUpdateTime.has_value() && (m_lastUpdateTime.value() == currentTime)) {
        return false;
      }
      m_lastUpdateTime = currentTime;
      return true;
    }

  private:
    std::unique_ptr<Zep::ZepEditor> m_editor;
    std::optional<decltype(m_editor->GetMRUBuffer()->GetLastUpdateTime())> m_lastUpdateTime;
    std::optional<float> m_dpiScale;

    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
  };
}
