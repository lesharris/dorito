#pragma once

#include "config.h"

#include "raylib.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "common/common.h"
#include "core/layers/GameLayerStack.h"
#include "core/events/EventManager.h"

#include <iostream>
#include <filesystem>


#ifdef APPLE

  #include "external/mac/FolderManager.h"

#endif

namespace dorito {

  class Dorito {
  public:
    static Dorito &Get() {
      static Dorito instance;

      return instance;
    }

    void Run();

    Vector2 MousePosition() { return m_MousePosition; }

    Vector2 PreviousMousePosition() { return m_PreviousMousePosition; }

    Vector2 WorldCoords() { return m_WorldCoords; }

    RenderTexture &GetRenderTexture() { return m_RenderTexture; }

    Texture &GetLogoTexture() { return m_Logo; }

    int32_t ScreenWidth() {
      return m_ScreenWidth;
    }

    int32_t ScreenHeight() {
      return m_ScreenHeight;
    }

  private:
    Dorito() {

#ifdef APPLE
      fm::FolderManager folderManager;

      std::string logPath = (char *) folderManager.pathForDirectory(fm::NSApplicationSupportDirectory,
                                                                    fm::NSUserDirectory);
      logPath += "/Dorito";

      if (!DirectoryExists(logPath.c_str())) {
        std::filesystem::create_directory(logPath);
      }

      logPath += "/dorito.log";

      m_LogoPath = folderManager.pathForResource("dorito.png");
#endif

#ifdef WINDOWS
      std::string logPath = "dorito.log";
      m_LogoPath = "assets/dorito.png";
#endif

      auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath, true);

      m_ConsoleLogger = std::make_shared<spdlog::logger>("console", spdlog::sinks_init_list{consoleSink, fileSink});

      spdlog::register_logger(m_ConsoleLogger);

      Initialize();
    };

    ~Dorito();

    void Initialize();

  private:
    static void RaylibTraceCallback(int logLevel, const char *text, va_list args);

  private:
    void HandleViewportResized(const Events::ViewportResized &event);

    void HandleMouseMove(const Events::UIMouseMove &event);

    void HandleWantQuit(const Events::WantQuit &event);

    void HandleVBlank(const Events::VBlank &event);

  private:
    GameLayerStack *m_GameLayers = nullptr;

    Ref<spdlog::logger> m_ConsoleLogger;

    std::string m_Title;

    Color m_DefaultBackground{30, 30, 30, 255};

    bool m_Running = true;

    int32_t m_ScreenWidth = 1600;
    int32_t m_ScreenHeight = 900;

    Vector2 m_ViewportSize{1600, 900};

    double m_CurrentTime = 0;
    double m_Timestep = 0;

    Vector2 m_MousePosition{};
    Vector2 m_PreviousMousePosition{};
    Vector2 m_WorldCoords{};

    Camera2D m_Camera{};
    RenderTexture m_RenderTexture;

    Texture m_Logo;
    std::string m_LogoPath;

    class System *m_System = nullptr;
  };

} // dorito

