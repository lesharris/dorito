#pragma once

#include <vector>

#include <raylib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace dorito {
  struct GamePrefs {
    uint32_t cyclesPerFrame;
    std::vector<bool> quirks;
    std::vector<Color> palette;
  };

  void to_json(json &j, const GamePrefs &gp);

  void from_json(const json &j, GamePrefs &gp);

}

void to_json(json &j, const Color &c);

void from_json(const json &j, Color &c);
