#include "UI.h"
#include "imgui_internal.h"

#include <zep.h>
#include <nfd.h>
#include <raylib.h>

#include "system/Bus.h"

namespace dorito {

  bool UI::ShowDemo = false;
  bool UI::ShowEmu = true;
  bool UI::ShowMemory = true;
  bool UI::ShowRegisters = true;
  bool UI::ShowDisassembly = true;
  bool UI::ShowAudio = true;
  bool UI::ShowColorEditor = false;
  bool UI::ShowCodeEditor = true;
  bool UI::ShowSoundEditor = true;
  bool UI::ShowSpriteEditor = true;

  uint16_t UI::PrevPC = 0;

  void UI::OnAttach() {
    ImGui::CreateContext(nullptr);

    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Render fonts at half scale for sharper fonts.
    auto dpi = GetWindowScaleDPI();
    io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 16.0f * dpi.y);
    io.FontGlobalScale = 1.0f / dpi.y;

    auto glfwWindow = glfwGetCurrentContext();
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    EventManager::Get().Attach<
        Events::KeyPressed,
        &::dorito::UI::HandleKeyPress
    >(this);

    EventManager::Get().Attach<
        Events::UIResetPC,
        &::dorito::UI::HandleResetPC
    >(this);
  }

  void UI::OnDetach() {
    EventManager::Get().DetachAll(this);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void UI::EndFrame() {
    ImGuiIO &io = ImGui::GetIO();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      auto currentContext = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(currentContext);
    }
  }

  void UI::Render() {
    auto &bus = Bus::Get();
    auto &io = ImGui::GetIO();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Dockspace", nullptr,
                 ImGuiWindowFlags_MenuBar |
                 ImGuiWindowFlags_NoDocking |
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBringToFrontOnFocus |
                 ImGuiWindowFlags_NoNavFocus);

    ImGui::PopStyleVar(3);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

    m_MainMenu.Draw();

    // Display Windows
    {

      if (ShowDemo) {
        ImGui::ShowDemoWindow(&ShowDemo);
      }

      if (ShowMemory) {
        m_MemoryEditor.Draw();
      }

      if (ShowRegisters) {
        m_Registers.Draw();
      }

      if (ShowDisassembly) {
        m_Disassembly.Draw();
      }

      if (ShowAudio) {
        m_Audio.Draw();
      }

      if (ShowColorEditor) {
        m_ColorEditor.Draw();
      }

      if (ShowCodeEditor) {
        m_Editor.Draw();
      }

      if (ShowSpriteEditor) {
        m_SpriteEditor.Draw();
      }

      if (ShowSoundEditor) {
        m_SoundEditor.Draw();
      }

      m_Viewport.Draw();
    }

    ImGui::End();
  }

  std::string UI::Name() const noexcept {
    return "ui";
  }

  /*
   * Event Handlers
   */
  void UI::HandleKeyPress(Events::KeyPressed &) {

  }

  void UI::HandleResetPC(Events::UIResetPC &) {
    PrevPC = 0;
  }


  /*
   * Utilities
   */

  ImVec2 operator+(const ImVec2 &lhs, const ImVec2 &rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
  }

  ImVec2 UI::ImRotate(const ImVec2 &v, float cos_a, float sin_a) {
    return ImVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a);
  }

  void UI::ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle) {
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    ImVec2 pos[4] =
        {
            center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
        };
    ImVec2 uvs[4] =
        {
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 1.0f),
            ImVec2(1.0f, 0.0f),
            ImVec2(0.0f, 0.0f)
        };

    draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
  }

} // dorito