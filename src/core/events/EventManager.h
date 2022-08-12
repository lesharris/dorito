#pragma once

#include <entt/entt.hpp>

#include "Event.h"
#include "Events.h"

namespace dorito {
  class EventManager {
  public:
    static EventManager &Get() {
      static EventManager instance;

      return instance;
    }

    static entt::dispatcher &Dispatcher() { return Get().m_Dispatcher; }

    static entt::dispatcher &GuiDispatcher() { return Get().m_GuiDispatcher; }

    template<typename EventStruct, auto Handler, class Listener>
    constexpr void Attach(Listener *const inListener, bool gui = false) {
      if (!gui) {
        auto sink = m_Dispatcher.template sink<EventStruct>();
        sink.template connect<Handler>(*inListener);
      } else {
        auto sink = m_GuiDispatcher.template sink<EventStruct>();
        sink.template connect<Handler>(*inListener);
      }
    }

    template<class Listener, typename... Events>
    constexpr void DetachAll(Listener *const inListener, bool gui = false) {
      if (!gui) {
        ((m_Dispatcher.template sink<Events>().
            template disconnect(*inListener)), ...);
      } else {
        ((m_GuiDispatcher.template sink<Events>().
            template disconnect(*inListener)), ...);
      }
    }

  private:
    EventManager() = default;

    ~EventManager() = default;

  private:
    entt::dispatcher m_Dispatcher{};
    entt::dispatcher m_GuiDispatcher{};
  };
}