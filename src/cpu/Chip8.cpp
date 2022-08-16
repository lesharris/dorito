#include "Chip8.h"

#include <regex>
#include <fstream>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "system/Bus.h"

namespace dorito {
  Chip8::Chip8() {
    Reset();
    m_Instructions = {
        {
            0x0,
            {0x0,
                "0NNN", "nop",
                1, {OperandType::Number12bit},
                &Chip8::ProcUnimplemented
            }
        },
        {
            0xC0,
            {0xC0,
                "00CN", "scroll-down N",
                1, {OperandType::Number4bit},
                &Chip8::ProcScrollDown
            }
        },
        {
            0xD0,
            {0xD0,
                "00DN", "scroll-up N",
                1, {OperandType::Number4bit},
                &Chip8::ProcScrollUp
            }
        },
        {
            0xE0,
            {0xE0,
                "00E0", "clear",
                0, {},
                &Chip8::ProcClearScreen
            }
        },
        {
            0xEE,
            {0xEE,
                "00EE", "return",
                0, {},
                &Chip8::ProcReturn
            }
        },
        {
            0xFB,
            {0xFB,
                "00FB", "scroll-right",
                0, {},
                &Chip8::ProcScrollRight
            }
        },
        {
            0xFC,
            {0xFC,
                "00FC", "scroll-left",
                0, {},
                &Chip8::ProcScrollLeft
            }
        },
        {
            0xFD,
            {0xFD,
                "00FD", "exit",
                0, {},
                &Chip8::ProcExit
            }
        },
        {
            0xFE,
            {0xFE,
                "00FE", "lores",
                0, {},
                &Chip8::ProcLores
            }
        },
        {
            0xFF,
            {0xFF,
                "00FF", "hires",
                0, {},
                &Chip8::ProcHires
            }
        },
        {
            0x1000,
            {0x1000,
                "1NNN", "jump NNN",
                1, {OperandType::Number12bit},
                &Chip8::ProcJump
            }
        },
        {
            0x2000,
            {0x2000,
                "2NNN", "call NNN",
                1, {OperandType::Number12bit},
                &Chip8::ProcCall
            }
        },
        {
            0x3000,
            {0x3000,
                "3XNN", "skip vX == NN",
                2, {OperandType::Register, OperandType::Number8bit},
                &Chip8::ProcSkipEqualsLiteral
            }
        },
        {
            0x4000,
            {0x4000,
                "4XNN", "skip vX != NN",
                2, {OperandType::Register, OperandType::Number8bit},
                &Chip8::ProcSkipNotEqualsLiteral
            }
        },
        {
            0x5000,
            {0x5000,
                "5XY0", "skip vX == vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcSkipRegEqualsReg
            }
        },
        {
            0x5002,
            {0x5002,
                "5XY2", "save vX - vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcRegisterSave
            }
        },
        {
            0x5003,
            {0x5003,
                "5XY3", "load vX - vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcRegisterLoad
            }
        },
        {
            0x6000,
            {0x6000,
                "6XNN", "vX := NN",

                2, {OperandType::Register, OperandType::Number8bit},
                &Chip8::ProcLoadLiteral
            }
        },
        {
            0x7000,
            {0x7000,
                "7XNN", "vX += NN",
                2, {OperandType::Register, OperandType::Number8bit},
                &Chip8::ProcAddLiteral
            }
        },
        {
            0x8000,
            {0x8000,
                "8XY0", "vX := vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcLoadRegister
            }
        },
        {
            0x8001,
            {0x8001,
                "8XY1", "vX |= vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcORReg
            }
        },
        {
            0x8002,
            {0x8002,
                "8XY2", "vX &= vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcANDReg
            }
        },
        {
            0x8003,
            {0x8003,
                "8XY3", "vX ^= vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcXORReg
            }
        },
        {
            0x8004,
            {0x8004,
                "8XY4", "vX += vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcAdd
            }
        },
        {
            0x8005,
            {0x8005,
                "8XY5", "vX -= vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcSubtractYFromX
            }
        },
        {
            0x8006,
            {0x8006,
                "8XY6", "vX := vY >> 1",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcShiftRight
            }
        },
        {
            0x8007,
            {0x8007,
                "8XY7", "vX =- vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcSubtractXFromY
            }
        },
        {
            0x800E,
            {0x800E,
                "8XYE", "vX := vY << 1",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcShiftLeft
            }
        },
        {
            0x9000,
            {0x9000,
                "9XY0", "skip vX != vY",
                2, {OperandType::Register, OperandType::Register},
                &Chip8::ProcSkipRegNotEqualReg
            }
        },
        {
            0xA000,
            {0xA000,
                "ANNN", "i := NNN",
                1, {OperandType::Number12bit},
                &Chip8::ProcLoadILiteral
            }
        },
        {
            0xB000,
            {0xB000,
                "BNNN", "jump v0 + NNN",
                1, {OperandType::Number12bit},
                &Chip8::ProcJumpRelative
            }
        },
        {
            0xC000,
            {0xC000,
                "CXNN", "vX := random & NN",
                2, {OperandType::Register, OperandType::Number8bit},
                &Chip8::ProcRandom
            }
        },
        {
            0xD000,
            {0xD000,
                "DXYN", "sprite vX vY N",
                3, {OperandType::Register, OperandType::Register, OperandType::Number4bit},
                &Chip8::ProcDrawSprite
            }
        },
        {
            0xE09E,
            {0xE09E,
                "EX9E", "skip vX == key pressed",
                1, {OperandType::Register},
                &Chip8::ProcSkipKeyPressed
            }
        },
        {
            0xE0A1,
            {0xE0A1,
                "EXA1", "skip vX != key pressed",
                1, {OperandType::Register},
                &Chip8::ProcSkipKeyNotPressed
            }
        },
        {
            0xF000,
            {0xF000,
                "F000", "i := long NNNN",
                1, {OperandType::Number16bit},
                &Chip8::ProcLoadIExtended
            }
        },
        {
            0xF001,
            {0xF001,
                "FN01", "plane N",
                1, {OperandType::Number4bit},
                &Chip8::ProcPlane
            }
        },
        {
            0xF002,
            {0xF002,
                "F002", "audio",
                0, {},
                &Chip8::ProcAudio
            }
        },
        {
            0xF007,
            {0xF007,
                "FX07", "vX := delay",
                1, {OperandType::Register},
                &Chip8::ProcLoadDelayToReg
            }
        },
        {
            0xF00A,
            {0xF00A,
                "FX0A", "vX := key",
                1, {OperandType::Register},
                &Chip8::ProcLoadKeypressToReg
            }
        },
        {
            0xF015,
            {0xF015,
                "FX15", "delay := vX",
                1, {OperandType::Register},
                &Chip8::ProcLoadRegToDelay
            }
        },
        {
            0xF018,
            {0xF018,
                "FX18", "buzzer := vX",
                1, {OperandType::Register},
                &Chip8::ProcLoadRegToBuzzer
            }
        },
        {
            0xF01E,
            {0xF01E,
                "FX1E", "i += vX",
                1, {OperandType::Register},
                &Chip8::ProcAddIWithReg
            }
        },
        {
            0xF029,
            {0xF029,
                "FX29", "i := hex vX",
                1, {OperandType::Register},
                &Chip8::ProcLoadISpriteAddr
            }
        },
        {
            0xF030,
            {0xF030,
                "FX30", "i := bighex vX",
                1, {OperandType::Register},
                &Chip8::ProcLoadIBigSpriteAddr
            }
        },
        {
            0xF033,
            {0xF033,
                "FX33", "bcd vX",
                1, {OperandType::Register},
                &Chip8::ProcLoadBCD
            }
        },
        {
            0xF03A,
            {0xF03A,
                "FX3A", "pitch := vX",
                1, {OperandType::Register},
                &Chip8::ProcPitch
            }
        },
        {
            0xF055,
            {0xF055,
                "FX55", "save vX",
                1, {OperandType::Register},
                &Chip8::ProcRegisterSaveIncrement
            }
        },
        {
            0xF065,
            {0xF065,
                "FX65", "load vX",
                1, {OperandType::Register},
                &Chip8::ProcRegisterLoadIncrement
            }
        },
        {
            0xF075,
            {0xF075,
                "FX75", "saveflags vX",
                1, {OperandType::Register},
                &Chip8::ProcSaveFlags
            }
        },
        {
            0xF085,
            {0xF085,
                "FX85", "loadflags vX",
                1, {OperandType::Register},
                &Chip8::ProcLoadFlags
            }
        },
        {
            0xFFFF,
            {0xFFFF,
                "FFFF", "invalid",
                0, {},
                &Chip8::ProcUnimplemented
            }
        }
    };
  }

  void Chip8::Reset() {
    regs.pc = 0x200;
    regs.i = 0;
    regs.st = 0;
    regs.dt = 0;
    regs.latch = 0;
    regs.pitch = 4000.0;

    memset(regs.v, 0, 16);
    memset(regs.keys, false, 16);
    memset(regs.quirks, false, 8);
    memset(mOperands, 0, sizeof(mOperands));

    m_Cycles = 0;
    m_Halted = true;
    m_Waiting = false;
    m_HighRes = false;
    m_KeyPressRegister = 0;

    m_CurrentInstruction = nullptr;

    m_WaitForInterrupt = 0;

    m_Disassembly.clear();
    m_DisasmCount = 0;
  }

  void Chip8::Tick(uint32_t cycles) {
    if (m_Halted || m_Waiting)
      return;

    m_Cycles += cycles;

    for (uint32_t c = 0; c < cycles; c++) {
      Step();
    }

    if (m_WaitForInterrupt == 1) {
      m_WaitForInterrupt = 2;
    }
  }

  void Chip8::TickTimers() {
    if (regs.st > 0)
      regs.st--;

    if (regs.dt > 0)
      regs.dt--;
  }

  void Chip8::Step() {
    if (m_Waiting)
      return;

    m_PrevPC = regs.pc;

    DisassembleNext();
    Fetch();

    // Execute
    if (m_CurrentInstruction) {
      // Lol this syntax is toxic
      (this->*(m_CurrentInstruction->proc))();
    }
  }

  void Chip8::Fetch() {
    auto &bus = Bus::Get();

    uint8_t high = bus.Read(regs.pc++);
    uint8_t low = bus.Read(regs.pc++);

    regs.latch = (high << 8) | low;

    m_CurrentInstruction = Decode(regs.latch);

    ExtractOperands(regs.latch, m_CurrentInstruction, true);
  }

  void Chip8::ExtractOperands(uint16_t opcode, Chip8::Instruction *current, bool advancePC) {
    auto &bus = Bus::Get();

    uint16_t operand = opcode & 0x0FFF;

    auto currentNibble = [](uint16_t operand, uint8_t position) {
      return (operand & (0xF << (position * 4))) >> (position * 4);
    };

    for (uint8_t i = 0; i < current->operand_count; i++) {

      /* TODO: Store operand position in instruction so I
       * don't have to special case these things
       */

      if (current->code == 0x00C0 ||
          current->code == 0x00D0) {
        mOperands[0].type = OperandType::Number4bit;
        mOperands[0].value = opcode & 0x000F;
        break;
      }

      if (current->code == 0xF001) {
        mOperands[0].type = OperandType::Number4bit;
        mOperands[0].value = (opcode & 0x0F00) >> 8;
        break;
      }

      auto type = current->operand_order[i];
      auto currentNibblePosition = 2 - i;

      mOperands[i].type = type;

      switch (type) {
        case OperandType::Register:
        case OperandType::Number4bit:
          mOperands[i].value = currentNibble(operand, currentNibblePosition);
          break;

        case OperandType::Number8bit: {
          auto highNibble = currentNibble(operand, currentNibblePosition);
          currentNibblePosition--;
          auto lowNibble = currentNibble(operand, currentNibblePosition);

          mOperands[i].value = (highNibble << 4) | lowNibble;
        }
          break;

        case OperandType::Number12bit:
          mOperands[i].value = operand;
          break;

        case OperandType::Number16bit: {
          uint8_t addrHigh = advancePC ? bus.Read(regs.pc++) : bus.Read(regs.pc + 2);
          uint8_t addrLow = advancePC ? bus.Read(regs.pc++) : bus.Read(regs.pc + 3);

          uint16_t addr = (addrHigh << 8) | addrLow;

          mOperands[i].value = addr;
        }
          break;
      }
    }
  }

  Chip8::Instruction *Chip8::Decode(uint16_t code, bool silent) {
    /* I never thought an ISA would make me miss
     * addressing modes. Yet here we are.
     */
    switch ((code & 0xF000) >> 12) {
      case 0x0:
        switch (code & 0xFF) {
          case 0xE0:
          case 0xEE:
          case 0xFB:
          case 0xFC:
          case 0xFD:
          case 0xFE:
          case 0xFF:
            return &m_Instructions[code & 0xFF];

          default:
            switch (code & 0xF0) {
              case 0xC0:
              case 0xD0:
                return &m_Instructions[code & 0xF0];
            }

            if (!silent)
              spdlog::get("console")->warn("Invalid opcode at 0x{:04X}: 0x{:04X}", regs.pc - 2, code);
            return &m_Instructions[0xFFFF];
        }

      case 0x1:
      case 0x2:
      case 0x3:
      case 0x4:
      case 0x6:
      case 0x7:
      case 0x9:
      case 0xA:
      case 0xB:
      case 0xC:
      case 0xD:
        return &m_Instructions[code & 0xF000];

      case 0x5:
      case 0x8:
        switch (code & 0xF00F) {
          case 0x5000:
          case 0x5002:
          case 0x5003:
          case 0x8000:
          case 0x8001:
          case 0x8002:
          case 0x8003:
          case 0x8004:
          case 0x8005:
          case 0x8006:
          case 0x8007:
          case 0x800E:
            return &m_Instructions[code & 0xF00F];

          default:
            if (!silent)
              spdlog::get("console")->warn("Invalid opcode at 0x{:04X}: 0x{:04X}", regs.pc - 2, regs.latch);
            return &m_Instructions[0xFFFF];
        }

      case 0xE:
        switch (code & 0xF0FF) {
          case 0xE09E:
          case 0xE0A1:
            return &m_Instructions[code & 0xF0FF];

          default:
            if (!silent)
              spdlog::get("console")->warn("Invalid opcode at 0x{:04X}: 0x{:04X}", regs.pc - 2, regs.latch);
            return &m_Instructions[0xFFFF];
        }

      case 0xF:
        switch (code & 0xF0FF) {
          case 0xF000:
          case 0xF001:
          case 0xF002:
          case 0xF007:
          case 0xF00A:
          case 0xF015:
          case 0xF018:
          case 0xF01E:
          case 0xF029:
          case 0xF030:
          case 0xF033:
          case 0xF03A:
          case 0xF055:
          case 0xF065:
          case 0xF075:
          case 0xF085:
            return &m_Instructions[code & 0xF0FF];

          default:
            if (!silent)
              spdlog::get("console")->warn("Invalid opcode at 0x{:04X}: 0x{:04X}", regs.pc - 2, regs.latch);
            return &m_Instructions[0xFFFF];
        }
    }

    // Shouldn't get here, just for completeness.
    if (!silent)
      spdlog::get("console")->warn("Invalid opcode at 0x{:04X}: 0x{:04X}", regs.pc - 2, regs.latch);
    return &m_Instructions[0xFFFF];
  }

  void Chip8::DisassembleNext() {
    auto &bus = Bus::Get();

    uint8_t high = bus.Read(regs.pc);
    uint8_t low = bus.Read(regs.pc + 1);
    uint8_t nextHigh = bus.Read(regs.pc + 2);
    uint8_t nextLow = bus.Read(regs.pc + 3);

    uint16_t opcode = (high << 8) | low;

    auto current = Decode(opcode, true);
    ExtractOperands(opcode, current, false);

    bool isExtended = current->code == 0xF000;

    if (!m_Disassembly.contains(regs.pc)) {
      bool firstReg = true;
      std::string code = current->label;

      // Populate Operands
      for (uint8_t i = 0; i < current->operand_count; i++) {
        std::string pattern;

        switch (mOperands[i].type) {
          case OperandType::Register:
            if (firstReg) {
              pattern = "X";
              firstReg = false;
            } else {
              pattern = "Y";
            }
            break;
          case OperandType::Number4bit:
            pattern = "N";
            break;
          case OperandType::Number8bit:
            pattern = "NN";
            break;
          case OperandType::Number12bit:
            pattern = "NNN";
            break;
          case OperandType::Number16bit:
            pattern = "NNNN";
            break;
        }

        std::regex re{pattern};
        std::smatch m;

        if (std::regex_search(code, m, re)) {
          std::string formatted;

          switch (mOperands[i].type) {
            case OperandType::Register:
            case OperandType::Number4bit:
              formatted = fmt::format("{:1X}", mOperands[i].value);
              break;

            case OperandType::Number8bit:
              formatted = fmt::format("0x{:02X} ({})", mOperands[i].value, mOperands[i].value);
              break;

            case OperandType::Number12bit:
              formatted = fmt::format("0x{:03X} ({})", mOperands[i].value, mOperands[i].value);
              break;

            case OperandType::Number16bit:
              formatted = fmt::format("0x{:4X} ({})", mOperands[i].value, mOperands[i].value);
              break;
          }

          code.replace(m[0].first, m[0].second, formatted);
        }
      }

      std::string bytes = isExtended
                          ? fmt::format("{:02X} {:02X} {:02X} {:02X}",
                                        high, low, nextHigh, nextLow)
                          : fmt::format("{:02X} {:02X}", high, low);

      m_Disassembly[regs.pc] = {
          static_cast<uint16_t>(regs.pc),
          m_DisasmCount++,
          code,
          bytes
      };

      uint8_t index = 0;
      for (auto &[_, line]: m_Disassembly) {
        line.index = index++;
      }

    }
  }

  void Chip8::SetFlag(uint8_t dest, uint16_t value, bool isSet) {
    regs.v[dest] = (value & 0xFF);
    regs.v[0xF] = isSet ? 1 : 0;
  }

  void Chip8::Skip() {
    auto &bus = Bus::Get();

    uint8_t high = bus.Read(regs.pc);
    uint8_t low = bus.Read(regs.pc + 1);

    uint16_t nextOp = (high << 8) | low;

    regs.pc += (nextOp == 0xF000) ? 4 : 2;
  }

  /* CPU Instruction Procedures */

  /* 0NNN */
  void Chip8::ProcUnimplemented() {}

  /* 00CN */
  void Chip8::ProcScrollDown() {
    auto &bus = Bus::Get();

    bus.ScrollDown(mOperands[0].value);
  }

  void Chip8::ProcScrollUp() {
    auto &bus = Bus::Get();

    bus.ScrollUp(mOperands[0].value);
  }

  /* 00E0 */
  void Chip8::ProcClearScreen() {
    auto &bus = Bus::Get();
    bus.ClearScreen();
  }

  /* 00EE */
  void Chip8::ProcReturn() {
    auto &bus = Bus::Get();
    regs.pc = bus.Pop();
  }

  /* 00FB */
  void Chip8::ProcScrollRight() {
    auto &bus = Bus::Get();

    bus.ScrollRight();
  }

  /* 00FC */
  void Chip8::ProcScrollLeft() {
    auto &bus = Bus::Get();

    bus.ScrollLeft();
  }

  /* 00FD */
  void Chip8::ProcExit() {
    m_Halted = true;
  }

  /* 00FE */
  void Chip8::ProcLores() {
    auto &bus = Bus::Get();
    m_HighRes = false;
    bus.SetHighRes(false);
  }

  /* 00FF */
  void Chip8::ProcHires() {
    auto &bus = Bus::Get();
    m_HighRes = true;
    bus.SetHighRes(true);
  }

  /* 1NNN */
  void Chip8::ProcJump() {
    regs.pc = mOperands[0].value;
  }

  /* 2NNN */
  void Chip8::ProcCall() {
    auto &bus = Bus::Get();
    bus.Push(regs.pc);
    regs.pc = mOperands[0].value;
  }

  /* 3XNN */
  void Chip8::ProcSkipEqualsLiteral() {
    if (regs.v[mOperands[0].value] == mOperands[1].value) {
      Skip();
    }
  }

  /* 4XNN */
  void Chip8::ProcSkipNotEqualsLiteral() {
    if (regs.v[mOperands[0].value] != mOperands[1].value) {
      Skip();
    }
  }

  /* 5XY0 */
  void Chip8::ProcSkipRegEqualsReg() {
    if (regs.v[mOperands[0].value] == regs.v[mOperands[1].value]) {
      Skip();
    }
  }

  /* 5XY2 */
  void Chip8::ProcRegisterSave() {
    auto &bus = Bus::Get();

    uint8_t x = mOperands[0].value;
    uint8_t y = mOperands[1].value;

    int dist = std::abs(x - y);

    if (x < y) {
      for (auto z = 0; z <= dist; z++) {
        bus.Write(regs.i + z, regs.v[x + z]);
      }
    } else {
      for (auto z = 0; z <= dist; z++) {
        bus.Write(regs.i + z, regs.v[x - z]);
      }
    }
  }

  /* 5XY3 */
  void Chip8::ProcRegisterLoad() {
    auto &bus = Bus::Get();

    uint8_t x = mOperands[0].value;
    uint8_t y = mOperands[1].value;

    int dist = std::abs(x - y);

    if (x < y) {
      for (auto z = 0; z <= dist; z++) {
        regs.v[x + z] = bus.Read(regs.i + z);
      }
    } else {
      for (auto z = 0; z <= dist; z++) {
        regs.v[x - z] = bus.Read(regs.i + z);
      }
    }
  }

  /* 6XNN */
  void Chip8::ProcLoadLiteral() {
    regs.v[mOperands[0].value] = mOperands[1].value;
  }

  /* 7XNN */
  void Chip8::ProcAddLiteral() {
    uint16_t sum = regs.v[mOperands[0].value] + mOperands[1].value;
    regs.v[mOperands[0].value] = sum & 0xFF;
  }

  /* 8XY0 */
  void Chip8::ProcLoadRegister() {
    regs.v[mOperands[0].value] = regs.v[mOperands[1].value];
  }

  /* 8XY1 */
  void Chip8::ProcORReg() {
    regs.v[mOperands[0].value] |= regs.v[mOperands[1].value];

    if (QuirkSet(Quirk::Logic)) {
      regs.v[0xF] = 0;
    }
  }

  /* 8XY2 */
  void Chip8::ProcANDReg() {
    regs.v[mOperands[0].value] &= regs.v[mOperands[1].value];

    if (QuirkSet(Quirk::Logic)) {
      regs.v[0xF] = 0;
    }
  }

  /* 8XY3 */
  void Chip8::ProcXORReg() {
    regs.v[mOperands[0].value] ^= regs.v[mOperands[1].value];

    if (QuirkSet(Quirk::Logic)) {
      regs.v[0xF] = 0;
    }
  }

  /* 8XY4 */
  void Chip8::ProcAdd() {
    uint16_t sum = regs.v[mOperands[0].value] + regs.v[mOperands[1].value];

    SetFlag(mOperands[0].value, sum, sum > 0xFF);
  }

  /* 8XY5 */
  void Chip8::ProcSubtractYFromX() {
    uint8_t difference = regs.v[mOperands[0].value] - regs.v[mOperands[1].value];

    SetFlag(mOperands[0].value,
            difference,
            (regs.v[mOperands[0].value] >= regs.v[mOperands[1].value]));
  }

  /* 8XY6 */
  void Chip8::ProcShiftRight() {
    auto x = regs.v[mOperands[0].value];
    auto y = regs.v[mOperands[1].value];

    if (QuirkSet(Quirk::Shift)) {
      y = x;
    }

    uint16_t value = y >> 1;
    SetFlag(mOperands[0].value, value, regs.v[mOperands[1].value] & 1);
  }

  /* 8XY7 */
  void Chip8::ProcSubtractXFromY() {
    uint16_t difference = regs.v[mOperands[1].value] - regs.v[mOperands[0].value];

    SetFlag(mOperands[0].value,
            difference,
            (regs.v[mOperands[1].value] >= regs.v[mOperands[0].value]));
  }

  /* 8XYE */
  void Chip8::ProcShiftLeft() {
    auto x = regs.v[mOperands[0].value];
    auto y = regs.v[mOperands[1].value];

    if (QuirkSet(Quirk::Shift)) {
      y = x;
    }

    uint16_t value = y << 1;
    SetFlag(mOperands[0].value, value, (regs.v[mOperands[1].value] >> 7) & 0x1);
  }

  /* 9XY0 */
  void Chip8::ProcSkipRegNotEqualReg() {
    if (regs.v[mOperands[0].value] != regs.v[mOperands[1].value]) {
      Skip();
    }
  }

  /* ANNN */
  void Chip8::ProcLoadILiteral() {
    regs.i = mOperands[0].value;
  }

  /* BNNN */
  void Chip8::ProcJumpRelative() {
    if (QuirkSet(Quirk::Jump))
      regs.pc = mOperands[0].value + regs.v[(mOperands[0].value & 0xF00) >> 8];
    else
      regs.pc = regs.v[0] + mOperands[0].value;
  }

  /* CXNN */
  void Chip8::ProcRandom() {
    std::uniform_int_distribution<int> dist(0, 255);

    regs.v[mOperands[0].value] = dist(mt) & mOperands[1].value;
  }

  /* DXYN */
  void Chip8::ProcDrawSprite() {
    if (WaitForInterrupt()) {
      return;
    }

    auto &bus = Bus::Get();

    uint8_t x = regs.v[mOperands[0].value];
    uint8_t y = regs.v[mOperands[1].value];
    uint8_t height = mOperands[2].value;
    uint16_t i = regs.i;

    uint8_t planeMask = bus.GetPlane();

    uint8_t dispWidth = bus.DisplayWidth();
    uint8_t dispHeight = bus.DisplayHeight();

    uint8_t spriteWidth = height == 0 ? 16 : 8;
    uint8_t spriteHeight = height == 0 ? 16 : height;

    if (QuirkSet(Quirk::LoresSprites)) {
      spriteWidth = 8;
    }

    /* If initial coordinate of sprite is outside the screen bounds
     * we modulo it back into bounds.
     * https://laurencescotford.com/chip-8-on-the-cosmac-vip-drawing-sprites/
     * https://github.com/Timendus/chip8-test-suite
     */
    if (y >= dispHeight)
      y %= dispHeight;

    if (x >= dispWidth)
      x %= dispWidth;

    regs.v[0xF] = 0;

    uint8_t collided = 0;

    for (auto layer = 0; layer < 2; layer++) {

      if ((planeMask & (layer + 1)) == 0)
        continue;

      for (uint8_t n = 0; n < spriteHeight; n++) {
        uint16_t line = height == 0
                        ? bus.Read((2 * n) + i) << 8 | bus.Read((2 * n) + i + 1)
                        : bus.Read(i + n);


        /* Special Sprite height 0 handling when not in hires
         * makes Hap's test rom work. Not sure if this ever shows up
         * anywhere else. Made it a quirk.
         */

        if (QuirkSet(Quirk::LoresSprites)) {
          if (height == 0 && !m_HighRes)
            line = bus.Read(i + n) << 8 | bus.Read(i + n + 1);
        }

        for (auto b = 0; b < spriteWidth; b++) {
          uint8_t bit = height == 0
                        ? 15 - b
                        : 7 - b;

          uint8_t pixel = (line & (1 << bit)) >> bit;

          if (QuirkSet(Quirk::Clip) &&
              (x + b >= dispWidth || y + n >= dispHeight)) {
            pixel = 0;
          }

          if (!pixel)
            continue;

          if (bus.Plot(layer, x + b, y + n)) {
            collided = 1;
          }
        }
      }

      i += (height == 0 ? 32 : height);
    }

    regs.v[0xF] = collided;
  }

  /* EX9E */
  void Chip8::ProcSkipKeyPressed() {
    uint8_t key = regs.v[mOperands[0].value] & 0xF;

    if (regs.keys[key]) {
      Skip();
    }
  }

  /* EXA1 */
  void Chip8::ProcSkipKeyNotPressed() {
    uint8_t key = regs.v[mOperands[0].value] & 0xF;

    if (!regs.keys[key]) {
      Skip();
    }
  }

  /* F000 */
  void Chip8::ProcLoadIExtended() {
    regs.i = mOperands[0].value;
  }

  /* FN01 */
  void Chip8::ProcPlane() {
    auto &bus = Bus::Get();

    bus.SetPlane(mOperands[0].value);
  }

  /* F002 */
  void Chip8::ProcAudio() {
    auto &bus = Bus::Get();

    for (auto n = regs.i; n < regs.i + 16; n++) {
      bus.WriteAudio(n - regs.i, bus.Read(n));
    }

    bus.UseBeepBuffer(false);
  }

  /* FX07 */
  void Chip8::ProcLoadDelayToReg() {
    regs.v[mOperands[0].value] = regs.dt;
  }

  /* FX0A */
  void Chip8::ProcLoadKeypressToReg() {
    m_KeyPressRegister = mOperands[0].value;
    m_Waiting = true;
  }

  /* FX15 */
  void Chip8::ProcLoadRegToDelay() {
    regs.dt = regs.v[mOperands[0].value];
  }

  /* FX18 */
  void Chip8::ProcLoadRegToBuzzer() {
    regs.st = regs.v[mOperands[0].value];
  }

  /* FX1E */
  void Chip8::ProcAddIWithReg() {
    /* This quirk makes Tronix's SC Chip test rom pass all tests.
     * Tronix offers the following explanation:
     * ERROR 24
     *  Check FX1E (I = I + VX) buffer overflow. If buffer overflow, register
     *  VF must be set to 1, otherwise 0. As a result, register VF not set to 1.
     *  This undocumented feature of the Chip-8 and used by Spacefight 2019!
     *  game.
     *
     *  https://github.com/metteo/chip8-test-rom
     */

    if (QuirkSet(Quirk::IRegCarry)) {
      uint32_t sum = regs.i + regs.v[mOperands[0].value];

      regs.i = sum & 0xFFF;
      regs.v[0xF] = sum > 0xFFF ? 1 : 0;
    } else {
      regs.i += regs.v[mOperands[0].value];
    }
  }

  /* FX29 */
  void Chip8::ProcLoadISpriteAddr() {
    auto &bus = Bus::Get();

    regs.i = bus.CharacterAddress(regs.v[mOperands[0].value & 0xF]);
  }

  /* FX30 */
  void Chip8::ProcLoadIBigSpriteAddr() {
    auto &bus = Bus::Get();
    regs.i = bus.BigCharacterAddress(regs.v[mOperands[0].value] & 0xF);
  }

  /* FX33 */
  void Chip8::ProcLoadBCD() {
    auto &bus = Bus::Get();

    // Unpacked NBCD
    uint8_t value = regs.v[mOperands[0].value];

    bus.Write(regs.i, (value / 100) % 10);
    bus.Write(regs.i + 1, (value / 10) % 10);
    bus.Write(regs.i + 2, value % 10);
  }

  /* FX3A */
  void Chip8::ProcPitch() {
    auto &bus = Bus::Get();

    // This is pretty much just voodoo to me... wish the spec went into more details.
    regs.pitch = 4000.0 * std::pow(2.0, (regs.v[mOperands[0].value] - 64.0) / 48.0);
    m_PitchDirty = true;
    bus.UseBeepBuffer(false);
  }

  /* FX55 */
  void Chip8::ProcRegisterSaveIncrement() {
    auto &bus = Bus::Get();

    uint8_t n = 0;
    uint8_t i = 0;

    do {
      bus.Write(regs.i + i++, regs.v[n]);
      n++;
    } while (n <= mOperands[0].value);

    if (!QuirkSet(Quirk::LoadStore))
      regs.i = (regs.i + mOperands[0].value + 1) & 0xFFFF;
  }

  /* FX65 */
  void Chip8::ProcRegisterLoadIncrement() {
    auto &bus = Bus::Get();

    uint8_t n = 0;
    uint8_t i = 0;

    do {
      regs.v[n] = bus.Read(regs.i + i++);
      n++;
    } while (n <= mOperands[0].value);

    if (!QuirkSet(Quirk::LoadStore))
      regs.i = (regs.i + mOperands[0].value + 1) & 0xFFFF;
  }

  /* FX75 */
  void Chip8::ProcSaveFlags() {
    auto &bus = Bus::Get();

    const auto &romPath = bus.Path();
    std::string path = romPath + ".flags";

    std::ofstream stream(path.c_str(), std::ios::binary);

    if (!stream.good()) {
      spdlog::get("console")->warn("Couldn't save flags at: {}", path);
      return;
    }

    char flags[16];
    memset(flags, 0, 16);

    uint8_t n = 0;

    do {
      flags[n] = regs.v[n];
      n++;
    } while (n <= mOperands[0].value);

    stream.write(flags, 16);
    stream.close();
  }

  /* FX85 */
  void Chip8::ProcLoadFlags() {
    auto &bus = Bus::Get();

    const auto &romPath = bus.Path();
    std::string path = romPath + ".flags";

    std::ifstream stream(path.c_str(), std::ios::binary);

    if (!stream.good()) {
      spdlog::get("console")->warn("Couldn't load flags at: {}", path);
      memset(regs.v, 0, 16);
      return;
    }

    char flags[16];
    memset(flags, 0, 16);

    stream.read(flags, 16);

    uint8_t n = 0;

    do {
      regs.v[n] = flags[n];
      n++;
    } while (n <= mOperands[0].value);

    stream.close();
  }

  /* Handling for VBlank sprite rendering is pretty much copied
   * straight from Timendus' excellent silicon8.
   * https://github.com/Timendus/silicon8
   *
   * Thanks to Timendus for his test rom which showed the problem
   * to begin with as well.
   */
  bool Chip8::WaitForInterrupt() {
    if (!QuirkSet(Quirk::VBlank)) {
      return false;
    }

    switch (m_WaitForInterrupt) {
      case 0:
        m_WaitForInterrupt = 1;
        regs.pc -= 2;
        return true;

      case 1:
        regs.pc -= 2;
        return true;

      default:
        m_WaitForInterrupt = 0;
        return false;
    }
  }

} // dorito