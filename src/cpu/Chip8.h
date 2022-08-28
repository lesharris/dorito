#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <random>

namespace dorito {

  class Chip8 {
  public:
    using InstructionProc = void (Chip8::*)();

    enum class Quirk {
      Shift = 0,
      LoadStore,
      Clip,
      Jump,
      Logic,
      LoresSprites,
      VBlank,
      IRegCarry
    };

    struct Breakpoint {
      std::string label;
      uint16_t addr;
      bool enabled;
    };

    struct Registers {
      // General purpose registers
      uint8_t v[16];

      // Program Counter
      uint16_t pc;

      // Fetching
      uint16_t latch;

      // Index Register
      uint16_t i;

      // Sound and Delay Timers
      uint8_t st;
      uint8_t dt;

      // Audio Pitch Frequency
      double pitch;

      // Keys
      bool keys[16];

      // Quirks
      bool quirks[8];

      // Breakpoints
      std::vector<Breakpoint> breakpoints;
    };

    struct DisassemblyLine {
      uint16_t addr;
      uint16_t index;
      std::string text;
      std::string bytes;
    };

    enum class OperandType {
      Register = 0,
      Number4bit,
      Number8bit,
      Number12bit,
      Number16bit
    };

    struct Operand {
      OperandType type;
      uint16_t value;
    };

    struct Instruction {
      uint16_t code;
      std::string encoding;
      std::string label;
      uint8_t operand_count;
      OperandType operand_order[3];
      InstructionProc proc;
    };

  public:
    Chip8();

    void Reset();

    void Tick(uint32_t cycles);

    void Step();

    void TickTimers();

    void Halted(bool isHalted) {
      m_Halted = isHalted;
    }

    std::string OperandTypeName(const OperandType &type) {
      return m_OpTypeLabels[type];
    }

    void SetKeyState(uint8_t key, bool state) {
      regs.keys[key & 0xF] = state;
    }

    void KeyPressed(uint8_t key) {
      if (m_Waiting) {
        regs.v[m_KeyPressRegister] = key;
        m_Waiting = false;
      }
    }

    void SetQuirks(const std::vector<Quirk> &quirks, bool isSet) {
      for (const auto &quirk: quirks) {
        regs.quirks[static_cast<uint8_t>(quirk)] = isSet;
      }
    }

    void SetHighRes(bool isSet) {
      m_HighRes = isSet;
    }

    void AddBreakpoint(const Breakpoint &bp) {
      regs.breakpoints.push_back(bp);
    }

    void ToggleBreakpoint(const Breakpoint &breakpoint) {
      for (auto &bp: regs.breakpoints) {
        if (bp.label == breakpoint.label) {
          bp.enabled = !bp.enabled;
          break;
        }
      }
    }

    void RemoveBreakpoint(const Breakpoint &breakpoint) {
      std::vector<Breakpoint> newBps;

      for (const auto &bp: regs.breakpoints) {
        if (bp.label != breakpoint.label)
          newBps.push_back(bp);
      }

      regs.breakpoints = newBps;
    }

    void ClearBreakpoints() {
      regs.breakpoints.clear();
    }

    [[nodiscard]] const std::vector<Breakpoint> &Breakpoints() const {
      return regs.breakpoints;
    }

    [[nodiscard]] bool Halted() const { return m_Halted; }

    [[nodiscard]] std::vector<bool> GetQuirks() const {
      std::vector<bool> result;
      for (bool quirk: regs.quirks)
        result.push_back(quirk);

      return result;
    }

  private:
    void Fetch();

    void ExtractOperands(uint16_t opcode, Instruction *current, bool advancePC);

    Instruction *Decode(uint16_t code, bool silent = false);

    void DisassembleNext();

    void SetFlag(uint8_t dest, uint16_t value, bool isSet);

    void Skip();

    bool QuirkSet(const Quirk &quirk) {
      return regs.quirks[static_cast<uint8_t>(quirk)];
    }

    bool WaitForInterrupt();

  private:
    void ProcUnimplemented();

    void ProcClearScreen();

    void ProcReturn();

    void ProcJump();

    void ProcCall();

    void ProcSkipEqualsLiteral();

    void ProcSkipNotEqualsLiteral();

    void ProcSkipRegEqualsReg();

    void ProcLoadLiteral();

    void ProcAddLiteral();

    void ProcLoadRegister();

    void ProcORReg();

    void ProcANDReg();

    void ProcXORReg();

    void ProcAdd();

    void ProcSubtractYFromX();

    void ProcShiftRight();

    void ProcSubtractXFromY();

    void ProcShiftLeft();

    void ProcSkipRegNotEqualReg();

    void ProcLoadILiteral();

    void ProcLoadIExtended();

    void ProcJumpRelative();

    void ProcRandom();

    void ProcDrawSprite();

    void ProcSkipKeyPressed();

    void ProcSkipKeyNotPressed();

    void ProcLoadDelayToReg();

    void ProcLoadKeypressToReg();

    void ProcLoadRegToDelay();

    void ProcLoadRegToBuzzer();

    void ProcAddIWithReg();

    void ProcLoadISpriteAddr();

    void ProcLoadBCD();

    void ProcRegisterSaveIncrement();

    void ProcRegisterLoadIncrement();

    void ProcSaveFlags();

    void ProcLoadFlags();

    void ProcExit();

    void ProcLoadIBigSpriteAddr();

    void ProcHires();

    void ProcLores();

    void ProcScrollDown();

    void ProcScrollRight();

    void ProcScrollLeft();

    void ProcScrollUp();

    void ProcPlane();

    void ProcRegisterSave();

    void ProcRegisterLoad();

    void ProcPitch();

    void ProcAudio();

  private:
    friend class RegistersWidget;

    friend class DisassemblyWidget;

    friend class MonitorsWidget;

    friend class EditorWidget;
    
    friend class AudioWidget;

    friend class Bus;

  private:
    std::map<uint16_t, Instruction> m_Instructions;

    std::map<OperandType, std::string> m_OpTypeLabels{
        {OperandType::Register,    "Register"},
        {OperandType::Number4bit,  "Number4bit"},
        {OperandType::Number8bit,  "Number8bit"},
        {OperandType::Number12bit, "Number12bit"},
        {OperandType::Number16bit, "Number16bit"},
    };

    Registers regs{};

    Operand mOperands[3]{};

    Instruction *m_CurrentInstruction = nullptr;

    uint32_t m_Cycles = 0;

    bool m_Halted = true;
    bool m_Waiting = false;
    bool m_HighRes = false;
    bool m_PitchDirty = false;

    std::map<uint16_t, DisassemblyLine> m_Disassembly;
    uint16_t m_DisasmCount = 0;
    uint16_t m_PrevPC = 0x200;

    std::random_device rd;
    std::mt19937 mt{rd()};

    uint8_t m_KeyPressRegister = 0;

    uint8_t m_WaitForInterrupt = 0;
  };

} // dorito

