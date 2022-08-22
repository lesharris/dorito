#pragma once

#include <raylib.h>

#include <string>
#include <vector>

#include "core/events/EventManager.h"

#include "cpu/Chip8.h"
#include "cpu/Memory.h"
#include "display/Display.h"

#include "common/Preferences.h"

namespace dorito {

  class Bus {
  public:
    enum class CompatProfile {
      VIP,
      SCHIP,
      XOChip
    };

  public:
    static Bus &Get() {
      static Bus instance;

      return instance;
    }

  public:
    void Tick();

    void TickTimers();

    void LoadRom(const std::string &path);

    uint8_t Read(uint16_t addr);

    void Write(uint16_t addr, uint8_t data);

    void WriteAudio(uint8_t position, uint8_t data);

    uint16_t Pop();

    void Push(uint16_t addr);

    void ClearScreen() { m_Display.Clear(); }

    void ScrollUp(uint8_t count) { m_Display.ScrollUp(count); }

    void ScrollDown(uint8_t count) { m_Display.ScrollDown(count); }

    void ScrollRight() { m_Display.ScrollRight(4); }

    void ScrollLeft() { m_Display.ScrollLeft(4); }

    void SetHighRes(bool isSet) {
      m_Display.HighRes(isSet);
      auto mask = m_Display.PlaneMask();
      m_Display.PlaneMask(0x3);
      m_Display.Clear();
      m_Display.PlaneMask(mask);
    }

    bool Plot(uint8_t plane, uint8_t x, uint8_t y) {
      return m_Display.Plot(plane, x, y);
    }

    uint16_t CharacterAddress(uint8_t character) {
      return m_Ram.CharacterAddress(character);
    }

    uint16_t BigCharacterAddress(uint8_t character) {
      return m_Ram.BigCharacterAddress(character);
    }

    void SetQuirk(Chip8::Quirk quirk, bool isSet);

    void SetPlane(uint8_t plane) {
      m_Display.PlaneMask(plane);
    }

    uint8_t GetPlane() {
      return m_Display.PlaneMask();
    }

    void UseBeepBuffer(bool use) {
      m_UseBeepBuffer = use;
      m_Ram.UseBeep(use);
    }

    void Muted(bool isMuted) {
      m_Muted = isMuted;
    }

    void AddRecentSourceFile(const std::string &path);

  public:
    Display &GetDisplay() {
      return m_Display;
    }

    Chip8 &GetCpu() {
      return m_Cpu;
    }

    Memory &GetRam() {
      return m_Ram;
    }

    [[nodiscard]] const std::vector<std::string> &RecentRoms() const {
      return m_RecentRoms;
    }

    [[nodiscard]] const std::vector<std::string> &RecentSourceFiles() const {
      return m_RecentSourceFiles;
    }

    [[nodiscard]] bool Muted() const {
      return m_Muted;
    }

    [[nodiscard]] uint8_t DisplayWidth() const {
      return m_Display.Width();
    }

    [[nodiscard]] uint8_t DisplayHeight() const {
      return m_Display.Height();
    }

    [[nodiscard]] std::vector<bool> Quirks() const {
      return m_Cpu.GetQuirks();
    }

    [[nodiscard]] CompatProfile GetCompatProfile() const {
      return m_CompatProfile;
    }

    [[nodiscard]] uint16_t CyclesPerFrame() const {
      return m_CyclesPerFrame;
    }

    [[nodiscard]] const std::vector<Color> &Palette() const {
      return m_Display.Palette();
    }

    [[nodiscard]] const std::vector<std::vector<uint8_t>> &Buffers() const {
      return m_Display.Buffers();
    }

    [[nodiscard]] bool Running() const { return m_Running; }

    [[nodiscard]] const std::string &Path() const { return m_RomPath; }

  private:
    Bus();

    ~Bus();

  private:
    static void AudioCallback(void *buffer, uint32_t frames);

    static void LowpassFilterCallback(void *buffer, uint32_t frames);

  private:
    void SetCompatProfile(const CompatProfile &profile);

    void SaveGamePrefs();

    void LoadGamePrefs();

    void SavePrefs();

    void LoadPrefs();

  private:
    void HandleLoadRom(const Events::LoadROM &event);

    void HandleStepCpu(const Events::StepCPU &event);

    void HandleExecute(const Events::ExecuteCPU &event);

    void HandleSetCycles(const Events::SetCycles &event);

    void HandleReset(const Events::Reset &event);

    void HandleUnload(const Events::UnloadROM &event);

    void HandleKeyDown(const Events::KeyDown &event);

    void HandleKeyUp(const Events::KeyUp &event);

    void HandleKeyReleased(const Events::KeyReleased &event);

    void HandleVIPCompat(const Events::VIPCompat &event);

    void HandleSCHIPCompat(const Events::SCHIPCompat &event);

    void HandleXOCompat(const Events::XOCompat &event);

    void HandleSetQuirk(const Events::SetQuirk &event);

    void HandleSavePrefs(const Events::SavePrefs &event);

    void HandleSaveAppPrefs(const Events::SaveAppPrefs &event);

    void HandleSetMute(const Events::SetMute &event);

    void HandleRunCode(const Events::RunCode &event);

    void HandleClearRecents(const Events::UIClearRecents &event);

    void HandleClearRecentSources(const Events::UIClearRecentSources &event);

    void HandleAddRecentSourceFile(const Events::UIAddRecentSourceFile &event);

  private:
    friend class UI;

  private:
    Chip8 m_Cpu;
    Memory m_Ram;
    Display m_Display;

    uint16_t m_CyclesPerFrame = 100;
    bool m_Running = false;
    bool m_UseBeepBuffer = true;
    bool m_Muted = false;

    CompatProfile m_CompatProfile = CompatProfile::XOChip;

    std::string m_RomPath;
    GamePrefs m_GamePrefs;

    DoritoPrefs m_Prefs;

    AudioStream m_Sound;

    std::vector<std::string> m_RecentRoms;
    std::vector<std::string> m_RecentSourceFiles;
  };


} // dorito

