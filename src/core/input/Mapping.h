#pragma once

#include <unordered_map>

#include "Keys.h"
#include "InputActions.h"

namespace dorito::Input {
  class Mapping {
  public:
    void Add(const Key &key, ActionType type) {
      m_Mapping.insert_or_assign(key, type);
    }

    void Set(const std::unordered_map<Key, ActionType> &map) {
      m_Mapping = map;
    }

    ActionType operator[](const Key &key) {
      if (m_Mapping.contains(key)) {
        return m_Mapping[key];
      }
      return ActionType::None;
    }

    std::unordered_map<Key, ActionType>::iterator begin() { return m_Mapping.begin(); }

    std::unordered_map<Key, ActionType>::iterator end() { return m_Mapping.end(); }

    [[nodiscard]] std::unordered_map<Key, ActionType>::const_iterator
    begin() const { return m_Mapping.begin(); }

    [[nodiscard]] std::unordered_map<Key, ActionType>::const_iterator
    end() const { return m_Mapping.end(); }

  private:
    std::unordered_map<Key, ActionType> m_Mapping;
  };
}