#pragma once

#include <string>

namespace dorito {

  class Resources {
  public:
    static Resources &Get() {
      static Resources instance;

      return instance;
    }

  public:
    std::string ConfigDirectory();

    std::string FontPath(const std::string &name);

    std::string ImagePath(const std::string &name);

  private:
    Resources() {}
  };

} // dorito
