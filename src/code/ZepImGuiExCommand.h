#pragma once

#include "zep.h"

namespace dorito {

  class ZepImGuiExCommand : public Zep::ZepExCommand {
  public:
    ZepImGuiExCommand(Zep::ZepEditor &editor) : Zep::ZepExCommand(editor) {}

    void Run(const std::vector<std::string> &args) override;

    const char *ExCommandName() const override;

  private:
  };

} // dorito
