#include "ViewportWidget.h"

#include "layers/UI.h"

namespace dorito {
  void ViewportWidget::Draw() {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

    static float angle = 0.0f;

    // Render our Emu Viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::SetNextWindowClass(&window_class);

    if (!ImGui::Begin("Screen", &UI::ShowEmu,
                      ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse |
                      ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoDecoration)) {
      ImGui::End();
    } else {
      auto windowWidth = GetScreenWidth();
      auto windowHeight = GetScreenHeight();

      auto size = ImGui::GetContentRegionAvail();
      auto contentMin = ImGui::GetWindowContentRegionMin();
      auto windowPos = ImGui::GetWindowPos();
      auto mousePos = ImGui::GetMousePos();

      auto topLeft = ImVec2{windowPos.x + contentMin.x, windowPos.y + contentMin.y};
      auto bottomRight = ImVec2{topLeft.x + size.x, topLeft.y + size.y};

      ImVec2 emuWindowSize = GetLargestSizeForViewport();
      ImVec2 emuWindowPos = GetCenteredPositionForViewport(emuWindowSize);

      ImGui::SetCursorPos(emuWindowPos);

      // Is mouse is over the viewport window...
      if (ImGui::IsMouseHoveringRect(topLeft, bottomRight)) {
        // Transform from screen to window local coords
        ImVec2 position = {mousePos.x - topLeft.x, mousePos.y - topLeft.y};

        EventManager::Dispatcher().trigger<Events::UIMouseMove>({position.x, position.y});
      }

      if (windowWidth != m_PrevScreenWidth || windowHeight != m_PrevScreenHeight) {
        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;

        EventManager::Dispatcher().trigger<Events::ViewportResized>({size.x, size.y});
      } else {
        auto &texture = Dorito::Get().GetRenderTexture();

        /*auto &io = ImGui::GetIO();

         angle += io.DeltaTime * 1.0f;

         float ninety = 1.5708;
         float one80 = 3.14159;
         float two70 = 4.71239;

         ImageRotated(reinterpret_cast<ImTextureID>((uint64_t) texture.texture.id),
                      {
                          emuWindowPos.x,
                          (emuWindowPos.y / 2.0f) + (emuWindowSize.y) - ImGui::GetCursorPosY()
                      },
                      emuWindowSize, 0);*/


        ImGui::Image(reinterpret_cast<ImTextureID>((uint64_t) texture.texture.id),
                     emuWindowSize,
                     {0, 1}, {1, 0});

        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;
      }

      ImGui::End();
    }

    ImGui::PopStyleVar(3);
  }

  ImVec2 ViewportWidget::GetCenteredPositionForViewport(ImVec2 &aspectSize) {
    ImVec2 size = ImGui::GetContentRegionAvail();

    size.x -= ImGui::GetScrollX();
    size.y -= ImGui::GetScrollY();

    float viewportX = (size.x / 2.0f) - (aspectSize.x / 2.0);
    float viewportY = (size.y / 2.0f) - (aspectSize.y / 2.0);

    return {viewportX + ImGui::GetCursorPosX(), viewportY + ImGui::GetCursorPosY()};
  }

  ImVec2 ViewportWidget::GetLargestSizeForViewport() {
    ImVec2 size = ImGui::GetContentRegionAvail();

    size.x -= ImGui::GetScrollX();
    size.y -= ImGui::GetScrollY();

    float aspectWidth = size.x;
    float aspectHeight = aspectWidth / (2.0f / 1.0f);

    if (aspectHeight > size.y) {
      aspectHeight = size.y;
      aspectWidth = aspectHeight * (2.0f / 1.0f);
    }

    return {aspectWidth, aspectHeight};
  }
} // dorito