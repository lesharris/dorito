#pragma once

#include <string>

#include "Widget.h"
#include "external/zep/ZepEditor.h"

namespace dorito {

  class EditorWidget : public Widget {
  public:
    void Draw() override;

    std::string Name() override { return "Editor"; };

  private:
    CodeEditor m_Editor{"Code.o8"};
    std::string m_SourceFile;
  };

} // dorito
