
#include "config.h"
#include "input/Input.h"
#include "events/EventManager.h"
#include "events/Events.h"

#include "Dorito.h"

#include "layers/Emu.h"
#include "layers/UI.h"

#include <SDL.h>

namespace dorito {
  void Dorito::Run() {
    if (!m_GameLayers)
      return;

    m_GameLayers->PushState(CreateRef<Emu>());
    m_GameLayers->PushOverlay(CreateRef<UI>());

    double lastTime = GetTime();

    m_PreviousMousePosition = GetMousePosition();

    do {
      m_CurrentTime = GetTime();
      m_Timestep = m_CurrentTime - lastTime;

      m_WorldCoords = GetScreenToWorld2D(m_MousePosition, m_Camera);

      Input::Manager::Get().Poll();

      // Dispatch queued events
      EventManager::Dispatcher().update();

      for (const auto &layer: *m_GameLayers) {
        layer->Update(m_Timestep);
      }

      BeginDrawing();

      for (const auto &layer: *m_GameLayers) {
        layer->BeginFrame();
      }

      ClearBackground(m_DefaultBackground);

      for (auto layer: *m_GameLayers) {
        if (!layer->RenderTarget())
          layer->Render();
      }

      for (const auto &layer: *m_GameLayers) {
        layer->EndFrame();
      }

      EndDrawing();

      EventManager::Dispatcher().trigger(Events::HandleAudio{});

      lastTime = m_CurrentTime;
    } while (m_Running && !WindowShouldClose());
  }

  void Dorito::Initialize() {
    SetTraceLogCallback(RaylibTraceCallback);

    m_Title = fmt::format("Dorito v{}", DORITO_VERSION);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_ScreenWidth, m_ScreenHeight, m_Title.c_str());

    SetWindowMinSize(100, 100);

    SetExitKey(0);

    SetTargetFPS(62);

    while (!IsWindowReady());
    m_Camera.target = {0, 0};
    m_Camera.offset = {0, 0};
    m_Camera.rotation = 0.0f;
    m_Camera.zoom = 1.0f;

    m_GameLayers = new GameLayerStack();

    float aspect = 4.0f / 3.0f;
    m_ScreenWidth = static_cast<int32_t>(m_ScreenHeight * aspect);

    m_RenderTexture = LoadRenderTexture(m_ScreenWidth, m_ScreenHeight);
    SetTextureFilter(m_RenderTexture.texture, TEXTURE_FILTER_POINT);

    SDL_Init(SDL_INIT_AUDIO);

    EventManager::Get().Attach<
        Events::ViewportResized,
        &Dorito::HandleViewportResized
    >(this);

    EventManager::Get().Attach<
        Events::UIMouseMove,
        &Dorito::HandleMouseMove
    >(this);

    EventManager::Get().Attach<
        Events::WantQuit,
        &Dorito::HandleWantQuit
    >(this);

    EventManager::Get().Attach<
        Events::VBlank,
        &Dorito::HandleVBlank
    >(this);
  }

  Dorito::~Dorito() {
    EventManager::Get().DetachAll(this);

    delete m_GameLayers;

    UnloadRenderTexture(m_RenderTexture);

    SDL_Quit();
    CloseWindow();
  }

  void Dorito::RaylibTraceCallback(int logLevel, const char *text, va_list args) {
    char buffer[4096];

    vsnprintf(buffer, 4096, text, args);
    buffer[4095] = '\0';

    auto log = spdlog::get("raylib");

    switch (logLevel) {
      case LOG_TRACE:
        log->trace("{}", buffer);
        break;

      case LOG_DEBUG:
        log->debug("{}", buffer);
        break;

      case LOG_INFO:
        log->info("{}", buffer);
        break;

      case LOG_WARNING:
        log->warn("{}", buffer);
        break;

      case LOG_ERROR:
        log->error("{}", buffer);
        break;

      case LOG_FATAL:
        log->critical("{}", buffer);
        break;

      default:
        break;
    }
  }

  void Dorito::HandleViewportResized(const Events::ViewportResized &event) {
    UnloadRenderTexture(m_RenderTexture);

    m_ViewportSize.x = event.x;
    m_ViewportSize.y = event.y;

    float aspect = 4.0f / 3.0f;

    m_ScreenWidth = static_cast<int32_t>(event.y * aspect);
    m_ScreenHeight = event.y;

    m_RenderTexture = LoadRenderTexture(m_ScreenWidth, m_ScreenHeight);
    SetTextureFilter(m_RenderTexture.texture, TEXTURE_FILTER_POINT);
  }

  void Dorito::HandleMouseMove(const Events::UIMouseMove &event) {
    m_PreviousMousePosition = m_MousePosition;

    m_MousePosition = event.position;
  }

  void Dorito::HandleWantQuit(const Events::WantQuit &) {
    m_Running = false;
  }

  void Dorito::HandleVBlank(const Events::VBlank &) {
    BeginTextureMode(m_RenderTexture);
    ClearBackground(m_DefaultBackground);
    for (auto layer: *m_GameLayers) {
      if (layer->RenderTarget()) {
        layer->Render();
      }
    }
    EndTextureMode();
  }

} // dorito