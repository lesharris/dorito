#pragma once

#include <raylib.h>
#include <vector>
#include <map>

#include <imgui/imgui.h>

#include "common/common.h"
#include "core/Dorito.h"

#include "core/events/EventManager.h"
#include "core/layers/GameLayer.h"

#include "system/Bus.h"

#include "widgets/AudioWidget.h"
#include "widgets/ColorEditorWidget.h"
#include "widgets/DisassemblyWidget.h"
#include "widgets/EditorWidget.h"
#include "widgets/MainMenuWidget.h"
#include "widgets/MemoryEditorWidget.h"
#include "widgets/RegistersWidget.h"
#include "widgets/ViewportWidget.h"
#include "widgets/SpriteEditorWidget.h"
#include "widgets/SoundEditorWidget.h"
#include "widgets/MonitorsWidget.h"
#include "widgets/BreakpointsWidget.h"

namespace dorito {

  class UI : public GameLayer {
  public:
    void OnAttach() override;

    void OnDetach() override;

    void Render() override;

    std::string Name() const noexcept override;

    bool RenderTarget() override { return false; }

    void BeginFrame() override;

    void EndFrame() override;

    std::map<std::string, bool> GetWidgetStatus() const {
      std::map<std::string, bool> results;

      for (auto widget: m_Widgets) {
        results.insert_or_assign(widget->Name(), widget->Enabled());
      }

      return results;
    }

    void SetWidgetStatus(const std::map<std::string, bool> statuses) {
      for (const auto &[name, status]: statuses) {
        std::string nameBinding = name;
        auto it = std::find_if(m_Widgets.begin(), m_Widgets.end(), [nameBinding](Ref<Widget> &widget) {
          return widget->Name() == nameBinding;
        });

        if (it != std::end(m_Widgets)) {
          (*it)->Enabled(status);
        }
      }
    }

  public:
    static uint16_t PrevPC;

  private:
    void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle);

    ImVec2 ImRotate(const ImVec2 &v, float cos_a, float sin_a);

  private:
    void HandleKeyPress(Events::KeyPressed &event);

    void HandleResetPC(Events::UIResetPC &event);

    void HandleToggleEnabled(Events::UIToggleEnabled &event);

    void HandleToggleImguiDemo(Events::ToggleImguiDemo &event);

  private:
    std::vector<Ref<Widget>> m_Widgets;
    std::string m_IniPath;
    bool m_ShowImGuiDemo = false;
  };

} // dorito

