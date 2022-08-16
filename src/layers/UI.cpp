#include "UI.h"
#include "imgui_internal.h"

#include <zep.h>
#include <nfd.h>

#include "system/Bus.h"

namespace dorito {
  void UI::OnAttach() {
// Hack to use opengl3 backend for imgui
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
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    if (opt_fullscreen) {
      ImGui::SetNextWindowPos(viewport->WorkPos);
      ImGui::SetNextWindowSize(viewport->WorkSize);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
      dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Dockspace", nullptr, window_flags);

    if (!opt_padding)
      ImGui::PopStyleVar();

    if (opt_fullscreen)
      ImGui::PopStyleVar(2);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    auto cycles = bus.CyclesPerFrame();

    m_7Cycles = cycles == 7;
    m_15Cycles = cycles == 15;
    m_20Cycles = cycles == 20;
    m_30Cycles = cycles == 30;
    m_100Cycles = cycles == 100;
    m_200Cycles = cycles == 200;
    m_500Cycles = cycles == 500;
    m_1000Cycles = cycles == 1000;
    m_10000Cycles = cycles == 10000;

    auto quirks = bus.Quirks();

    m_ShiftQuirk = quirks[static_cast<uint8_t>(Chip8::Quirk::Shift)];
    m_LoadStoreQuirk = quirks[static_cast<uint8_t>(Chip8::Quirk::LoadStore)];
    m_JumpQuirk = quirks[static_cast<uint8_t>(Chip8::Quirk::Jump)];
    m_LogicQuirk = quirks[static_cast<uint8_t>(Chip8::Quirk::Logic)];
    m_ClipQuirk = quirks[static_cast<uint8_t>(Chip8::Quirk::Clip)];
    m_VBlankQuirk = quirks[static_cast<uint8_t>(Chip8::Quirk::VBlank)];
    m_IRegCarryQuirk = quirks[static_cast<uint8_t>(Chip8::Quirk::IRegCarry)];

    auto profile = bus.GetCompatProfile();

    m_VIPMode = profile == Bus::CompatProfile::VIP;
    m_SCHIPMode = profile == Bus::CompatProfile::SCHIP;
    m_XOMode = profile == Bus::CompatProfile::XOChip;

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open ROM...", nullptr)) {
          nfdchar_t *outPath = nullptr;
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

      if (ImGui::BeginMenu("Speed")) {
        if (ImGui::MenuItem("7 Cycles/Frame", nullptr, &m_7Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(7));
        }

        if (ImGui::MenuItem("15 Cycles/Frame", nullptr, &m_15Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(15));
        }

        if (ImGui::MenuItem("20 Cycles/Frame", nullptr, &m_20Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(20));
        }

        if (ImGui::MenuItem("30 Cycles/Frame", nullptr, &m_30Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(30));
        }

        if (ImGui::MenuItem("100 Cycles/Frame", nullptr, &m_100Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(100));
        }

        if (ImGui::MenuItem("200 Cycles/Frame", nullptr, &m_200Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(200));
        }

        if (ImGui::MenuItem("500 Cycles/Frame", nullptr, &m_500Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(500));
        }

        if (ImGui::MenuItem("1000 Cycles/Frame", nullptr, &m_1000Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(1000));
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Warp Factor 10k", nullptr, &m_10000Cycles)) {
          EventManager::Dispatcher().enqueue(Events::SetCycles(10000));
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Compatibility")) {
        if (ImGui::MenuItem("VIP", nullptr, &m_VIPMode)) {
          EventManager::Dispatcher().enqueue<Events::VIPCompat>({});
        }
        if (ImGui::MenuItem("SCHIP", nullptr, &m_SCHIPMode)) {
          EventManager::Dispatcher().enqueue<Events::SCHIPCompat>({});
        }
        if (ImGui::MenuItem("XO-Chip", nullptr, &m_XOMode)) {
          EventManager::Dispatcher().enqueue<Events::XOCompat>({});
        }
        ImGui::Separator();

        if (ImGui::BeginMenu("Quirks")) {
          if (ImGui::MenuItem("<<= and >>= modify vx in place and ignore vy", nullptr, &m_ShiftQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::Shift, m_ShiftQuirk});
          }

          if (ImGui::MenuItem("load and store operations leave i unchanged", nullptr, &m_LoadStoreQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::LoadStore, m_LoadStoreQuirk});
          }

          if (ImGui::MenuItem("4 high bits of target address determines the offset register of jump0 instead of v0.",
                              nullptr, &m_JumpQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::Jump, m_JumpQuirk});
          }

          if (ImGui::MenuItem("clear vF after vx |= vy, vx &= vy, and vx ^= vy",
                              nullptr, &m_LogicQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::Logic, m_LogicQuirk});
          }

          if (ImGui::MenuItem("clip sprites at screen edges instead of wrapping",
                              nullptr, &m_ClipQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::Clip, m_ClipQuirk});
          }

          if (ImGui::MenuItem("render sprites only in vblank",
                              nullptr, &m_VBlankQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::VBlank, m_VBlankQuirk});
          }

          if (ImGui::MenuItem("render height 0 sprites as 8x16 sprites in lores mode",
                              nullptr, &m_LoresQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::LoresSprites, m_LoresQuirk});
          }

          if (ImGui::MenuItem("set carry if I overflows, clear otherwise",
                              nullptr, &m_IRegCarryQuirk)) {
            EventManager::Dispatcher().enqueue<Events::SetQuirk>({Chip8::Quirk::IRegCarry, m_IRegCarryQuirk});
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

      auto &currentPallete = bus.m_Display.Palette();

      if (ImGui::BeginMenu("Colors")) {

        drawPalette(m_GreyPalette);
        if (ImGui::MenuItem("Grayscale", nullptr, &m_PalGrey)) {
          m_PalOcto = false;
          m_PalNeat = false;
          m_PalKesh = false;
          EventManager::Dispatcher().trigger<Events::SetPalette>(Events::SetPalette{m_GreyPalette});
        }

        drawPalette(m_NeatPalette);
        if (ImGui::MenuItem("Neatboy", nullptr, &m_PalNeat)) {
          m_PalOcto = false;
          m_PalGrey = false;
          m_PalKesh = false;
          EventManager::Dispatcher().trigger<Events::SetPalette>(Events::SetPalette{m_NeatPalette});
        }

        drawPalette(m_OctoPalette);
        if (ImGui::MenuItem("Octo", nullptr, &m_PalOcto)) {
          m_PalGrey = false;
          m_PalNeat = false;
          m_PalKesh = false;
          EventManager::Dispatcher().trigger<Events::SetPalette>(Events::SetPalette{m_OctoPalette});
        }

        drawPalette(m_KeshaPalette);
        if (ImGui::MenuItem("Kesha", nullptr, &m_PalKesh)) {
          m_PalOcto = false;
          m_PalNeat = false;
          m_PalGrey = false;
          EventManager::Dispatcher().trigger<Events::SetPalette>(Events::SetPalette{m_KeshaPalette});
        }

        ImGui::Separator();

        drawPalette(currentPallete);
        if (ImGui::MenuItem("Set Colors...")) {
          m_ShowColorEditor = true;
          m_PalGrey = false;
          m_PalOcto = false;
          m_PalNeat = false;
          m_PalKesh = false;
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

    // Display Windows
    {

      if (m_ShowDemo) {
        ImGui::ShowDemoWindow(&m_ShowDemo);
      }

      if (m_ShowMemory) {
        static MemoryEditor romViewer;

        romViewer.DrawWindow("Memory", &bus.m_Ram.m_Ram[0], 1024 * 64);
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

  /*
   * Windows
   */


  void UI::Viewport() {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

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
      }

      ImGui::TableSetColumnIndex(1);
      if (ImGui::ColorPicker3("##fc1color", (float *) &fc1Color, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(fc1Color);
        EventManager::Dispatcher().trigger<Events::SetColor>({1, color});
      }

      ImGui::TableSetColumnIndex(2);
      if (ImGui::ColorPicker3("##fc2color", (float *) &fc2Color, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(fc2Color);
        EventManager::Dispatcher().trigger<Events::SetColor>({2, color});
      }

      ImGui::TableSetColumnIndex(3);
      if (ImGui::ColorPicker3("##blcolor", (float *) &blColor, ImGuiColorEditFlags_NoSmallPreview)) {
        auto color = ImvecToColor(blColor);
        EventManager::Dispatcher().trigger<Events::SetColor>({3, color});
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


} // dorito