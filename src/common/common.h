#pragma once

#include <memory>

/*
 * Taken from Hazel Dev by Cherno. Thanks Yan.
 * https://hazelengine.com
 * https://github.com/TheCherno/Hazel
 */

namespace dorito {
  template<typename T>
  using Scope = std::unique_ptr<T>;

  template<typename T, typename ... Args>
  constexpr Scope<T> CreateScope(Args &&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }

  template<typename T>
  using Ref = std::shared_ptr<T>;

  template<typename T, typename ... Args>
  constexpr Ref<T> CreateRef(Args &&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }

  // Utilities
  uint8_t Bit(uint8_t data, uint8_t number);

  void SetBit(uint8_t &data, uint8_t number, bool isSet);

  bool IsBetween(uint16_t addr, uint16_t start_addr, uint16_t end_addr);
}
