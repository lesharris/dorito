#include "SoundEditorWidget.h"

#include <algorithm>
#include <random>

#include "layers/UI.h"

#include "external/imgui-knobs.h"

namespace dorito {
  std::vector<uint8_t> SoundEditorWidget::EditorBuffer = std::vector<uint8_t>(16);
  std::vector<uint8_t> SoundEditorWidget::ToneBuffer = std::vector<uint8_t>(16);

  SoundEditorWidget::SoundEditorWidget() {
    m_Sound = LoadAudioStream(44100, 32, 1);
    SetAudioStreamCallback(m_Sound, &SoundEditorWidget::EditorAudioCallback);
    AttachAudioStreamProcessor(m_Sound, &SoundEditorWidget::EditorLowpassFilterCallback);
    SetAudioStreamVolume(m_Sound, 1.0f);

    m_Tone = LoadAudioStream(44100, 32, 1);
    SetAudioStreamCallback(m_Tone, &SoundEditorWidget::ToneAudioCallback);
    AttachAudioStreamProcessor(m_Tone, &SoundEditorWidget::EditorLowpassFilterCallback);
    SetAudioStreamVolume(m_Tone, 1.0f);

    FillTonePattern();
  }

  SoundEditorWidget::~SoundEditorWidget() {
    DetachAudioStreamProcessor(m_Sound, &SoundEditorWidget::EditorLowpassFilterCallback);
    UnloadAudioStream(m_Sound);
    DetachAudioStreamProcessor(m_Tone, &SoundEditorWidget::EditorLowpassFilterCallback);
    UnloadAudioStream(m_Tone);
  }

  void SoundEditorWidget::Draw() {
    if (!ImGui::Begin(ICON_FA_MUSIC " Sound Editor", &UI::ShowSoundEditor)) {
      ImGui::End();
    } else {

      PatternToolbar();

      ImGui::Separator();

      ImGui::BeginTable("sndeditortable", 2, ImGuiTableFlags_None);
      ImGui::TableSetupColumn("left", ImGuiTableColumnFlags_WidthFixed, 512.0f + 2.0f);
      ImGui::TableSetupColumn("right", ImGuiTableColumnFlags_None);
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      PatternEditor();

      ImGui::TableSetColumnIndex(1);
      if (ImGuiKnobs::KnobInt("Pitch", &m_Pitch, 0, 255, 1, "%d",
                              ImGuiKnobVariant_Wiper, 0,
                              ImGuiKnobFlags_DragHorizontal)) {
        SetAudioStreamPitch(m_Sound, std::pow(2.0, (m_Pitch - 64.0) / 48.0));
      }

      ImGui::EndTable();

      ImGui::Separator();

      ByteOutput();

      ImGui::Separator();
      ImGui::Dummy({0.0f, 5.0f});

      TonePattern();

      ImGui::Text("Blend Mode: ");
      ImGui::SameLine();
      if (ImGui::RadioButton("None", m_BlendMode == BlendMode::None)) {
        m_BlendMode = BlendMode::None;
        FillTonePattern();
      }

      ImGui::SameLine();

      if (ImGui::RadioButton("AND", m_BlendMode == BlendMode::AND)) {
        m_BlendMode = BlendMode::AND;
        FillTonePattern();
      }

      ImGui::SameLine();

      if (ImGui::RadioButton("OR", m_BlendMode == BlendMode::OR)) {
        m_BlendMode = BlendMode::OR;
        FillTonePattern();
      }

      ImGui::SameLine();

      if (ImGui::RadioButton("XOR", m_BlendMode == BlendMode::XOR)) {
        m_BlendMode = BlendMode::XOR;
        FillTonePattern();
      }

      ImGui::BeginTable("toneeditorcontrols", 2, ImGuiTableFlags_None);
      ImGui::TableSetupColumn("Width", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Pulse", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      if (ImGui::InputInt("##width", &m_Width)) {
        if (m_Width < 0)
          m_Width = 0;

        m_Width &= 0xFF;

        FillTonePattern();
      }

      ImGui::TableSetColumnIndex(1);
      if (ImGui::InputFloat("##pulse", &m_Pulse, 0.1f, 0.0f, "%g")) {
        if (m_Pulse < 0.0f)
          m_Pulse = 0.0f;

        if (m_Pulse > 1.0f)
          m_Pulse = 1.0f;

        FillTonePattern();
      }

      ImGui::EndTable();

      ImGui::Separator();
      ImGui::Dummy({0.0f, 5.0f});

      if (IsAudioStreamPlaying(m_Tone)) {
        if (ImGui::Button(ICON_FA_STOP " Stop##tone")) {
          StopAudioStream(m_Tone);
        }
      } else {
        if (ImGui::Button(ICON_FA_PLAY " Play##tone")) {
          PlayAudioStream(m_Tone);
        }
      }

      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_SHARE " Apply to Pattern")) {
        EditorBuffer = ToneBuffer;
        m_Pattern = m_TonePattern;
      }

      ImGui::End();
    }
  }

  void SoundEditorWidget::PatternToolbar() {
    if (IsAudioStreamPlaying(m_Sound)) {
      if (ImGui::Button(ICON_FA_STOP " Stop")) {
        StopAudioStream(m_Sound);
      }
    } else {
      if (ImGui::Button(ICON_FA_PLAY " Play")) {
        PlayAudioStream(m_Sound);
      }
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_LEFT)) {
      std::rotate(m_Pattern.begin(), m_Pattern.begin() + 1, m_Pattern.end());
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_RIGHT)) {
      std::rotate(m_Pattern.rbegin(), m_Pattern.rbegin() + 1, m_Pattern.rend());
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_DICE)) {
      std::random_device rnd_device;
      std::mt19937 mt{rnd_device()};
      std::uniform_int_distribution<uint16_t> dist{0, 1};
      std::uniform_int_distribution<uint16_t> byteDist{0, 255};

      std::generate(m_Pattern.begin(), m_Pattern.end(), [&dist, &mt]() {
        return dist(mt);
      });

      m_Pitch = byteDist(mt);
      SetAudioStreamPitch(m_Sound, std::pow(2.0, (m_Pitch - 64.0) / 48.0));
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_TRASH)) {
      memset(&m_Pattern[0], 0, 128);
    }
  }

  void SoundEditorWidget::PatternEditor() {
    auto &bus = Bus::Get();
    auto &palette = bus.GetDisplay().Palette();

    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

    float xscale = 4;
    canvas_sz.x = 128.0f * xscale;
    canvas_sz.y = 24.0f * 2;

    auto title_size = ImGui::CalcTextSize("Pattern", nullptr, false, canvas_sz.x);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (canvas_sz.x - title_size[0]) * 0.5f);
    ImGui::Text("%s", "Pattern");

    canvas_p0.y += 24.0f + 6.0f;

    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(palette[0].r, palette[0].g, palette[0].b, palette[0].a));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(palette[1].r, palette[1].g, palette[1].b, palette[1].a), 0, 0,
                       2.0f);

    ImGui::SetCursorScreenPos(canvas_p0);
    ImGui::InvisibleButton("soundcanvas", canvas_sz,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    const bool is_hovered = ImGui::IsItemHovered();
    const ImVec2 origin(canvas_p0.x, canvas_p0.y);
    const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

    if (is_hovered && (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
      bool leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
      bool rightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

      uint8_t value = 0;

      if (leftDown)
        value = 1;

      if (rightDown)
        value = 0;

      m_Pattern[(size_t) (mouse_pos_in_canvas.x / xscale)] = value;
    }

    for (auto i = 0; i < 128; i++) {
      if (m_Pattern[i] != 0) {

        draw_list->AddLine(
            {origin.x + (i * xscale), origin.y},
            {origin.x + (i * xscale), origin.y + (24.0f * 2.0f)},
            IM_COL32(palette[1].r, palette[1].g, palette[1].b, palette[1].a), xscale);
      }
    }
  }

  void SoundEditorWidget::ByteOutput() {
    std::string output;
    uint16_t count = 0;

    auto bytes = PackPattern(m_Pattern);

    EditorBuffer = bytes;

    FillTonePattern();

    for (const auto &byte: bytes) {
      output += fmt::format("0x{:02X} ", byte);

      count++;
      if (count % 10 == 0)
        output += "\n";
    }

    ImGui::InputTextMultiline("##soundsource", (char *) output.c_str(), output.size(),
                              ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3),
                              ImGuiInputTextFlags_ReadOnly);
    if (ImGui::Button(ICON_FA_COPY " Copy", {100, 30})) {
      ImGui::SetClipboardText(output.c_str());
    }
  }

  void SoundEditorWidget::EditorAudioCallback(void *buffer, uint32_t frames) {
    static uint32_t cursor = 0;
    std::vector<float> bits;

    float *output = (float *) buffer;

    auto &bus = Bus::Get();
    auto pattern = EditorBuffer;

    auto widgetPatternToBits = [](const std::vector<uint8_t> &pattern) {
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

    bits = widgetPatternToBits(pattern);

    for (uint32_t i = 0; i < frames; i++) {
      output[i] = bits[cursor++];
      cursor %= 512;
    }
  }

  void SoundEditorWidget::EditorLowpassFilterCallback(void *buffer, uint32_t frames) {
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

  void SoundEditorWidget::TonePattern() {
    auto &bus = Bus::Get();
    auto &palette = bus.GetDisplay().Palette();

    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

    float xscale = 4;
    canvas_sz.x = 128.0f * xscale;
    canvas_sz.y = 24.0f * 2;

    auto title_size = ImGui::CalcTextSize("Tone Generator", nullptr, false, canvas_sz.x);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (canvas_sz.x - title_size[0]) * 0.5f);
    ImGui::Text("%s", "Tone Generator");

    canvas_p0.y += 24.0f + 6.0f;

    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(palette[0].r, palette[0].g, palette[0].b, palette[0].a));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(palette[1].r, palette[1].g, palette[1].b, palette[1].a), 0, 0,
                       2.0f);

    ImGui::SetCursorScreenPos(canvas_p0);
    ImGui::InvisibleButton("tonecanvas", canvas_sz,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    const ImVec2 origin(canvas_p0.x, canvas_p0.y);

    for (auto i = 0; i < 128; i++) {
      if (m_TonePattern[i] != 0) {

        draw_list->AddLine(
            {origin.x + (i * xscale), origin.y},
            {origin.x + (i * xscale), origin.y + (24.0f * 2.0f)},
            IM_COL32(palette[1].r, palette[1].g, palette[1].b, palette[1].a), xscale);
      }
    }
  }

  void SoundEditorWidget::ToneAudioCallback(void *buffer, uint32_t frames) {
    static uint32_t cursor = 0;
    std::vector<float> bits;

    float *output = (float *) buffer;

    auto &bus = Bus::Get();
    auto pattern = ToneBuffer;

    auto widgetPatternToBits = [](const std::vector<uint8_t> &pattern) {
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

    bits = widgetPatternToBits(pattern);

    for (uint32_t i = 0; i < frames; i++) {
      output[i] = bits[cursor++];
      cursor %= 512;
    }
  }

  void SoundEditorWidget::FillTonePattern() {
    auto pulse = std::ceilf(m_Pulse * m_Width);

    for (auto i = 0; i < 16; i++) {
      uint8_t r = 0;
      for (auto b = 0; b < 8; b++)
        r |= ((((i % 16) * 8 + b) % m_Width) < pulse ? 1 : 0) * (1 << (7 - b));

      switch (m_BlendMode) {
        case BlendMode::None:
          ToneBuffer[i] = r;
          break;
        case BlendMode::AND:
          ToneBuffer[i] = r & EditorBuffer[i];
          break;
        case BlendMode::OR:
          ToneBuffer[i] = r | EditorBuffer[i];
          break;
        case BlendMode::XOR:
          ToneBuffer[i] = r ^ EditorBuffer[i];
          break;
      }
    }

    m_TonePattern = UnpackPattern(ToneBuffer);
  }

  std::vector<uint8_t> SoundEditorWidget::PackPattern(const std::vector<uint8_t> &pattern) {
    std::vector<uint8_t> results;

    auto getNibble = [&](const std::vector<uint8_t> &data, uint16_t &index) {
      uint8_t nibble = 0;

      for (auto i = 0; i < 4; i++) {
        uint8_t bit = (3 - i);
        uint8_t pixel = data[index++];

        nibble |= (pixel << bit);

      }
      return nibble;
    };

    uint16_t index = 0;
    uint16_t count = 0;

    do {
      uint8_t high = getNibble(pattern, index);
      uint8_t low = getNibble(pattern, index);

      uint8_t packed = (high << 4) | low;

      results.push_back(packed);
      count++;
    } while (count < 16);

    return results;
  }

  std::vector<uint8_t> SoundEditorWidget::UnpackPattern(const std::vector<uint8_t> &pattern) {
    std::vector<uint8_t> bits = std::vector<uint8_t>(128);
    auto i = 0;

    for (const uint8_t byte: pattern) {
      uint8_t mask = 128;

      while (mask != 0) {
        const uint8_t val = (byte & mask) != 0 ? 1 : 0;
        bits[i++] = val;
        mask >>= 1;
      }
    }

    return bits;
  }

} // dorito