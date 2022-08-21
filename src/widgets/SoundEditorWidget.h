#pragma once

#include "Widget.h"

namespace dorito {

  class SoundEditorWidget : public Widget {
  public:
    SoundEditorWidget();

    ~SoundEditorWidget();

    std::string Name() override {
      return "SoundEditor";
    }

    void Draw() override;

  private:
    void PatternEditor();

    void ByteOutput();

    void PatternToolbar();

    void TonePattern();

    void FillTonePattern();

    std::vector<uint8_t> PackPattern(const std::vector<uint8_t> &pattern);

    std::vector<uint8_t> UnpackPattern(const std::vector<uint8_t> &pattern);

  private:
    static std::vector<uint8_t> EditorBuffer;
    static std::vector<uint8_t> ToneBuffer;

  private:
    static void EditorAudioCallback(void *buffer, uint32_t frames);

    static void ToneAudioCallback(void *buffer, uint32_t frames);

    static void EditorLowpassFilterCallback(void *buffer, uint32_t frames);

  private:
    enum class BlendMode {
      None,
      AND,
      OR,
      XOR
    };

  private:
    AudioStream m_Sound;
    AudioStream m_Tone;

    std::vector<uint8_t> m_Pattern = std::vector<uint8_t>(128);
    int m_Pitch = 64;

    std::vector<uint8_t> m_TonePattern = std::vector<uint8_t>(128);
    BlendMode m_BlendMode = BlendMode::None;
    int32_t m_Width = 16;
    float m_Pulse = 0.5f;
  };

} // dorito
