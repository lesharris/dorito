#include "ZepEditor.h"

#include <GLFW/glfw3.h>

#include "zep/regress.h"
#include "zep/imgui/display_imgui.h"

#include "code/ZepSyntaxOcto.h"

namespace dorito {
  CodeEditor::CodeEditor(const std::string &name)
      : m_editor(std::make_unique<Zep::ZepEditor>(
      new Zep::ZepDisplay_ImGui(),
      Zep::ZepPath(""),
      0,
      new Zep::ZepFileSystemCPP(""))) {

    m_editor->RegisterCallback(this);

    Zep::ZepRegressExCommand::Register(*m_editor);

    auto &config = m_editor->GetConfig();
    config.autoHideCommandRegion = false;
    config.showNormalModeKeyStrokes = true;

    ZepSyntaxOcto::registerSyntax(m_editor);

    m_editor->InitWithText(name, "");
    m_editor->SetGlobalMode(Zep::ZepMode_Standard::StaticName());
    m_editor->GetActiveBuffer()->Clear();
    m_editor->GetActiveBuffer()->SetFileFlags(Zep::FileFlags::Dirty, false);
  }

  void CodeEditor::draw() {
    auto dpiScale = ImGui::GetWindowDpiScale();

    if (!m_dpiScale.has_value() || (m_dpiScale.value() != dpiScale)) {
      m_dpiScale = dpiScale;
      auto &display = m_editor->GetDisplay();
      display.SetPixelScale(Zep::NVec2f(dpiScale));

      int fontPixelHeight = 18;
      auto font = ImGui::GetFont();

      display.SetFont(Zep::ZepTextType::UI, std::make_shared<Zep::ZepFont_ImGui>(display, font, fontPixelHeight));
      display.SetFont(Zep::ZepTextType::Text, std::make_shared<Zep::ZepFont_ImGui>(display, font, fontPixelHeight));
      display.SetFont(Zep::ZepTextType::Heading1,
                      std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.75)));
      display.SetFont(Zep::ZepTextType::Heading2,
                      std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.5)));
      display.SetFont(Zep::ZepTextType::Heading3,
                      std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.25)));
    }

    auto min = ImGui::GetCursorScreenPos();
    auto max = ImGui::GetContentRegionAvail();
    max.x = std::max(1.0f, max.x);
    max.y = std::max(1.0f, max.y);

    // Fill the window
    max.x = min.x + max.x;
    max.y = min.y + max.y;
    m_editor->SetDisplayRegion(Zep::NVec2f(min.x, min.y), Zep::NVec2f(max.x, max.y));

    // Display the editor inside this window
    m_editor->Display();
    auto &io = ImGui::GetIO();

    if (io.MouseDelta.x != 0 || io.MouseDelta.y != 0) {
      m_editor->OnMouseMove(Zep::toNVec2f(io.MousePos));
    }

    if (io.MouseClicked[0]) {
      if (m_editor->OnMouseDown(Zep::toNVec2f(io.MousePos), Zep::ZepMouseButton::Left)) {
        // Hide the mouse click from imgui if we handled it
        io.MouseClicked[0] = false;
      }
    }

    if (io.MouseClicked[1]) {
      if (m_editor->OnMouseDown(Zep::toNVec2f(io.MousePos), Zep::ZepMouseButton::Right)) {
        // Hide the mouse click from imgui if we handled it
        io.MouseClicked[0] = false;
      }
    }

    if (io.MouseReleased[0]) {
      if (m_editor->OnMouseUp(Zep::toNVec2f(io.MousePos), Zep::ZepMouseButton::Left)) {
        // Hide the mouse click from imgui if we handled it
        io.MouseClicked[0] = false;
      }
    }

    if (io.MouseReleased[1]) {
      if (m_editor->OnMouseUp(Zep::toNVec2f(io.MousePos), Zep::ZepMouseButton::Right)) {
        // Hide the mouse click from imgui if we handled it
        io.MouseClicked[0] = false;
      }
    }

    if (ImGui::IsWindowFocused()) {
      bool handled = false;

      uint32_t mod = 0;

      static std::map<int, int> MapUSBKeys = {
          {GLFW_KEY_F1,  Zep::ExtKeys::F1},
          {GLFW_KEY_F2,  Zep::ExtKeys::F2},
          {GLFW_KEY_F3,  Zep::ExtKeys::F3},
          {GLFW_KEY_F4,  Zep::ExtKeys::F4},
          {GLFW_KEY_F5,  Zep::ExtKeys::F5},
          {GLFW_KEY_F6,  Zep::ExtKeys::F6},
          {GLFW_KEY_F7,  Zep::ExtKeys::F7},
          {GLFW_KEY_F8,  Zep::ExtKeys::F8},
          {GLFW_KEY_F9,  Zep::ExtKeys::F9},
          {GLFW_KEY_F10, Zep::ExtKeys::F10},
          {GLFW_KEY_F11, Zep::ExtKeys::F11},
          {GLFW_KEY_F12, Zep::ExtKeys::F12},
      };

      if (io.KeyCtrl) {
        mod |= Zep::ModifierKey::Ctrl;
      }
      if (io.KeyShift) {
        mod |= Zep::ModifierKey::Shift;
      }
      bool super = io.KeySuper;

      auto pWindow = m_editor->GetActiveTabWindow()->GetActiveWindow();
      const auto &buffer = pWindow->GetBuffer();

      // Check USB Keys
      for (auto &usbKey: MapUSBKeys) {
        if (ImGui::IsKeyPressed(usbKey.first)) {
          buffer.GetMode()->AddKeyPress(usbKey.second, mod);
          return;
        }
      }

      if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::TAB, mod);
        return;
      }
      if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::ESCAPE, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::RETURN, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::DEL, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::HOME, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::END, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::BACKSPACE, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::RIGHT, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::LEFT, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::UP, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::DOWN, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_PageDown))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::PAGEDOWN, mod);
        return;
      } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_PageUp))) {
        buffer.GetMode()->AddKeyPress(Zep::ExtKeys::PAGEUP, mod);
        return;
      } else if (io.KeyCtrl) {
        if (ImGui::IsKeyPressed('1')) {
          m_editor->SetGlobalMode(Zep::ZepMode_Standard::StaticName());
          handled = true;
        } else if (ImGui::IsKeyPressed('2')) {
          m_editor->SetGlobalMode(Zep::ZepMode_Vim::StaticName());
          handled = true;
        } else if (ImGui::IsKeyPressed('V')) {
          auto pos = m_editor->GetActiveBuffer()->GetLastEditLocation();
          Zep::ChangeRecord changeRecord;
          m_editor->GetActiveBuffer()->Insert(pos, ImGui::GetClipboardText(), changeRecord);
          handled = true;
        } else {
          for (int ch = 'A'; ch <= 'Z'; ch++) {
            if (ImGui::IsKeyPressed(ch)) {
              buffer.GetMode()->AddKeyPress(ch - 'A' + 'a', mod);
              handled = true;
            }
          }

          if (ImGui::IsKeyPressed(GLFW_KEY_SPACE)) {
            buffer.GetMode()->AddKeyPress(' ', mod);
            handled = true;
          }
        }
      }

      if (!handled) {
        for (int n = 0; n < io.InputQueueCharacters.Size && io.InputQueueCharacters[n]; n++) {
          // Ignore '\r' - sometimes ImGui generates it!
          if (io.InputQueueCharacters[n] == '\r') continue;

          buffer.GetMode()->AddKeyPress(io.InputQueueCharacters[n], mod);
        }
      }
    } else {
      // Don't blink the cursor if not focused.
      m_editor->ResetCursorTimer();
    }
  }

  void CodeEditor::Notify(std::shared_ptr<Zep::ZepMessage> message) {
    if (message->messageId == Zep::Msg::GetClipBoard) {
      message->str = ImGui::GetClipboardText();
      message->handled = true;
    } else if (message->messageId == Zep::Msg::SetClipBoard) {
      ImGui::SetClipboardText(message->str.c_str());
      message->handled = true;
    } else if (message->messageId == Zep::Msg::RequestQuit) {
      message->handled = true;
    }
  }
}