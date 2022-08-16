#include <spdlog/spdlog.h>
#include "Bus.h"

namespace dorito {
  Bus::Bus() {
    EventManager::Get().Attach<
        Events::LoadROM,
        &Bus::HandleLoadRom
    >(this);

    EventManager::Get().Attach<
        Events::StepCPU,
        &Bus::HandleStepCpu
    >(this);

    EventManager::Get().Attach<
        Events::ExecuteCPU,
        &Bus::HandleExecute
    >(this);

    EventManager::Get().Attach<
        Events::SetCycles,
        &Bus::HandleSetCycles
    >(this);

    EventManager::Get().Attach<
        Events::Reset,
        &Bus::HandleReset
    >(this);

    EventManager::Get().Attach<
        Events::UnloadROM,
        &Bus::HandleUnload
    >(this);

    EventManager::Get().Attach<
        Events::KeyDown,
        &Bus::HandleKeyDown
    >(this);

    EventManager::Get().Attach<
        Events::KeyUp,
        &Bus::HandleKeyUp
    >(this);

    EventManager::Get().Attach<
        Events::KeyReleased,
        &Bus::HandleKeyReleased
    >(this);

    EventManager::Get().Attach<
        Events::VIPCompat,
        &Bus::HandleVIPCompat
    >(this);

    EventManager::Get().Attach<
        Events::SCHIPCompat,
        &Bus::HandleSCHIPCompat
    >(this);

    EventManager::Get().Attach<
        Events::XOCompat,
        &Bus::HandleXOCompat
    >(this);

    EventManager::Get().Attach<
        Events::SetQuirk,
        &Bus::HandleSetQuirk
    >(this);

    m_Sound = LoadAudioStream(44100, 32, 1);
    SetAudioStreamCallback(m_Sound, &Bus::AudioCallback);
    AttachAudioStreamProcessor(m_Sound, &Bus::LowpassFilterCallback);
    SetAudioStreamVolume(m_Sound, 1.0f);
  }

  Bus::~Bus() {
    DetachAudioStreamProcessor(m_Sound, &Bus::LowpassFilterCallback);
    UnloadAudioStream(m_Sound);
    EventManager::Get().DetachAll(this);
  }

  uint8_t Bus::Read(uint16_t addr) {
    return m_Ram.Read(addr);
  }

  void Bus::Write(uint16_t addr, uint8_t data) {
    m_Ram.Write(addr, data);
  }

  void Bus::WriteAudio(uint8_t position, uint8_t data) {
    m_Ram.WriteAudio(position, data);
  }

  uint16_t Bus::Pop() {
    return m_Ram.Pop();
  }

  void Bus::Push(uint16_t addr) {
    m_Ram.Push(addr);
  }

  void Bus::Tick() {
    if (!m_Cpu.Halted()) {
      m_Cpu.Tick(m_CyclesPerFrame);

      if (m_Cpu.m_PitchDirty) {
        SetAudioStreamPitch(m_Sound, m_Cpu.regs.pitch / 4000.0f);
        m_Cpu.m_PitchDirty = false;
      }

      if (m_Cpu.regs.st > 0) {
        if (!IsAudioStreamPlaying(m_Sound)) {
          PlayAudioStream(m_Sound);
        }
      }

      if (m_Cpu.regs.st == 0 && IsAudioStreamPlaying(m_Sound)) {
        StopAudioStream(m_Sound);
      }
    }
  }

  void Bus::LoadRom(const std::string &path) {
    if (IsAudioStreamPlaying(m_Sound)) {
      StopAudioStream(m_Sound);
    }

    m_RomPath = path;

    m_Ram.LoadRom(path);
    m_Cpu.Reset();
    m_Display.Reset();
    SetCompatProfile(m_CompatProfile);

    m_UseBeepBuffer = true;

    m_Running = false;
  }

  void Bus::TickTimers() {
    m_Cpu.TickTimers();
  }

  void Bus::HandleStepCpu(const Events::StepCPU &) {
    TickTimers();
    m_Cpu.Step();
  }

  void Bus::HandleLoadRom(const Events::LoadROM &event) {
    LoadRom(event.path);
  }

  void Bus::HandleExecute(const Events::ExecuteCPU &event) {
    m_Cpu.Halted(!event.execute);
    m_Running = event.execute;
  }

  void Bus::HandleSetCycles(const Events::SetCycles &event) {
    m_CyclesPerFrame = event.cycles;
  }

  void Bus::HandleReset(const Events::Reset &) {
    m_Cpu.Reset();
    m_Display.Reset();
    m_Ram.Reset();

    if (!m_RomPath.empty()) {
      LoadRom(m_RomPath);
    }

    SetCompatProfile(m_CompatProfile);
    m_Running = false;
  }

  void Bus::HandleUnload(const Events::UnloadROM &) {
    if (IsAudioStreamPlaying(m_Sound)) {
      StopAudioStream(m_Sound);
    }

    m_Ram.Reset();
    m_Cpu.Reset();
    m_Display.Reset();
    SetCompatProfile(m_CompatProfile);
    m_Running = false;
  }

  void Bus::HandleKeyDown(const Events::KeyDown &event) {
    int8_t index = -1;

    switch (event.key) {
      case KEY_X:
        index = 0;
        break;

      case KEY_ONE:
        index = 1;
        break;

      case KEY_TWO:
        index = 2;
        break;

      case KEY_THREE:
        index = 3;
        break;

      case KEY_Q:
        index = 4;
        break;

      case KEY_W:
        index = 5;
        break;

      case KEY_E:
        index = 6;
        break;

      case KEY_A:
        index = 7;
        break;

      case KEY_S:
        index = 8;
        break;

      case KEY_D:
        index = 9;
        break;

      case KEY_Z:
        index = 0xA;
        break;

      case KEY_C:
        index = 0xB;
        break;

      case KEY_FOUR:
        index = 0xC;
        break;

      case KEY_R:
        index = 0xD;
        break;

      case KEY_F:
        index = 0xE;
        break;

      case KEY_V:
        index = 0xF;
        break;

      default:
        break;
    }

    if (index > -1) {
      m_Cpu.SetKeyState(index, true);
    }
  }

  void Bus::HandleKeyUp(const Events::KeyUp &event) {
    int8_t index = -1;

    switch (event.key) {
      case KEY_X:
        index = 0;
        break;

      case KEY_ONE:
        index = 1;
        break;

      case KEY_TWO:
        index = 2;
        break;

      case KEY_THREE:
        index = 3;
        break;

      case KEY_Q:
        index = 4;
        break;

      case KEY_W:
        index = 5;
        break;

      case KEY_E:
        index = 6;
        break;

      case KEY_A:
        index = 7;
        break;

      case KEY_S:
        index = 8;
        break;

      case KEY_D:
        index = 9;
        break;

      case KEY_Z:
        index = 0xA;
        break;

      case KEY_C:
        index = 0xB;
        break;

      case KEY_FOUR:
        index = 0xC;
        break;

      case KEY_R:
        index = 0xD;
        break;

      case KEY_F:
        index = 0xE;
        break;

      case KEY_V:
        index = 0xF;
        break;

      default:
        break;
    }

    if (index > -1) {
      m_Cpu.SetKeyState(index, false);
    }
  }

  void Bus::HandleKeyReleased(const Events::KeyReleased &event) {
    int8_t index = -1;

    switch (event.key) {
      case KEY_X:
        index = 0;
        break;

      case KEY_ONE:
        index = 1;
        break;

      case KEY_TWO:
        index = 2;
        break;

      case KEY_THREE:
        index = 3;
        break;

      case KEY_Q:
        index = 4;
        break;

      case KEY_W:
        index = 5;
        break;

      case KEY_E:
        index = 6;
        break;

      case KEY_A:
        index = 7;
        break;

      case KEY_S:
        index = 8;
        break;

      case KEY_D:
        index = 9;
        break;

      case KEY_Z:
        index = 0xA;
        break;

      case KEY_C:
        index = 0xB;
        break;

      case KEY_FOUR:
        index = 0xC;
        break;

      case KEY_R:
        index = 0xD;
        break;

      case KEY_F:
        index = 0xE;
        break;

      case KEY_V:
        index = 0xF;
        break;

      default:
        break;
    }

    if (index > -1) {
      m_Cpu.KeyPressed(index);
    }
  }

  void Bus::SetCompatProfile(const Bus::CompatProfile &profile) {
    m_CompatProfile = profile;

    m_Cpu.SetQuirks(
        {
            Chip8::Quirk::Clip,
            Chip8::Quirk::Logic,
            Chip8::Quirk::Jump,
            Chip8::Quirk::LoadStore,
            Chip8::Quirk::Shift,
            Chip8::Quirk::LoresSprites,
            Chip8::Quirk::VBlank,
            Chip8::Quirk::IRegCarry
        }, false);

    switch (profile) {
      case CompatProfile::VIP:
        m_Cpu.SetQuirks(
            {
                Chip8::Quirk::Logic,
                Chip8::Quirk::Clip,
                Chip8::Quirk::VBlank
            }, true);
        break;
      case CompatProfile::SCHIP:
        m_Cpu.SetQuirks(
            {
                Chip8::Quirk::Shift,
                Chip8::Quirk::LoadStore,
                Chip8::Quirk::Jump,
                Chip8::Quirk::Clip
            }, true);
        break;

      default:
        break;
    }
  }

  void Bus::HandleVIPCompat(const Events::VIPCompat &) {
    SetCompatProfile(CompatProfile::VIP);
  }

  void Bus::HandleSCHIPCompat(const Events::SCHIPCompat &) {
    SetCompatProfile(CompatProfile::SCHIP);
  }

  void Bus::HandleXOCompat(const Events::XOCompat &) {
    SetCompatProfile(CompatProfile::XOChip);
  }

  void Bus::SetQuirk(Chip8::Quirk quirk, bool isSet) {
    m_Cpu.SetQuirks({quirk}, isSet);
  }

  void Bus::HandleSetQuirk(const Events::SetQuirk &event) {
    SetQuirk(event.quirk, event.value);
  }

  void Bus::AudioCallback(void *buffer, uint32_t frames) {
    static uint32_t cursor = 0;
    std::vector<float> bits;

    float *output = (float *) buffer;

    auto &bus = Bus::Get();
    auto &pattern = bus.m_Ram.GetAudioBuffer();

    /* Stolen from Timendus' excellent silicon8.
    * https://github.com/Timendus/silicon8/blob/ec8dc770a0305d3782881cdc8bb4eed5c954bca0/web-client/sound.js
    * Extend the pattern to an array of individual bits, quadrupling each bit to
    * get a nicer square wave.
    */
    auto patternToBits = [](const std::vector<uint8_t> &pattern) {
      std::vector<float> bits = std::vector<float>(512);
      auto i = 0;

      for (const uint8_t byte: pattern) {
        uint8_t mask = 128;

        while (mask != 0) {
          const uint8_t val = (byte & mask) != 0 ? 1 : 0;
          bits[i++] = val;
          bits[i++] = val;
          bits[i++] = val;
          bits[i++] = val;
          mask >>= 1;
        }
      }

      return bits;
    };

    bits = patternToBits(pattern);


    for (uint32_t i = 0; i < frames; i++) {
      output[i] = bits[cursor++];
      cursor %= 512;
    }
  }

  void Bus::LowpassFilterCallback(void *buffer, uint32_t frames) {
    static float low[2] = {0.0f, 0.0f};
    const float pi = 3.14159265358979323846f;
    static const float cutoff = 18000.0f / 44100.0f;
    float c = std::cosf(2 * pi * cutoff);
    const float k = c - 1 + std::sqrtf(c * c - 4 * c + 3);

    for (unsigned int i = 0; i < frames * 2; i += 2) {
      float l = ((float *) buffer)[i], r = ((float *) buffer)[i + 1];
      low[0] += k * (l - low[0]);
      low[1] += k * (r - low[1]);
      ((float *) buffer)[i] = low[0];
      ((float *) buffer)[i + 1] = low[1];
    }
  }

} // dorito