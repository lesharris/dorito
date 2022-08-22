cmake_minimum_required(VERSION 3.13...3.22)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
endif ()

project(
    Dorito
    VERSION 0.5.0
    HOMEPAGE_URL https://lesharris.com/dorito
    LANGUAGES C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_SOURCE_DIR}/cmake/AddIconToBinary.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/AddFonts.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/AddPNG.cmake)

# Dependencies

# Fetch deps
find_package(raylib 4.2.0 QUIET)
if (NOT raylib_FOUND)
  include(FetchContent)
  FetchContent_Declare(
      raylib
      URL https://github.com/raysan5/raylib/archive/refs/tags/4.2.0.zip
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif ()
endif ()

find_package(zep QUIET)
if (NOT zep_FOUND)
  include(FetchContent)
  FetchContent_Declare(
      zep
      URL https://github.com/Rezonality/zep/archive/refs/heads/master.zip
  )
  FetchContent_GetProperties(zep)
  if (NOT zep_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(zep)
    set(BUILD_IMGUI ON CACHE BOOL "" FORCE)
    set(BUILD_DEMOS OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXTENSIONS OFF CACHE BOOL "" FORCE)
    add_subdirectory(${zep_SOURCE_DIR} ${zep_BINARY_DIR})
  endif ()
endif ()

# vcpkg deps
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(unofficial-nativefiledialog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)

# Source Files
set(SOURCE_FILES
    src/main.cpp
    src/core/Dorito.cpp
    src/core/Dorito.h
    src/common/common.h
    src/core/events/EventManager.h
    src/core/events/Event.h
    src/core/events/Events.h
    src/core/input/Input.cpp
    src/core/input/Input.h
    src/core/input/Keys.h
    src/core/input/InputActions.h
    src/core/input/Mapping.h
    src/core/layers/GameLayerStack.cpp
    src/core/layers/GameLayerStack.h
    src/core/layers/GameLayer.h
    src/layers/Emu.cpp
    src/layers/Emu.h
    src/layers/UI.cpp
    src/layers/UI.h
    src/external/imgui_extra/imgui_impl_glfw.cpp
    src/external/imgui/backends/imgui_impl_opengl3.cpp
    src/external/imgui/imgui_demo.cpp
    src/external/imgui/imgui_draw.cpp
    src/external/imgui/imgui_tables.cpp
    src/external/imgui/imgui_widgets.cpp
    src/external/imgui/imgui.cpp
    src/external/imgui_extra/imgui_memory_editor.h
    src/external/zep/ZepEditor.cpp
    src/external/zep/ZepEditor.h
    src/external/zep/mode_repl.cpp
    src/external/zep/mode_repl.h
    src/external/octo_compiler.h
    src/external/octo_compiler.c
    src/common/common.cpp
    src/cpu/Chip8.cpp
    src/cpu/Chip8.h
    src/cpu/Memory.cpp
    src/cpu/Memory.h
    src/system/Bus.cpp
    src/system/Bus.h
    src/display/Display.cpp
    src/display/Display.h
    src/common/Preferences.cpp
    src/common/Preferences.h
    src/code/ZepSyntaxOcto.cpp
    src/code/ZepSyntaxOcto.h
    src/widgets/EditorWidget.cpp
    src/widgets/EditorWidget.h
    src/widgets/Widget.h
    src/widgets/MainMenuWidget.cpp
    src/widgets/MainMenuWidget.h
    src/widgets/RegistersWidget.cpp
    src/widgets/RegistersWidget.h
    src/widgets/DisassemblyWidget.cpp
    src/widgets/DisassemblyWidget.h
    src/widgets/AudioWidget.cpp
    src/widgets/AudioWidget.h
    src/widgets/ColorEditorWidget.cpp
    src/widgets/ColorEditorWidget.h
    src/widgets/MemoryEditorWidget.cpp
    src/widgets/MemoryEditorWidget.h
    src/widgets/ViewportWidget.cpp
    src/widgets/ViewportWidget.h
    src/widgets/SoundEditorWidget.cpp
    src/widgets/SoundEditorWidget.h
    src/widgets/SpriteEditorWidget.cpp
    src/widgets/SpriteEditorWidget.h
    src/widgets/MonitorsWidget.cpp
    src/widgets/MonitorsWidget.h
    src/widgets/BreakpointsWidget.cpp
    src/widgets/BreakpointsWidget.h
    src/external/IconsFontAwesome5.h
    src/external/imgui-knobs.cpp)

if (APPLE)
  set(SOURCE_FILES ${SOURCE_FILES}
      src/external/mac/FolderManager.mm
      src/external/mac/FolderManager.h)
endif ()

# App Icon
AddIconToBinary(SOURCE_FILES ICONS ${CMAKE_SOURCE_DIR}/platform/dorito.ico ${CMAKE_SOURCE_DIR}/platform/dorito.icns)

# Dorito 🎉
if (MSVC)
  add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
elseif (APPLE)
  AddFont(SOURCE_FILES FONT ${CMAKE_SOURCE_DIR}/assets/fonts/CascadiaMono.ttf)
  AddFont(SOURCE_FILES FONT ${CMAKE_SOURCE_DIR}/assets/fonts/fa-solid-900.ttf)
  AddPNG(SOURCE_FILES PNG ${CMAKE_SOURCE_DIR}/assets/dorito.png)

  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES})

  set_target_properties(${PROJECT_NAME} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/platform/Info.plist.in"
      )
  #AddPlist(SOURCE_FILES PLIST ${CMAKE_SOURCE_DIR}/platform/Info.plist)

else ()
  # To do - Add a desktop file for an icon and install stuff?
  add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif ()

# Compile Options
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif ()

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else ()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif ()

# Generated Files
configure_file("src/common/config.h.in" "src/common/config.h")

# Include directories and libraries
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src/external
    ${PROJECT_SOURCE_DIR}/src/external/imgui
    ${PROJECT_SOURCE_DIR}/src/external/imgui/backends
    ${zep_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt::fmt
    raylib
    spdlog::spdlog
    EnTT::EnTT
    unofficial::nativefiledialog::nfd
    nlohmann_json::nlohmann_json
    Zep::Zep
    scn::scn)

if (APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif ()

