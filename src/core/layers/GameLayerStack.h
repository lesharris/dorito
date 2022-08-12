#pragma once

#include <vector>

#include "common/common.h"
#include "GameLayer.h"

namespace dorito {

  class GameLayerStack {
  public:
    GameLayerStack() = default;

    ~GameLayerStack();

    [[maybe_unused]] void PushState(const Ref<GameLayer> &state);

    [[maybe_unused]] void PushOverlay(const Ref<GameLayer> &overlay);

    [[maybe_unused]] void PopState(const Ref<GameLayer> &state);

    [[maybe_unused]] void PopOverlay(const Ref<GameLayer> &overlay);

    std::vector<Ref<GameLayer>>::iterator begin() { return m_States.begin(); }

    std::vector<Ref<GameLayer>>::iterator end() { return m_States.end(); }

    std::vector<Ref<GameLayer>>::reverse_iterator rbegin() { return m_States.rbegin(); }

    std::vector<Ref<GameLayer>>::reverse_iterator rend() { return m_States.rend(); }

    [[nodiscard]] std::vector<Ref<GameLayer>>::const_iterator begin() const { return m_States.begin(); }

    [[nodiscard]] std::vector<Ref<GameLayer>>::const_iterator end() const { return m_States.end(); }

    [[nodiscard]] std::vector<Ref<GameLayer>>::const_reverse_iterator rbegin() const { return m_States.rbegin(); }

    [[nodiscard]] std::vector<Ref<GameLayer>>::const_reverse_iterator rend() const { return m_States.rend(); }

    Ref<GameLayer> &operator[](int index) {
      if ((size_t) index < m_States.size()) {
        return m_States[index];
      }
      return m_DefaultStateRef;
    }

  private:
    std::vector<Ref<GameLayer>> m_States;
    GameLayer m_DefaultState;
    Ref<GameLayer> m_DefaultStateRef = CreateRef<GameLayer>();
    unsigned int m_Index = 0;
  };

} // dorito

