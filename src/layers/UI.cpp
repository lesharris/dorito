#include "UI.h"
#include "imgui_internal.h"

#include <zep.h>
#include <nfd.h>

#include "system/Bus.h"

namespace dorito {
  void UI::OnAttach() {
    ImGui::CreateContext(nullptr);

    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Render fonts at half scale for sharper fonts.
    auto dpi = GetWindowScaleDPI();
    io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 16.0f * dpi.y);
    io.FontGlobalScale = 1.0f / dpi.y;

    auto glfwWindow = glfwGetCurrentContext();
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    EventManager::Get().Attach<
        Events::KeyPressed,
        &::dorito::UI::HandleKeyPress
    >(this);
  }

  void UI::OnDetach() {
    EventManager::Get().DetachAll(this);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void UI::EndFrame() {
    ImGuiIO &io = ImGui::GetIO();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      auto currentContext = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(currentContext);
    }
  }

  void UI::Render() {
    auto &bus = Bus::Get();
    auto &io = ImGui::GetIO();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Dockspace", nullptr,
                 ImGuiWindowFlags_MenuBar |
                 ImGuiWindowFlags_NoDocking |
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBringToFrontOnFocus |
                 ImGuiWindowFlags_NoNavFocus);

    ImGui::PopStyleVar(3);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

    UpdateMenuStatus();
    DrawMenubar();

    // Display Windows
    {

      if (m_ShowDemo) {
        ImGui::ShowDemoWindow(&m_ShowDemo);
      }

      if (m_ShowMemory) {
        static MemoryEditor memoryViewer;

        memoryViewer.DrawWindow("Memory", &bus.m_Ram.m_Ram[0], 1024 * 64);
      }

      if (m_ShowRegisters) {
        Registers();
      }

      if (m_ShowDisassembly) {
        Disassembly();
      }

      if (m_ShowAudio) {
        Audio();
      }

      if (m_ShowColorEditor) {
        ColorEditor();
      }

      if (m_ShowCodeEditor) {
        Code();
      }

      Viewport();
    }

    ImGui::End();
  }

  std::string UI::Name() const noexcept {
    return "ui";
  }

  /*
   * Event Handlers
   */
  void UI::HandleKeyPress(Events::KeyPressed &) {

  }

  /*
   * Utilities
   */
  ImVec2 UI::GetLargestSizeForViewport() {
    ImVec2 size = ImGui::GetContentRegionAvail();

    size.x -= ImGui::GetScrollX();
    size.y -= ImGui::GetScrollY();

    float aspectWidth = size.x;
    float aspectHeight = aspectWidth / (2.0f / 1.0f);

    if (aspectHeight > size.y) {
      aspectHeight = size.y;
      aspectWidth = aspectHeight * (2.0f / 1.0f);
    }

    return {aspectWidth, aspectHeight};
  }

  ImVec2 UI::GetCenteredPositionForViewport(ImVec2 &aspectSize) {
    ImVec2 size = ImGui::GetContentRegionAvail();

    size.x -= ImGui::GetScrollX();
    size.y -= ImGui::GetScrollY();

    float viewportX = (size.x / 2.0f) - (aspectSize.x / 2.0);
    float viewportY = (size.y / 2.0f) - (aspectSize.y / 2.0);

    return {viewportX + ImGui::GetCursorPosX(), viewportY + ImGui::GetCursorPosY()};
  }

  ImVec2 operator+(const ImVec2 &lhs, const ImVec2 &rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
  }

  ImVec2 UI::ImRotate(const ImVec2 &v, float cos_a, float sin_a) {
    return ImVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a);
  }

  void UI::ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle) {
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    ImVec2 pos[4] =
        {
            center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
            center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
        };
    ImVec2 uvs[4] =
        {
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 1.0f),
            ImVec2(1.0f, 0.0f),
            ImVec2(0.0f, 0.0f)
        };

    draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
  }

  void UI::EnablePalette(uint8_t pal) {
    memset(m_PaletteState, 0, 9);
    m_PaletteState[pal] = true;
  }

  void UI::SetQuirkEntry(uint8_t quirkIndex, bool isSet) {
    auto quirk = static_cast<Chip8::Quirk>(quirkIndex);
    auto it = std::find_if(
        m_QuirkEntries.begin(),
        m_QuirkEntries.end(),
        [quirk](const QuirkEntry &entry) {
          return quirk == entry.quirk;
        });

    if (it != std::end(m_QuirkEntries)) {
      it->set = isSet;
    }
  }

  void UI::UpdateMenuStatus() {
    auto &bus = Bus::Get();

    auto cycles = bus.CyclesPerFrame();

    uint8_t index = 0;
    for (const auto &val: m_CycleSet) {
      if (m_CycleEntries[index].cycles == 0) {
        index++;
      }

      m_CycleEntries[index++].set = (cycles == val);
    }

    auto quirks = bus.Quirks();

    for (const auto &entry: m_QuirkEntries) {
      uint8_t quirkIndex = static_cast<uint8_t>(entry.quirk);
      SetQuirkEntry(quirkIndex, quirks[quirkIndex]);
    }

    auto profile = bus.GetCompatProfile();

    m_ProfileEntries[0].set = profile == Bus::CompatProfile::VIP;
    m_ProfileEntries[1].set = profile == Bus::CompatProfile::SCHIP;
    m_ProfileEntries[2].set = profile == Bus::CompatProfile::XOChip;

    m_DoritoMuted = bus.m_Muted;
  }

  /*
   * Windows
   */

  void UI::Viewport() {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

    static float angle = 0.0f;

    // Render our Emu Viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::SetNextWindowClass(&window_class);

    if (!ImGui::Begin("Screen", &m_ShowEmu,
                      ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse |
                      ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoDecoration)) {
      ImGui::End();
    } else {
      auto windowWidth = GetScreenWidth();
      auto windowHeight = GetScreenHeight();

      auto size = ImGui::GetContentRegionAvail();
      auto contentMin = ImGui::GetWindowContentRegionMin();
      auto windowPos = ImGui::GetWindowPos();
      auto mousePos = ImGui::GetMousePos();

      auto topLeft = ImVec2{windowPos.x + contentMin.x, windowPos.y + contentMin.y};
      auto bottomRight = ImVec2{topLeft.x + size.x, topLeft.y + size.y};

      ImVec2 emuWindowSize = GetLargestSizeForViewport();
      ImVec2 emuWindowPos = GetCenteredPositionForViewport(emuWindowSize);

      ImGui::SetCursorPos(emuWindowPos);

      // Is mouse is over the viewport window...
      if (ImGui::IsMouseHoveringRect(topLeft, bottomRight)) {
        // Transform from screen to window local coords
        ImVec2 position = {mousePos.x - topLeft.x, mousePos.y - topLeft.y};

        EventManager::Dispatcher().trigger<Events::UIMouseMove>({position.x, position.y});
      }

      if (windowWidth != m_PrevScreenWidth || windowHeight != m_PrevScreenHeight) {
        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;

        EventManager::Dispatcher().trigger<Events::ViewportResized>({size.x, size.y});
      } else {
        auto &texture = Dorito::Get().GetRenderTexture();

        /*auto &io = ImGui::GetIO();

         angle += io.DeltaTime * 1.0f;

         float ninety = 1.5708;
         float one80 = 3.14159;
         float two70 = 4.71239;

         ImageRotated(reinterpret_cast<ImTextureID>((uint64_t) texture.texture.id),
                      {
                          emuWindowPos.x,
                          (emuWindowPos.y / 2.0f) + (emuWindowSize.y) - ImGui::GetCursorPosY()
                      },
                      emuWindowSize, 0);*/


        ImGui::Image(reinterpret_cast<ImTextureID>((uint64_t) texture.texture.id),
                     emuWindowSize,
                     {0, 1}, {1, 0});

        m_PreviousWindowSize = size;
        m_PrevScreenWidth = windowWidth;
        m_PrevScreenHeight = windowHeight;
      }

      ImGui::End();
    }

    ImGui::PopStyleVar(3);
  }

  void UI::Registers() {
    auto &bus = Bus::Get();
    auto &cpu = bus.m_Cpu;

    if (!ImGui::Begin("Registers", &m_ShowRegisters)) {
      ImGui::End();
    } else {

      ImGui::BeginTable("flagsbtnscycles", 2);
      ImGui::TableSetupColumn("btns");
      ImGui::TableSetupColumn("cycles");

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      if (ImGui::Button(bus.Running() ? "Pause" : "Run")) {
        EventManager::Dispatcher().enqueue<Events::ExecuteCPU>({!bus.Running()});
      }

      ImGui::SameLine();

      if (!bus.Running()) {
        if (ImGui::Button("Step")) {
          EventManager::Dispatcher().enqueue<Events::StepCPU>({});
        }
      }

      ImGui::TableSetColumnIndex(1);
      auto cycleInfo = fmt::format("Total Cycles: {}", cpu.m_Cycles);
      auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(cycleInfo.c_str()).x
                   - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
      if (posX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(posX);
      ImGui::TextUnformatted(cycleInfo.c_str());

      ImGui::EndTable();

      ImGui::Separator();

      if (!bus.Running()) {
        static uint16_t addr = 0x200;

        ImGui::Text("Run to Address:");
        ImGui::SameLine();
        ImGui::InputScalar("##", ImGuiDataType_U16, &addr, nullptr, nullptr, "0x%04X");
        if (ImGui::Button("Go...")) {
          EventManager::Dispatcher().enqueue<Events::ExecuteUntil>({addr});
        }
      }

      ImGui::Separator();

      ImGui::BeginTable("pci", 3, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("I", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Latch", ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(fmt::format("0x{:04X}", cpu.regs.pc).c_str());

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(fmt::format("0x{:04X}", cpu.regs.i).c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(fmt::format("0x{:04X}", cpu.regs.latch).c_str());


      ImGui::EndTable();

      ImGui::BeginTable("dtst", 2, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Delay Timer", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Sound Timer", ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.dt).c_str());

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.st).c_str());

      ImGui::EndTable();

      ImGui::BeginTable("regs07", 8, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("V0", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V1", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V2", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V3", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V4", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V5", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V6", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V7", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      for (auto r = 0; r < 8; r++) {
        ImGui::TableSetColumnIndex(r);
        ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.v[r]).c_str());
      }

      ImGui::EndTable();

      ImGui::BeginTable("regs8F", 8, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("V8", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("V9", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VA", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VB", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VC", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VD", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VE", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("VF", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      for (auto r = 8; r < 16; r++) {
        ImGui::TableSetColumnIndex(r - 8);
        ImGui::TextUnformatted(fmt::format("0x{:02X}", cpu.regs.v[r]).c_str());
      }

      ImGui::EndTable();

      ImGui::Separator();

      ImGui::BeginTable("opsstack", 2, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Operands", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Stack", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      ImGui::BeginTable("ops", 2, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("OpType", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("OpValue", ImGuiTableColumnFlags_None);

      if (cpu.m_CurrentInstruction) {
        for (auto i = 0; i < cpu.m_CurrentInstruction->operand_count; i++) {
          ImGui::TableNextRow();
          auto &op = cpu.mOperands[i];
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%s", cpu.OperandTypeName(op.type).c_str());
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("0x%X (%d)", op.value, op.value);
        }
      }

      ImGui::EndTable();

      ImGui::TableSetColumnIndex(1);

      ImGui::BeginTable("stack", 1, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("address", ImGuiTableColumnFlags_None);

      for (const auto &item: bus.m_Ram.m_Stack) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("0x%X", item);
      }

      ImGui::EndTable();


      ImGui::EndTable();


      ImGui::End();
    }
  }

  void UI::Disassembly() {
    auto &bus = Bus::Get();
    auto &cpu = bus.m_Cpu;

    if (!ImGui::Begin("Disassembly", &m_ShowDisassembly)) {
      ImGui::End();
    } else {

      if (ImGui::BeginTable("disa", 3, ImGuiTableFlags_ScrollY |
                                       ImGuiTableFlags_BordersOuterH |
                                       ImGuiTableFlags_BordersOuterV |
                                       ImGuiTableFlags_RowBg)) {
        auto &lines = cpu.m_Disassembly;

        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthStretch);

        ImGuiListClipper clipper;
        size_t lineSize = lines.size();
        clipper.Begin((int) lineSize);

        auto lineByIndex = [lines](int index) -> Chip8::DisassemblyLine {
          for (const auto &[_, line]: lines) {
            if (line.index == index)
              return line;
          }

          return Chip8::DisassemblyLine{
              0xFFFF,
              0xFFFF,
              "",
              ""
          };
        };

        bool lineHighlite = false;

        while (clipper.Step()) {
          for (auto item = clipper.DisplayStart; item < clipper.DisplayEnd; item++) {
            auto line = lineByIndex(item);

            if (line.index == 0xFFFF)
              continue;

            ImGui::TableNextRow();

            if (!lineHighlite && (line.index == lines.size() - 1 ||
                                  line.addr == cpu.m_PrevPC)) {
              ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.18f, 0.47f, 0.59f, 0.65f));
              ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, row_bg_color);
              lineHighlite = true;
            }

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(fmt::format("${:04X}", line.addr).c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(fmt::format("{}", line.bytes).c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(line.text.c_str());
          }
        }

        if (cpu.m_PrevPC != m_PrevPC) {
          auto addr = cpu.m_PrevPC;

          if (lines.contains(addr)) {
            auto size = ImGui::GetWindowSize();
            auto &line = lines[addr];
            ImGui::SetScrollY(
                (clipper.ItemsHeight * (float) line.index - (size.y / 2) -
                 (clipper.ItemsHeight / clipper.ItemsCount)));
            m_PrevPC = addr;
          }
        }

        ImGui::EndTable();

      }

      ImGui::End();
    }
  }

  void UI::Audio() {
    auto &bus = Bus::Get();
    auto &cpu = bus.m_Cpu;
    auto &buffer = bus.m_Ram.GetAudioBuffer();

    if (!ImGui::Begin("Audio", &m_ShowAudio)) {
      ImGui::End();
    } else {
      ImGui::BeginTable("pitch", 1, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Pitch", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%gHz", cpu.regs.pitch);

      ImGui::EndTable();

      ImGui::Separator();

      ImGui::BeginTable("audiobuffer", 1, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Audio Buffer", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);

      for (auto n = 0; n < 16; n++) {
        ImGui::Text("0x%02X ", buffer[n]);

        if (n != 7)
          ImGui::SameLine();
      }

      ImGui::EndTable();

      ImGui::Separator();

      auto patternToBits = [](const std::vector<uint8_t> &pattern) {
        std::vector<float> bits = std::vector<float>(512);
        auto i = 0;

        for (const uint8_t byte: pattern) {
          uint8_t mask = 128;

          while (mask != 0) {
            const float val = (float) ((byte & mask) != 0 ? 1 : 0);
            bits[i++] = val;
            bits[i++] = val;
            bits[i++] = val;
            bits[i++] = val;
            mask >>= 1;
          }
        }

        return bits;
      };

      auto bits = patternToBits(buffer);

      ImGui::PlotLines("Waveform",
                       &bits[0], 512,
                       0, nullptr, -0.2f, 1.2f,
                       ImVec2(0, 100.0f));


      ImGui::End();
    }
  }

  void UI::ColorEditor() {
    auto &bus = Bus::Get();

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    auto &palette = bus.m_Display.Palette();

    auto colorToImvec = [](const Color &color) {
      return ImVec4{
          color.r / 255.0f,
          color.g / 255.0f,
          color.b / 255.0f,
          color.a / 255.0f
      };
    };

    auto ImvecToColor = [](const ImVec4 &vec) {
      return Color{
          static_cast<unsigned char>(vec.x * 255),
          static_cast<unsigned char>(vec.y * 255),
          static_cast<unsigned char>(vec.z * 255),
          static_cast<unsigned char>(vec.w * 255)
      };
    };

    ImVec4 bgColor = colorToImvec(palette[0]);
    ImVec4 fc1Color = colorToImvec(palette[1]);
    ImVec4 fc2Color = colorToImvec(palette[2]);
    ImVec4 blColor = colorToImvec(palette[3]);

    if (!ImGui::Begin("Edit Colors", &m_ShowColorEditor)) {
      ImGui::End();
    } else {

      ImGui::BeginTable("colors", 4, ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("Background Color", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Fill Color 1", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Fill Color 2", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("Blended Color", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      if (ImGui::ColorPicker3("##bgcolor", (float *) &bgColor, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(bgColor);
        EventManager::Dispatcher().trigger<Events::SetColor>({0, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }

      ImGui::TableSetColumnIndex(1);
      if (ImGui::ColorPicker3("##fc1color", (float *) &fc1Color, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(fc1Color);
        EventManager::Dispatcher().trigger<Events::SetColor>({1, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }

      ImGui::TableSetColumnIndex(2);
      if (ImGui::ColorPicker3("##fc2color", (float *) &fc2Color, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(fc2Color);
        EventManager::Dispatcher().trigger<Events::SetColor>({2, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }

      ImGui::TableSetColumnIndex(3);
      if (ImGui::ColorPicker3("##blcolor", (float *) &blColor, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(blColor);
        EventManager::Dispatcher().trigger<Events::SetColor>({3, color});
        EventManager::Dispatcher().enqueue(Events::SavePrefs());
      }
      ImGui::EndTable();

      ImGui::End();
    }
  }

  void UI::Code() {
    if (!ImGui::Begin("Code", &m_ShowCodeEditor)) {
      ImGui::End();
    } else {
      m_Editor.draw();
      ImGui::End();
    }
  }

  /*
   * UI Elements
   */
  void UI::DrawMenubar() {
    auto &bus = Bus::Get();

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open ROM...", nullptr)) {
          nfdchar_t *outPath = nullptr;

          EventManager::Dispatcher().trigger<Events::Reset>({});

          nfdresult_t result = NFD_OpenDialog("ch8,c8", nullptr, &outPath);

          switch (result) {
            case NFD_OKAY: {
              std::string path{outPath};
              m_PrevPC = 0;
              EventManager::Dispatcher().trigger(Events::LoadROM{path});
              delete outPath;
            }
              break;
            case NFD_CANCEL:
              break;
            case NFD_ERROR:
              spdlog::get("console")->error("{}", NFD_GetError());
              break;
          }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Unload ROM")) {
          m_PrevPC = 0;
          EventManager::Dispatcher().trigger(Events::UnloadROM{});
        }

        if (ImGui::MenuItem("Reset")) {
          m_PrevPC = 0;
          EventManager::Dispatcher().trigger(Events::Reset{});
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Quit", nullptr)) {
          EventManager::Dispatcher().trigger<Events::WantQuit>();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Mute Dorito", nullptr, &m_DoritoMuted)) {
          EventManager::Dispatcher().enqueue(Events::SetMute(m_DoritoMuted));
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Speed")) {
          for (auto &entry: m_CycleEntries) {
            if (entry.cycles == 0) {
              ImGui::Separator();
              continue;
            }

            if (ImGui::MenuItem(entry.label.c_str(), nullptr, &(entry.set))) {
              EventManager::Dispatcher().enqueue(Events::SetCycles(entry.cycles));
              EventManager::Dispatcher().enqueue(Events::SavePrefs());
            }
          }
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Compatibility")) {
          for (auto &entry: m_ProfileEntries) {
            if (ImGui::MenuItem(entry.label.c_str(), nullptr, &(entry.set))) {

              switch (entry.profile) {
                case Bus::CompatProfile::VIP:
                  EventManager::Dispatcher().enqueue<Events::VIPCompat>({});
                  break;
                case Bus::CompatProfile::SCHIP:
                  EventManager::Dispatcher().enqueue<Events::SCHIPCompat>({});
                  break;
                case Bus::CompatProfile::XOChip:
                  EventManager::Dispatcher().enqueue<Events::XOCompat>({});
                  break;
              }

              EventManager::Dispatcher().enqueue(Events::SavePrefs());
            }
          }

          ImGui::Separator();

          if (ImGui::BeginMenu("Quirks")) {
            for (auto &entry: m_QuirkEntries) {
              if (ImGui::MenuItem(entry.label.c_str(), nullptr, &(entry.set))) {
                EventManager::Dispatcher().enqueue<Events::SetQuirk>({entry.quirk, entry.set});
                EventManager::Dispatcher().enqueue(Events::SavePrefs());
              }
            }

            ImGui::EndMenu();
          }
          ImGui::EndMenu();
        }

        auto drawPalette = [](const std::vector<Color> &pal) {
          float sz = ImGui::GetTextLineHeight();
          ImVec2 p = ImGui::GetCursorScreenPos();

          for (auto i = 1; i <= 4; i++) {
            ImGui::GetWindowDrawList()->AddRectFilled(p,
                                                      ImVec2(p.x + sz, p.y + sz),
                                                      IM_COL32(
                                                          pal[i - 1].r,
                                                          pal[i - 1].g,
                                                          pal[i - 1].b,
                                                          pal[i - 1].a
                                                      ));

            p.x += sz + 4.0f;
          }
          ImGui::Dummy(ImVec2(sz * 5, sz));
          ImGui::SameLine();
        };

        auto &currentPalette = bus.m_Display.Palette();

        if (ImGui::BeginMenu("Colors")) {
          uint8_t index = 0;

          for (const auto &entry: m_PaletteEntries) {
            drawPalette(entry.palette);
            if (ImGui::MenuItem(entry.name.c_str(), nullptr, m_PaletteState[index++])) {
              EnablePalette(index - 1);
              EventManager::Dispatcher().trigger<Events::SetPalette>(Events::SetPalette{entry.palette});
              EventManager::Dispatcher().enqueue(Events::SavePrefs());
            }
          }

          ImGui::Separator();

          drawPalette(currentPalette);
          if (ImGui::MenuItem("Set Colors...", nullptr, m_PaletteState[8])) {
            m_ShowColorEditor = true;
            EnablePalette(8);
          }

          ImGui::EndMenu();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Development")) {
        ImGui::MenuItem("Code Editor", nullptr, &m_ShowCodeEditor);
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {
        if (ImGui::BeginMenu("CPU")) {
          ImGui::MenuItem("Registers", nullptr, &m_ShowRegisters);
          ImGui::MenuItem("Disassembly", nullptr, &m_ShowDisassembly);
          ImGui::EndMenu();
        }
        ImGui::MenuItem("Memory Viewer", nullptr, &m_ShowMemory);
        ImGui::MenuItem("Audio Viewer", nullptr, &m_ShowAudio);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, &m_ShowDemo);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

  }

} // dorito