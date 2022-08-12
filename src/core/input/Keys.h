#pragma once

#include <vector>
#include "raylib.h"

namespace dorito::Input {
  enum class Modifier {
    ALT,
    LEFT_ALT,
    RIGHT_ALT,
    SHIFT,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    SUPER,
    LEFT_SUPER,
    RIGHT_SUPER,
    CONTROL,
    LEFT_CONTROL,
    RIGHT_CONTROL,
    CAPSLOCK,
    NONE
  };

  class Key {
  public:
    Key() = default;

    Key(int key) : key(key), modifier(Modifier::NONE) {}

    Key(int key, Modifier modifier) : key(key), modifier(modifier) {}

  public:
    static bool IsControl(int key) {
      return key == KEY_LEFT_CONTROL || key == KEY_RIGHT_CONTROL;
    }

    static bool IsAlt(int key) {
      return key == KEY_LEFT_ALT || key == KEY_RIGHT_ALT;
    }

    static bool IsShift(int key) {
      return key == KEY_LEFT_SHIFT || key == KEY_RIGHT_SHIFT;
    }

    static bool IsSuper(int key) {
      return key == KEY_LEFT_SUPER || key == KEY_RIGHT_SUPER;
    }

    static bool IsModifier(int key) {
      switch (key) {
        case KEY_LEFT_CONTROL:
          return true;

        case KEY_RIGHT_CONTROL:
          return true;

        case KEY_LEFT_ALT:
          return true;

        case KEY_RIGHT_ALT:
          return true;

        case KEY_LEFT_SHIFT:
          return true;

        case KEY_RIGHT_SHIFT:
          return true;

        case KEY_LEFT_SUPER:
          return true;

        case KEY_RIGHT_SUPER:
          return true;

        case KEY_CAPS_LOCK:
          return true;

        default:
          return false;
      }
    }

    static Modifier KeyToModifier(int key) {
      switch (key) {
        case KEY_LEFT_CONTROL:
        case KEY_RIGHT_CONTROL:
          return Modifier::CONTROL;

        case KEY_LEFT_ALT:
        case KEY_RIGHT_ALT:
          return Modifier::ALT;

        case KEY_LEFT_SHIFT:
        case KEY_RIGHT_SHIFT:
          return Modifier::SHIFT;

        case KEY_LEFT_SUPER:
        case KEY_RIGHT_SUPER:
          return Modifier::SUPER;

        case KEY_CAPS_LOCK:
          return Modifier::CAPSLOCK;

        default:
          return Modifier::NONE;
      }
    }

    bool operator==(const Key &k) const {
      return k.key == key && k.modifier == modifier;
    }

  public:
    int key = -1;
    Modifier modifier;

  protected:
    friend std::hash<Key>;
  };
}

namespace std {
  template<>
  struct hash<dorito::Input::Key> {
    size_t operator()(dorito::Input::Key const &key) const noexcept {
      return key.key;
    }
  };
}