#include "GamePrefs.h"

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