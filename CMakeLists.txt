#
# Cmake for building imgui
#
cmake_minimum_required(VERSION 3.5.1)
project(IMGUI)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH ON)
set(CMAKE_MACOSX_RPATH 0)
if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(IMGUI_FONTS "Internal Fonts build in library" ON)
option(IMGUI_ICONS "Internal Icons build in library" ON)
option(IMGUI_APPS  "build apps base on imgui" ON)

find_package(PkgConfig REQUIRED)
if(PKG_CONFIG_FOUND)
pkg_search_module(GLEW REQUIRED glew)
endif()
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED sdl2)
find_package(SDL2_image REQUIRED SDL2_image>=2.0.0)
find_package(Freetype REQUIRED)

set(IMGUI_INC_DIRS
    ${SDL2_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(IMGUI_SRC
    imgui.cpp 
    imgui_demo.cpp 
    imgui_draw.cpp
    imgui_widgets.cpp 
    fonts/mono_hei.cpp
    backends/imgui_impl_sdl.cpp
    backends/imgui_impl_opengl2.cpp
    backends/imgui_impl_opengl3.cpp
    misc/cpp/imgui_stdlib.cpp
    misc/freetype/imgui_freetype.cpp
)

set(IMGUI_INCS
    imgui.h
    imgui_internal.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imgui_user.h
    backends/imgui_impl_sdl.h
    backends/imgui_impl_opengl2.h
    backends/imgui_impl_opengl3.h
    misc/cpp/imgui_stdlib.h
    misc/freetype/imgui_freetype.h
)

if (IMGUI_FONTS)
add_definitions(-DIMGUI_INTERNAL_FONTS)
set(IMGUI_SRC
    ${IMGUI_SRC}
    fonts/mono_sarasa.cpp
    fonts/mono_kai.cpp
    fonts/mono_song.cpp
    fonts/mono_yuan.cpp
)
endif()

if (IMGUI_ICONS)
add_definitions(-DIMGUI_INTERNAL_ICONS)
set(IMGUI_SRC
    ${IMGUI_SRC}
    icons/icons.cpp
    icons/folder.cpp
    icons/fontaudio.cpp
    icons/fa_brands.cpp
    icons/fa_regular.cpp
    icons/fa_solid.cpp
    icons/webfont.cpp
    icons/fork_webfont.cpp
    icons/material_icons.cpp
)
endif()

set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiFileDialog/ImGuiFileDialog.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiFileDialog/ImGuiFileDialog.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiFileDialog
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/implot/implot.cpp
    addon/implot/implot_items.cpp
    addon/implot/implot_demo.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/implot/implot.h
    addon/implot/implot_internal.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/implot
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/Spin/Spin.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/Spin/Spin.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/Spin
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ProgressIndicators/ProgressIndicators.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ProgressIndicators/ProgressIndicators.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ProgressIndicators
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/Splitter/Splitter.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/Splitter/Splitter.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/Splitter
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/TextEditor/TextEditor.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/TextEditor/TextEditor.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/TextEditor
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ToggleButton/ToggleButton.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ToggleButton/ToggleButton.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ToggleButton
)

set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/MemoryEditor/imgui_memory_editor.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/MemoryEditor
)

set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/markdown
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/imguiDock/imgui_dock.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/imguiDock/imgui_dock.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/imguiDock
)

set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/imnodes/imnodes.cpp
    addon/imnodes/imnodes_demo.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/imnodes/imnodes.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/imnodes
)

include_directories(${IMGUI_INC_DIRS})
add_library(imgui ${IMGUI_SRC} ${IMGUI_INCS})
set_property(TARGET imgui PROPERTY POSITION_INDEPENDENT_CODE ON)

add_executable(
    binary_to_compressed
    misc/fonts/binary_to_compressed_c.cpp
)

target_link_libraries(
    binary_to_compressed
    imgui
)

add_executable(
    img2c
    misc/tools/img2c.cpp
)

target_link_libraries(
    img2c
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_LIBRARY}
)

add_executable(
    example_sdl_opengl2
    test/example_sdl_opengl2.cpp
)

target_link_libraries(
    example_sdl_opengl2
    imgui
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_LIBRARY}
    ${GLEW_STATIC_LDFLAGS}
    ${OPENGL_LIBRARIES}
)

add_executable(
    example_sdl_opengl3
    test/example_sdl_opengl3.cpp
)

target_link_libraries(
    example_sdl_opengl3
    imgui
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_LIBRARY}
    ${GLEW_STATIC_LDFLAGS}
    ${OPENGL_LIBRARIES}
)

add_executable(
    example_sdl_rendering
    test/example_sdl_rendering.cpp
)

target_link_libraries(
    example_sdl_rendering
    imgui
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_LIBRARY}
    ${GLEW_STATIC_LDFLAGS}
    ${OPENGL_LIBRARIES}
)

find_package(OpenCV REQUIRED)
include_directories(
    ${OpenCV_INCLUDE_DIRS}
)
add_executable(
    example_sdl_video
    test/example_sdl_video.cpp
)
target_link_libraries(
    example_sdl_video
    imgui
    ${OpenCV_LIBS}
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_LIBRARY}
    ${GLEW_STATIC_LDFLAGS}
    ${OPENGL_LIBRARIES}
)

if (IMGUI_APPS)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
set(IMGUI_LIBRARYS imgui ${LINK_LIBS} PARENT_SCOPE )
set(IMGUI_INCLUDES ${IMGUI_INCS} PARENT_SCOPE )
set(IMGUI_INCLUDE_DIRS ${IMGUI_INC_DIRS} PARENT_SCOPE )
endif()