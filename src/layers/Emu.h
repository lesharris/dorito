#pragma once

#include "common/common.h"
#include "core/Dorito.h"

#include "core/events/EventManager.h"
#include "core/layers/GameLayer.h"

#include "system/Bus.h"

namespace dorito {

  class Emu : public GameLayer {
  public:
    Emu() : GameLayer("Emu") {}

  public:
    void OnAttach() override;

    void OnDetach() override;

    void Update(double timestep) override;

    void Render() override;

    void RenderTexture() override;

    [[nodiscard]] std::string Name() const noexcept override;

    bool RenderTarget() override { return true; }

  private:
    void HandleKeyPress(Events::KeyPressed &event);

    void HandleAction(const Events::InputAction &event);

  private:
    Dorito &app = Dorito::Get();
  };

} // dorito

