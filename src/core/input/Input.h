#pragma once

#include "raylib.h"
#include <vector>
#include <unordered_map>
#include <string>

#include "core/events/EventManager.h"

#include "core/Dorito.h"

#include "Keys.h"
#include "InputActions.h"
#include "Mapping.h"

namespace dorito::Input {
  class Manager {
  public:
    static Manager &Get() {
      static Manager instance;

      return instance;
    }

    void Poll();

  public:
    [[maybe_unused]] static char KeyToChar(int key) {
      if (m_PrintableKeys.contains(key)) {
        return m_PrintableKeys.at(key);
      } else {
        return ' ';
      }
    }

    [[maybe_unused]] static char ShiftedKeyToChar(int key) {
      if (m_ShiftedValue.contains(key)) {
        return m_ShiftedValue.at(key);
      } else {
        return m_PrintableKeys.at(key);
      }
    }

    [[maybe_unused]] static bool IsPrintable(int key) {
      return m_PrintableKeys.contains(key);
    }

  private:
    Manager();

    ~Manager() = default;

    void PollKeyboard();

    void PollMouse();

    void HandleActions();

    void HandlePressedActions();

    [[maybe_unused]] [[nodiscard]] Modifier GetModifierState() const;

    void SetModifierState(int key);

    void ClearModifierState(int key);

  private:
    struct ModifierState {
      bool control = false;
      bool alt = false;
      bool shift = false;
      bool super = false;
    };

  private:
    static const std::unordered_map<int, char> m_PrintableKeys;
    static const std::unordered_map<int, char> m_ShiftedValue;

  private:
    Mapping m_Mapping;

    std::vector<int> m_KeyQueue;

    ModifierState m_ModState;
    std::vector<bool> m_MouseState{false, false, false, false, false, false, false};

    std::unordered_map<int, bool> m_KeyState = {
        {KEY_NULL,          false},
        {KEY_APOSTROPHE,    false},
        {KEY_COMMA,         false},
        {KEY_MINUS,         false},
        {KEY_PERIOD,        false},
        {KEY_SLASH,         false},
        {KEY_ZERO,          false},
        {KEY_ONE,           false},
        {KEY_TWO,           false},
        {KEY_THREE,         false},
        {KEY_FOUR,          false},
        {KEY_FIVE,          false},
        {KEY_SIX,           false},
        {KEY_SEVEN,         false},
        {KEY_EIGHT,         false},
        {KEY_NINE,          false},
        {KEY_SEMICOLON,     false},
        {KEY_EQUAL,         false},
        {KEY_A,             false},
        {KEY_B,             false},
        {KEY_C,             false},
        {KEY_D,             false},
        {KEY_E,             false},
        {KEY_F,             false},
        {KEY_G,             false},
        {KEY_H,             false},
        {KEY_I,             false},
        {KEY_J,             false},
        {KEY_K,             false},
        {KEY_L,             false},
        {KEY_M,             false},
        {KEY_N,             false},
        {KEY_O,             false},
        {KEY_P,             false},
        {KEY_Q,             false},
        {KEY_R,             false},
        {KEY_S,             false},
        {KEY_T,             false},
        {KEY_U,             false},
        {KEY_V,             false},
        {KEY_W,             false},
        {KEY_X,             false},
        {KEY_Y,             false},
        {KEY_Z,             false},
        {KEY_LEFT_BRACKET,  false},
        {KEY_BACKSLASH,     false},
        {KEY_RIGHT_BRACKET, false},
        {KEY_GRAVE,         false},
        {KEY_SPACE,         false},
        {KEY_ESCAPE,        false},
        {KEY_ENTER,         false},
        {KEY_TAB,           false},
        {KEY_BACKSPACE,     false},
        {KEY_INSERT,        false},
        {KEY_DELETE,        false},
        {KEY_RIGHT,         false},
        {KEY_LEFT,          false},
        {KEY_DOWN,          false},
        {KEY_UP,            false},
        {KEY_PAGE_UP,       false},
        {KEY_PAGE_DOWN,     false},
        {KEY_HOME,          false},
        {KEY_END,           false},
        {KEY_CAPS_LOCK,     false},
        {KEY_SCROLL_LOCK,   false},
        {KEY_NUM_LOCK,      false},
        {KEY_PRINT_SCREEN,  false},
        {KEY_PAUSE,         false},
        {KEY_F1,            false},
        {KEY_F2,            false},
        {KEY_F3,            false},
        {KEY_F4,            false},
        {KEY_F5,            false},
        {KEY_F6,            false},
        {KEY_F7,            false},
        {KEY_F8,            false},
        {KEY_F9,            false},
        {KEY_F10,           false},
        {KEY_F11,           false},
        {KEY_F12,           false},
        {KEY_LEFT_SHIFT,    false},
        {KEY_LEFT_CONTROL,  false},
        {KEY_LEFT_ALT,      false},
        {KEY_LEFT_SUPER,    false},
        {KEY_RIGHT_SHIFT,   false},
        {KEY_RIGHT_CONTROL, false},
        {KEY_RIGHT_ALT,     false},
        {KEY_RIGHT_SUPER,   false},
        {KEY_KB_MENU,       false},
        {KEY_KP_0,          false},
        {KEY_KP_1,          false},
        {KEY_KP_2,          false},
        {KEY_KP_3,          false},
        {KEY_KP_4,          false},
        {KEY_KP_5,          false},
        {KEY_KP_6,          false},
        {KEY_KP_7,          false},
        {KEY_KP_8,          false},
        {KEY_KP_9,          false},
        {KEY_KP_DECIMAL,    false},
        {KEY_KP_DIVIDE,     false},
        {KEY_KP_MULTIPLY,   false},
        {KEY_KP_SUBTRACT,   false},
        {KEY_KP_ADD,        false},
        {KEY_KP_ENTER,      false},
        {KEY_KP_EQUAL,      false},
        {KEY_BACK,          false},
        {KEY_MENU,          false},
        {KEY_VOLUME_UP,     false},
        {KEY_VOLUME_DOWN,   false}
    };

    std::vector<int> m_MouseButtons =
        {
            MOUSE_BUTTON_LEFT,
            MOUSE_BUTTON_RIGHT,
            MOUSE_BUTTON_MIDDLE,
            MOUSE_BUTTON_SIDE,
            MOUSE_BUTTON_EXTRA,
            MOUSE_BUTTON_FORWARD,
            MOUSE_BUTTON_BACK
        };

    std::vector<int> m_Keys =
        {
            KEY_NULL,
            KEY_APOSTROPHE,
            KEY_COMMA,
            KEY_MINUS,
            KEY_PERIOD,
            KEY_SLASH,
            KEY_ZERO,
            KEY_ONE,
            KEY_TWO,
            KEY_THREE,
            KEY_FOUR,
            KEY_FIVE,
            KEY_SIX,
            KEY_SEVEN,
            KEY_EIGHT,
            KEY_NINE,
            KEY_SEMICOLON,
            KEY_EQUAL,
            KEY_A,
            KEY_B,
            KEY_C,
            KEY_D,
            KEY_E,
            KEY_F,
            KEY_G,
            KEY_H,
            KEY_I,
            KEY_J,
            KEY_K,
            KEY_L,
            KEY_M,
            KEY_N,
            KEY_O,
            KEY_P,
            KEY_Q,
            KEY_R,
            KEY_S,
            KEY_T,
            KEY_U,
            KEY_V,
            KEY_W,
            KEY_X,
            KEY_Y,
            KEY_Z,
            KEY_LEFT_BRACKET,
            KEY_BACKSLASH,
            KEY_RIGHT_BRACKET,
            KEY_GRAVE,
            KEY_SPACE,
            KEY_ESCAPE,
            KEY_ENTER,
            KEY_TAB,
            KEY_BACKSPACE,
            KEY_INSERT,
            KEY_DELETE,
            KEY_RIGHT,
            KEY_LEFT,
            KEY_DOWN,
            KEY_UP,
            KEY_PAGE_UP,
            KEY_PAGE_DOWN,
            KEY_HOME,
            KEY_END,
            KEY_CAPS_LOCK,
            KEY_SCROLL_LOCK,
            KEY_NUM_LOCK,
            KEY_PRINT_SCREEN,
            KEY_PAUSE,
            KEY_F1,
            KEY_F2,
            KEY_F3,
            KEY_F4,
            KEY_F5,
            KEY_F6,
            KEY_F7,
            KEY_F8,
            KEY_F9,
            KEY_F10,
            KEY_F11,
            KEY_F12,
            KEY_LEFT_SHIFT,
            KEY_LEFT_CONTROL,
            KEY_LEFT_ALT,
            KEY_LEFT_SUPER,
            KEY_RIGHT_SHIFT,
            KEY_RIGHT_CONTROL,
            KEY_RIGHT_ALT,
            KEY_RIGHT_SUPER,
            KEY_KB_MENU,
            KEY_KP_0,
            KEY_KP_1,
            KEY_KP_2,
            KEY_KP_3,
            KEY_KP_4,
            KEY_KP_5,
            KEY_KP_6,
            KEY_KP_7,
            KEY_KP_8,
            KEY_KP_9,
            KEY_KP_DECIMAL,
            KEY_KP_DIVIDE,
            KEY_KP_MULTIPLY,
            KEY_KP_SUBTRACT,
            KEY_KP_ADD,
            KEY_KP_ENTER,
            KEY_KP_EQUAL,
            KEY_BACK,
            KEY_MENU,
            KEY_VOLUME_UP,
            KEY_VOLUME_DOWN
        };
  };

} // dorito

