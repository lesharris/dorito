#pragma once

#include <vector>
#include <string>

#include "raylib.h"

#include "Event.h"
#include "core/input/InputActions.h"
#include "core/input/Keys.h"

#include "cpu/Chip8.h"

namespace dorito::Events {
  struct KeyPressed : public Event {
    explicit KeyPressed(int key = -1) : Event(), key(key) {};

    int key;
  };

  struct KeyReleased : public Event {
    explicit KeyReleased(int key = -1) : Event(), key(key) {};
    int key;
  };

  struct KeyDown : public Event {
    explicit KeyDown(int key = -1) : Event(), key(key) {};
    int key;
  };

  struct KeyUp : public Event {
    explicit KeyUp(int key = -1) : Event(), key(key) {};
    int key;
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

  struct StackUnderflow : public Event {
    StackUnderflow() : Event() {}
  };

  struct OutOfRangeMemAccess : public Event {
    explicit OutOfRangeMemAccess(uint16_t addr) : Event(), addr(addr) {}

    uint16_t addr;
  };

  struct StopBeep : public Event {
    StopBeep() : Event() {}
  };

  struct SetCycles : public Event {
    explicit SetCycles(uint16_t cycles) : Event(), cycles(cycles) {}

    uint16_t cycles;
  };

  struct VIPCompat : public Event {
    VIPCompat() : Event() {}
  };

  struct SCHIPCompat : public Event {
    SCHIPCompat() : Event() {}
  };

  struct XOCompat : public Event {
    XOCompat() : Event() {}
  };

  struct SetQuirk : public Event {
    SetQuirk(Chip8::Quirk quirk, bool value) : Event(), quirk(quirk), value(value) {}

    Chip8::Quirk quirk;
    bool value;
  };

  struct SetColor : public Event {
    SetColor(uint8_t index, Color color) : Event(), index(index), color(color) {}

    uint8_t index;
    Color color;
  };

  struct SetPalette : public Event {
    explicit SetPalette(const std::vector<Color> &palette) : Event(), palette(palette) {}

    std::vector<Color> palette;
  };

  struct SavePrefs : public Event {
    SavePrefs() : Event() {}
  };

  struct SetMute : public Event {
    SetMute(bool isSet) : Event(), isSet(isSet) {}

    bool isSet;
  };

  struct RunCode : public Event {
    explicit RunCode(const char *rom) : Event(), rom(rom) {}

    const char *rom;
  };

  struct UIResetPC : public Event {
    UIResetPC() : Event() {}
  };

  struct UISetWidgetEnabled : public Event {
    UISetWidgetEnabled(const std::string &name, bool isEnabled) : Event(), enabled(isEnabled), name(name) {}

    bool enabled;
    std::string name;
  };

  struct UIClearRecents : public Event {
    UIClearRecents() : Event() {}
  };

  struct UIClearRecentSources : public Event {
    UIClearRecentSources() : Event() {}
  };

  struct UIAddRecentSourceFile : public Event {
    UIAddRecentSourceFile(const std::string &path) : Event(), path(path) {}

    std::string path;
  };

  struct UIClearMonitors : public Event {
    UIClearMonitors() : Event() {}
  };

  struct UIAddMonitor : public Event {
    UIAddMonitor(int32_t type, int32_t base, int32_t len, char *format, char *name)
        : Event(), type(type), base(base), len(len), format(format), name(name) {}

    int32_t type;
    int32_t base;
    int32_t len;
    char *format;
    char *name;
  };
}