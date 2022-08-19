#include "Preferences.h"

void to_json(json &j, const Color &c) {
  j = json{{"r", c.r},
           {"g", c.g},
           {"b", c.b},
           {"a", c.a}};
}

void from_json(const json &j, Color &c) {
  j.at("r").get_to(c.r);
  j.at("g").get_to(c.g);
  j.at("b").get_to(c.b);
  j.at("a").get_to(c.a);
}

namespace dorito {
  void to_json(json &j, const DoritoPrefs &dp) {
    j = json{{"isMuted",           dp.isMuted},
             {"recentRoms",        dp.recentRoms},
             {"recentSourceFiles", dp.recentSourceFiles}};
  }

  void from_json(const json &j, DoritoPrefs &dp) {
    if (j.contains("isMuted"))
      j.at("isMuted").get_to(dp.isMuted);
    else
      dp.isMuted = false;

    if (j.contains("recentRoms"))
      j.at("recentRoms").get_to(dp.recentRoms);

    if (j.contains("recentSourceFiles"))
      j.at("recentSourceFiles").get_to(dp.recentSourceFiles);
  }

  void to_json(json &j, const GamePrefs &gp) {
    j = json{{"cyclesPerFrame", gp.cyclesPerFrame},
             {"quirks",         gp.quirks},
             {"palette",        gp.palette}};
  }

  void from_json(const json &j, GamePrefs &gp) {
    j.at("cyclesPerFrame").get_to(gp.cyclesPerFrame);
    j.at("quirks").get_to(gp.quirks);
    j.at("palette").get_to(gp.palette);
  }
} // dorito