#include "Emu.h"

#include <raylib.h>
#include "common/common.h"

namespace dorito {
  void Emu::OnAttach() {
    EventManager::Get().Attach<
        Events::KeyPressed,
        &Emu::HandleKeyPress
    >(this);

    EventManager::Get().Attach<
        Events::InputAction,
        &Emu::HandleAction
    >(this);
  }

  void Emu::OnDetach() {
    EventManager::Get().DetachAll(this);
  }

  void Emu::Update(double) {
    auto &bus = Bus::Get();
    bus.Tick();
    bus.TickTimers();
  }

  void Emu::Render() {
    auto &bus = Bus::Get();

    const auto &palette = bus.Palette();
    const auto &buffers = bus.Buffers();

    auto scw = app.ScreenWidth();

    uint8_t bufferHeight = 64;
    uint8_t bufferWidth = 128;

    auto scale = scw / bufferWidth;

    auto texWidth = app.GetRenderTexture().texture.width;
    auto texHeight = app.GetRenderTexture().texture.height;

    auto offsetX = (texWidth - (bufferWidth * scale)) / 2;
    auto offsetY = (texHeight - (bufferHeight * scale)) / 2;

    for (auto y = 0; y < bufferHeight; y++) {
      for (auto x = 0; x < bufferWidth; x++) {
        uint8_t p1 = buffers[0][y * bufferWidth + x];
        uint8_t p2 = buffers[1][y * bufferWidth + x];
        uint8_t entry = p2 << 1 | p1;
        
        DrawRectangle((x * scale) + offsetX, (y * scale) + offsetY,
                      scale, scale,
                      palette[entry]);
      }
    }
  }

  std::string Emu::Name() const noexcept {
    return {"emu"};
  }

  void Emu::HandleKeyPress(Events::KeyPressed &event) {
    if (event.handled) {
      return;
    }

    bool handled = true;

    switch (event.key) {
      default:
        handled = false;
        break;
    }

    event.handled = handled;
  }

  void Emu::HandleAction(const Events::InputAction &event) {
    switch (event.type) {

      default:
        break;
    }
  }

  void Emu::RenderTexture() {
  }
} // dorito