#include "ZepImGuiExCommand.h"

#include "core/events/EventManager.h"

namespace dorito {
  void ZepImGuiExCommand::Run(const std::vector<std::string> &) {
    EventManager::Dispatcher().enqueue<Events::ToggleImguiDemo>();
  }

  const char *ZepImGuiExCommand::ExCommandName() const {
    return "imguidemo";
  }
} // dorito