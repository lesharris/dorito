#include "Input.h"

namespace dorito::Input {
  const std::unordered_map<int, char> Manager::m_PrintableKeys = {
      {KEY_APOSTROPHE,    '\''},
      {KEY_COMMA,         ','},
      {KEY_MINUS,         '-'},
      {KEY_PERIOD,        '.'},
      {KEY_SLASH,         '/'},
      {KEY_ZERO,          '0'},
      {KEY_ONE,           '1'},
      {KEY_TWO,           '2'},
      {KEY_THREE,         '3'},
      {KEY_FOUR,          '4'},
      {KEY_FIVE,          '5'},
      {KEY_SIX,           '6'},
      {KEY_SEVEN,         '7'},
      {KEY_EIGHT,         '8'},
      {KEY_NINE,          '9'},
      {KEY_SEMICOLON,     ';'},
      {KEY_EQUAL,         '='},
      {KEY_A,             'a'},
      {KEY_B,             'b'},
      {KEY_C,             'c'},
      {KEY_D,             'd'},
      {KEY_E,             'e'},
      {KEY_F,             'f'},
      {KEY_G,             'g'},
      {KEY_H,             'h'},
      {KEY_I,             'i'},
      {KEY_J,             'j'},
      {KEY_K,             'k'},
      {KEY_L,             'l'},
      {KEY_M,             'm'},
      {KEY_N,             'n'},
      {KEY_O,             'o'},
      {KEY_P,             'p'},
      {KEY_Q,             'q'},
      {KEY_R,             'r'},
      {KEY_S,             's'},
      {KEY_T,             't'},
      {KEY_U,             'u'},
      {KEY_V,             'v'},
      {KEY_W,             'w'},
      {KEY_X,             'x'},
      {KEY_Y,             'y'},
      {KEY_Z,             'z'},
      {KEY_LEFT_BRACKET,  '['},
      {KEY_BACKSLASH,     '\\'},
      {KEY_RIGHT_BRACKET, ']'},
      {KEY_GRAVE,         '`'},
      {KEY_KP_0,          '0'},
      {KEY_KP_1,          '1'},
      {KEY_KP_2,          '2'},
      {KEY_KP_3,          '3'},
      {KEY_KP_4,          '4'},
      {KEY_KP_5,          '5'},
      {KEY_KP_6,          '6'},
      {KEY_KP_7,          '7'},
      {KEY_KP_8,          '8'},
      {KEY_KP_9,          '9'},
      {KEY_KP_DECIMAL,    '.'},
      {KEY_KP_DIVIDE,     '/'},
      {KEY_KP_MULTIPLY,   '*'},
      {KEY_KP_SUBTRACT,   '-'},
      {KEY_KP_ADD,        '+'},
      {KEY_KP_EQUAL,      '='},
      {KEY_SPACE,         ' '}
  };

  const std::unordered_map<int, char> Manager::m_ShiftedValue = {
      {KEY_APOSTROPHE,    '"'},
      {KEY_COMMA,         '<'},
      {KEY_MINUS,         '_'},
      {KEY_PERIOD,        '>'},
      {KEY_SLASH,         '?'},
      {KEY_ZERO,          ')'},
      {KEY_ONE,           '!'},
      {KEY_TWO,           '@'},
      {KEY_THREE,         '#'},
      {KEY_FOUR,          '$'},
      {KEY_FIVE,          '%'},
      {KEY_SIX,           '^'},
      {KEY_SEVEN,         '&'},
      {KEY_EIGHT,         '*'},
      {KEY_NINE,          '('},
      {KEY_SEMICOLON,     ':'},
      {KEY_A,             'A'},
      {KEY_B,             'B'},
      {KEY_C,             'C'},
      {KEY_D,             'D'},
      {KEY_E,             'E'},
      {KEY_F,             'F'},
      {KEY_G,             'G'},
      {KEY_H,             'H'},
      {KEY_I,             'I'},
      {KEY_J,             'J'},
      {KEY_K,             'K'},
      {KEY_L,             'L'},
      {KEY_M,             'M'},
      {KEY_N,             'N'},
      {KEY_O,             'O'},
      {KEY_P,             'P'},
      {KEY_Q,             'Q'},
      {KEY_R,             'R'},
      {KEY_S,             'S'},
      {KEY_T,             'T'},
      {KEY_U,             'U'},
      {KEY_V,             'V'},
      {KEY_W,             'W'},
      {KEY_X,             'X'},
      {KEY_Y,             'Y'},
      {KEY_Z,             'Z'},
      {KEY_LEFT_BRACKET,  '{'},
      {KEY_BACKSLASH,     '|'},
      {KEY_RIGHT_BRACKET, '}'},
      {KEY_GRAVE,         '~'}
  };

  Manager::Manager() {
    m_Mapping.Set(
        {

        });
  }

  void Manager::Poll() {
    PollKeyboard();
    PollMouse();
  }

  void Manager::PollKeyboard() {
    for (auto key = GetKeyPressed(); key != 0; key = GetKeyPressed()) {
      SetModifierState(key);
      m_KeyQueue.push_back(key);

      EventManager::Dispatcher().enqueue<Events::KeyPressed>(key);
    }

    for (auto key: m_Keys) {
      if (IsKeyReleased(key)) {
        ClearModifierState(key);

        EventManager::Dispatcher().enqueue<Events::KeyReleased>(key);
      }

      if (IsKeyUp(key) && m_KeyState[key]) {
        m_KeyState[key] = false;
        ClearModifierState(key);

        EventManager::Dispatcher().enqueue<Events::KeyUp>(key);
      }

      if (IsKeyDown(key)) {
        m_KeyState[key] = true;
        SetModifierState(key);

        EventManager::Dispatcher().enqueue<Events::KeyDown>(key);
      }
    }

    HandlePressedActions();
  }

  void Manager::PollMouse() {
    auto position = Dorito::Get().MousePosition();
    auto previousPosition = Dorito::Get().PreviousMousePosition();

    for (auto button: m_MouseButtons) {
      if (IsMouseButtonPressed(button)) {

        EventManager::Dispatcher().enqueue<Events::MousePressed>(button, position);
      }

      if (IsMouseButtonReleased(button)) {
        EventManager::Dispatcher().enqueue<Events::MouseReleased>(button, position);
      }

      if (IsMouseButtonDown(button)) {
        m_MouseState[button] = true;

        EventManager::Dispatcher().enqueue<Events::MouseDown>(button, position);
      }

      if (IsMouseButtonUp(button) && m_MouseState[button]) {
        m_MouseState[button] = false;

        EventManager::Dispatcher().enqueue<Events::MouseUp>(button, position);
      }
    }

    float mouseWheelAmount = GetMouseWheelMove();

    if (mouseWheelAmount != 0) {
      EventManager::Dispatcher().enqueue<Events::MouseWheel>(mouseWheelAmount, position);
    }

    if (position.x != previousPosition.x || position.y != previousPosition.y) {
      EventManager::Dispatcher().enqueue<Events::MouseMove>(position, previousPosition);
    }
  }

  void Manager::HandleActions() {

  }

  void Manager::HandlePressedActions() {
    while (!m_KeyQueue.empty()) {
      auto keyPressed = m_KeyQueue[0];
      m_KeyQueue.pop_back();

      if (Key::IsModifier(keyPressed)) {
        continue;
      }

      Modifier mod = Modifier::NONE;

      if (m_ModState.control) {
        mod = Modifier::CONTROL;
      } else if (m_ModState.shift) {
        mod = Modifier::SHIFT;
      } else if (m_ModState.alt) {
        mod = Modifier::ALT;
      } else if (m_ModState.super) {
        mod = Modifier::SUPER;
      }
      auto inputAction = m_Mapping[{keyPressed, mod}];

      if (inputAction != ActionType::None) {
        EventManager::Dispatcher().enqueue<Events::InputAction>(inputAction);
      }
    }

    m_KeyQueue.clear();
  }

  void Manager::SetModifierState(int key) {
    if (Key::IsModifier(key)) {
      if (Key::IsControl(key)) {
        m_ModState.control = true;
      } else if (Key::IsAlt(key)) {
        m_ModState.alt = true;
      } else if (Key::IsShift(key)) {
        m_ModState.shift = true;
      } else if (Key::IsSuper(key)) {
        m_ModState.super = true;
      }
    }
  }

  void Manager::ClearModifierState(int key) {
    if (Key::IsModifier(key)) {
      if (Key::IsControl(key)) {
        m_ModState.control = false;
      } else if (Key::IsAlt(key)) {
        m_ModState.alt = false;
      } else if (Key::IsShift(key)) {
        m_ModState.shift = false;
      } else if (Key::IsSuper(key)) {
        m_ModState.super = false;
      }
    }

  }

  [[maybe_unused]] Modifier Manager::GetModifierState() const {
    if (m_ModState.control) {
      return Modifier::CONTROL;
    } else if (m_ModState.shift) {
      return Modifier::SHIFT;
    } else if (m_ModState.alt) {
      return Modifier::ALT;
    } else if (m_ModState.super) {
      return Modifier::SUPER;
    }

    return Modifier::NONE;
  }
} // dorito