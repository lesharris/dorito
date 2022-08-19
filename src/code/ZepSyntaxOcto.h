#pragma once

#include <zep/editor.h>
#include <zep/syntax.h>

namespace dorito {

  class ZepSyntaxOcto final : public Zep::ZepSyntax {
    using tSyntaxFactory = std::function<std::shared_ptr<Zep::ZepSyntax>(Zep::ZepBuffer *)>;

  public:
    ZepSyntaxOcto(Zep::ZepBuffer &buffer);

    static void registerSyntax(std::unique_ptr<Zep::ZepEditor> &editor);

    virtual void UpdateSyntax() override;
  };

} // dorito

