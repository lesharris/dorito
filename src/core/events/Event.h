#pragma once

namespace dorito::Events {
  struct Event {
    Event() : handled(false) {};

    ~Event() = default;

    bool handled;
  };
}