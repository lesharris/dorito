#pragma once

#include <vector>
#include <string>

#include "raylib.h"

#include "Event.h"
#include "core/input/InputActions.h"
#include "core/input/Keys.h"

namespace dorito::Events {
  struct KeyPressed : public Event {
    KeyPressed(int key = -1) : Event(), key(key) {};

    int key;
  };

  struct KeyReleased : public Event {
    KeyReleased(int key = -1) : Event(), key(key) {};
    int key;
  };

  struct KeyDown : public Event {
    KeyDown(int key = -1) : Event(), key(key) {};
    int key;
  };

  struct KeyUp : public Event {
    KeyUp(int key = -1) : Event(), key(key) {};
    int key;
  };

  struct WindowResized : public Event {
    WindowResized(int width = 0, int height = 0) : Event(), width(width), height(height) {};
    int width;
    int height;
  };

  struct MouseMove : public Event {
    MouseMove(Vector2 position = {}, Vector2 previous = {})
        : Event(), position(position), previousPosition(previous) {}

    Vector2 position;
    Vector2 previousPosition;
  };

  struct MousePressed : public Event {
    MousePressed(int button = -1, Vector2 position = {})
        : Event(), button(button), position(position) {}

    int button;
    Vector2 position;
  };

  struct MouseReleased : public Event {
    MouseReleased(int button = -1, Vector2 position = {})
        : Event(), button(button), position(position) {}

    int button;
    Vector2 position;
  };

  struct MouseDown : public Event {
    MouseDown(int button = -1, Vector2 position = {})
        : Event(), button(button), position(position) {}

    int button;
    Vector2 position;
  };

  struct MouseUp : public Event {
    MouseUp(int button = -1, Vector2 position = {})
        : Event(), button(button), position(position) {}

    int button;
    Vector2 position;
  };

  struct MouseWheel : public Event {
    MouseWheel(float amount = 0.0f, Vector2 position = {})
        : Event(), amount(amount), position(position) {}

    float amount;
    Vector2 position;
  };

  struct InputAction : public Event {
    InputAction(Input::ActionType type = Input::ActionType::None) : Event(), type(type) {}

    Input::ActionType type{};
  };

  struct WantQuit : public Event {
    WantQuit() : Event() {}
  };

  struct ViewportResized : public Event {
    ViewportResized(float x = 0.0f, float y = 0.0f) : Event(), x(x), y(y) {};

    float x;
    float y;
  };

  struct UIMouseMove : public Event {
    UIMouseMove(Vector2 position = {0.0, 0.0}) : Event(), position(position) {};

    UIMouseMove(float x = 0.0f, float y = 0.0f) : Event(), position({x, y}) {};

    Vector2 position;
  };

  struct ExecuteCPU : public Event {
    ExecuteCPU(bool execute = true) : Event(), execute(execute) {}

    bool execute;
  };

  struct StepCPU : public Event {
    StepCPU() : Event() {}
  };

  struct ExecuteUntil : public Event {
    ExecuteUntil(uint16_t addr) : Event(), addr(addr) {}

    uint16_t addr;
  };

  struct VBlank : public Event {
    VBlank() : Event() {}
  };

  struct HandleAudio : public Event {
    HandleAudio() : Event() {}
  };

  struct LoadROM : public Event {
    LoadROM(const std::string &path) : Event(), path(path) {}

    std::string path;
  };

  struct UnloadROM : public Event {
    UnloadROM() : Event() {}
  };

  struct Reset : public Event {
    Reset() : Event() {}
  };
}