#include "Memory.h"

#include <fstream>

#include "core/events/EventManager.h"

namespace dorito {
  Memory::Memory() {
    Reset();
  }

  void Memory::LoadRom(const std::string &path) {
    Reset();

    std::ifstream stream(path.c_str(), std::ios::binary | std::ios::ate);

    if (!stream.good()) {
      // TODO: Do something here
      return;
    }

    auto position = stream.tellg();
    auto fileSize = static_cast<size_t>(position);

    std::vector<char> contents(fileSize);

    stream.seekg(0, std::ios::beg);
    stream.read(&contents[0], static_cast<std::streamsize>(position));
    stream.close();

    auto data = std::vector<uint8_t>(contents.begin(), contents.end());

    m_RomSize = data.size();

    uint16_t addr = 0x200;

    for (uint8_t b: data) {
      m_Ram[addr++] = b;
    }
  }


  void Memory::Reset() {
    memset(&m_Ram[0], 0, m_MemorySize);
    memset(&m_AudioBuffer[0], 0, 16);
    m_Stack.clear();

    m_UseBeep = true;

    LoadFont();
  }

  void Memory::LoadFont() {
    std::copy(m_StandardFont.begin(), m_StandardFont.end(), m_Ram.begin());
    std::copy(m_BigFont.begin(), m_BigFont.end(), m_Ram.begin() + m_StandardFont.size());
  }

  void Memory::Push(uint16_t addr) {
    m_Stack.push_front(addr);
  }

  uint16_t Memory::Pop() {
    if (m_Stack.empty()) {
      EventManager::Dispatcher().enqueue(Events::StackUnderflow{});
      return 0;
    }

    auto result = m_Stack.front();
    m_Stack.pop_front();

    return result;
  }

  void Memory::Write(uint16_t addr, uint8_t data) {
    if (addr < 0x200) {
      EventManager::Dispatcher().enqueue(Events::OutOfRangeMemAccess{addr});
      return;
    }

    m_Ram[addr] = data;
  }

  void Memory::WriteAudio(uint8_t position, uint8_t data) {
    m_AudioBuffer[position & 0xF] = data;
  }

  uint8_t Memory::Read(uint16_t addr) {
    return m_Ram[addr];
  }

  uint16_t Memory::CharacterAddress(uint8_t character) {
    uint16_t addr = (character & 0xF) * 5;
    return addr;
  }

  uint16_t Memory::BigCharacterAddress(uint8_t character) {
    uint16_t addr = m_StandardFont.size() + (10 * character);
    return addr;
  }

} // dorito