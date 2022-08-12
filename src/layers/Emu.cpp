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

  void Emu::Update(double timestep) {
  }

  void Emu::Render() {
   /* auto scw = app.ScreenWidth();
    const auto &buffer = m_GB.VideoBuffer();
    uint8_t bufferHeight = 144;
    uint8_t bufferWidth = 160;

    if (!m_GB.CartridgeLoaded())
      return;

    auto scale = scw / bufferWidth;

    auto &lcd = LCD::Get();

    if (lcd.LCDC_Enabled()) {
      for (auto y = 0; y < bufferHeight; y++) {
        for (auto x = 0; x < bufferWidth; x++) {
          DrawRectangle(x * scale, y * scale,
                        scale, scale,
                        buffer[(y * bufferWidth) + x]);
        }
      }
    }*/
  }

  std::string Emu::Name() const noexcept {
    return std::string("emu");
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