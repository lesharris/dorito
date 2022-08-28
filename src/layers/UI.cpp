#include <zep.h>
#include <raylib.h>

#include "UI.h"
#include "imgui_internal.h"
#include "common/Resources.h"

#include "system/Bus.h"
#include "external/IconsFontAwesome5.h"

namespace dorito {
  uint16_t UI::PrevPC = 0;

  void UI::OnAttach() {
    ImGui::CreateContext(nullptr);

    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

    io.ConfigWindowsMoveFromTitleBarOnly = true;

    auto &resourceManager = Resources::Get();

    std::string monoFont = resourceManager.FontPath("CascadiaMono.ttf");
    std::string iconFont = resourceManager.FontPath("fa-solid-900.ttf");

    m_IniPath = resourceManager.ConfigDirectory() + "/imgui.ini";

    io.IniFilename = m_IniPath.c_str();

    // Render fonts at half scale for sharper fonts.
    auto dpi = GetWindowScaleDPI();
    io.Fonts->AddFontFromFileTTF(monoFont.c_str(), 16.0f * dpi.y);

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF(iconFont.c_str(), 16.0f * dpi.y, &icons_config, icons_ranges);

    io.FontGlobalScale = 1.0f / dpi.y;

    m_Widgets = std::vector<Ref<Widget>>{
        Widget::Create<MainMenuWidget>(),
        Widget::Create<AudioWidget>(),
        Widget::Create<ColorEditorWidget>(),
        Widget::Create<DisassemblyWidget>(),
        Widget::Create<MemoryEditorWidget>(),
        Widget::Create<RegistersWidget>(),
        Widget::Create<ViewportWidget>(),
        Widget::Create<SpriteEditorWidget>(),
        Widget::Create<SoundEditorWidget>(),
        Widget::Create<MonitorsWidget>(),
        Widget::Create<BreakpointsWidget>(),
        Widget::Create<EditorWidget>()
    };

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

    EventManager::Get().Attach<
        Events::UIToggleEnabled,
        &::dorito::UI::HandleToggleEnabled
    >(this);

    EventManager::Get().Attach<
        Events::ToggleImguiDemo,
        &::dorito::UI::HandleToggleImguiDemo
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

    for (auto widget: m_Widgets) {
      if (widget->Enabled())
        widget->Draw();
    }

    if (m_ShowImGuiDemo) {
      ImGui::ShowDemoWindow();
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

  void UI::HandleToggleEnabled(Events::UIToggleEnabled &event) {
    for (auto widget: m_Widgets) {

      if (widget->Name() == event.name) {
        widget->Enabled(!widget->Enabled());

        EventManager::Dispatcher().enqueue(Events::SaveAppPrefs());
        break;
      }
    }
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

  void UI::HandleToggleImguiDemo(Events::ToggleImguiDemo &) {
    m_ShowImGuiDemo = !m_ShowImGuiDemo;
  }

} // dorito