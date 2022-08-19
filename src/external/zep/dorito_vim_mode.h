#include <zep.h>

class DoritoVimMode : public Zep::ZepMode_Vim {
public:
  bool UsesRelativeLines() const override {
    return false;
  }
};
