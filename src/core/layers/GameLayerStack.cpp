#include "GameLayerStack.h"

namespace dorito {
  GameLayerStack::~GameLayerStack() {
    for (auto &state: m_States) {
      state->OnDetach();
    }
  }

  [[maybe_unused]] void GameLayerStack::PushState(const Ref<GameLayer> &state) {
    m_States.emplace(m_States.begin() + m_Index, state);
    m_Index++;
    state->OnAttach();
  }

  [[maybe_unused]] void GameLayerStack::PushOverlay(const Ref<GameLayer> &overlay) {
    m_States.emplace_back(overlay);
    overlay->OnAttach();
  }

  [[maybe_unused]] void GameLayerStack::PopState(const Ref<GameLayer> &state) {
    auto it = std::find(m_States.begin(), m_States.begin() + m_Index, state);
    if (it != m_States.begin() + m_Index) {
      state->OnDetach();
      m_States.erase(it);
      m_Index--;
    }
  }

  [[maybe_unused]] void GameLayerStack::PopOverlay(const Ref<GameLayer> &overlay) {
    auto it = std::find(m_States.begin() + m_Index, m_States.end(), overlay);
    if (it != m_States.end()) {
      overlay->OnDetach();
      m_States.erase(it);
    }
  }
} // joten