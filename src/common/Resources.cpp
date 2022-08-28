#include "Resources.h"

#include "config.h"
#include <filesystem>
#include <raylib.h>

#ifdef APPLE

  #include "../../platform/FolderManager.h"

#endif

namespace dorito {
  std::string Resources::ConfigDirectory() {
#ifdef APPLE
    fm::FolderManager folderManager;

    std::string dir = (char *) folderManager.pathForDirectory(fm::NSApplicationSupportDirectory,
                                                              fm::NSUserDirectory);
    dir += "/Dorito";

    if (!DirectoryExists(dir.c_str())) {
      std::filesystem::create_directory(dir);
    }

    return dir;
#else
    return {"."};
#endif
  }

  std::string Resources::FontPath(const std::string &name) {
#ifdef APPLE
    fm::FolderManager folderManager;

    return folderManager.pathForResource(name.c_str());
#else
    std::string base = "assets/fonts/";
    return base + name;
#endif
  }

  std::string Resources::ImagePath(const std::string &name) {
#ifdef APPLE
    fm::FolderManager folderManager;
    return folderManager.pathForResource(name.c_str());
#else
    std::string path = "assets/" + name;
    return path;
#endif
  }
} // dorito