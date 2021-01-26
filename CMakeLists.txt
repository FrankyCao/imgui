#
# Cmake for building imgui
#
cmake_minimum_required(VERSION 3.5.1)
project(IMGUI)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH ON)
set(CMAKE_MACOSX_RPATH 0)
if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
if (POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(WIN32) 
    add_definitions(-D_CRT_SECURE_NO_WARNINGS) 
endif() 

SET(SDK_CONFIG_DIR ${PROJECT_SOURCE_DIR}/config/)
CONFIGURE_FILE(Config.h.in Config.h)

include_directories( 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}
)

find_package(PkgConfig)

option(IMGUI_GL2        "Build ImGui OpenGL2 backends" ON)
option(IMGUI_GL3        "Build ImGui OpenGL3 backends" ON)
option(IMGUI_SDL2       "Build ImGui SDL2 backends" ON)
option(IMGUI_GLFW       "Build ImGui GLFW backends" ON)
option(IMGUI_GLUT       "Build ImGui GLUT backends" OFF)
option(IMGUI_GLEW       "Build ImGui GLEW backends" ON)
option(IMGUI_GLES       "Build ImGui GLES backends" ON)
option(IMGUI_MATAL      "Build ImGui Apple Metal backends" ON)
option(IMGUI_VULKAN     "Build ImGui Vulkan backends" ON)
option(IMGUI_DX9        "Build ImGui Direct9 backends(Windows only)" ON)
option(IMGUI_DX10       "Build ImGui Direct10 backends(Windows only)" ON)
option(IMGUI_DX11       "Build ImGui Direct11 backends(Windows only)" ON)
option(IMGUI_DX12       "Build ImGui Direct12 backends(Windows only)" ON)
option(IMGUI_FREETYPE   "Build ImGui with FreeType support" ON)
option(IMGUI_FONTS      "Internal Fonts build in library" ON)
option(IMGUI_ICONS      "Internal Icons build in library" ON)
option(IMGUI_APPS       "build apps base on imgui" ON)
option(IMGUI_APPLE_APP  "build apple app base on imgui" OFF)

if (IMGUI_GL2 OR IMGUI_GL3)
    find_package(OpenGL)
endif(IMGUI_GL2 OR IMGUI_GL3)

if (IMGUI_VULKAN)
    find_package(Vulkan)
endif(IMGUI_VULKAN)

if (IMGUI_FREETYPE)
    find_package(Freetype)
endif(IMGUI_FREETYPE)

if(PKG_CONFIG_FOUND)
    if(IMGUI_GLEW)
        pkg_search_module(GLEW REQUIRED glew)
    endif(IMGUI_GLEW)
    if(IMGUI_GLFW)
        pkg_search_module(GLFW REQUIRED glfw3)
    endif(IMGUI_GLFW)
    if(IMGUI_GLUT)
        pkg_search_module(GLUT REQUIRED glut)
    endif(IMGUI_GLUT)
    if(IMGUI_SDL2)
        pkg_search_module(SDL2 REQUIRED sdl2)
        pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image>=2.0.0)
    endif(IMGUI_SDL2)
endif(PKG_CONFIG_FOUND)

# base include
set(IMGUI_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# base library
set(LINK_LIBS)

if(OPENGL_FOUND)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${OPENGL_INCLUDE_DIR}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${OPENGL_LIBRARIES}
    )
endif(OPENGL_FOUND)

if(FREETYPE_FOUND)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${FREETYPE_LIBRARIES}
    )
endif(FREETYPE_FOUND)

# base sources
set(IMGUI_SRC
    imgui.cpp 
    imgui_demo.cpp 
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    fonts/mono_sarasa.cpp
    misc/cpp/imgui_stdlib.cpp
)

# base headers
set(IMGUI_INCS
    imgui.h
    imgui_internal.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imgui_user.h
    misc/cpp/imgui_stdlib.h
)

# Android
if(ANDROID AND IMGUI_GLES)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_sdl_es2.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_sdl_es2.h
    )
endif()

# OpenGL
if(OPENGL_FOUND)
    add_definitions(-DIMGUI_OPENGL)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_opengl2.cpp
        backends/imgui_impl_opengl3.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_opengl2.h
        backends/imgui_impl_opengl3.h
    )
endif(OPENGL_FOUND)

if(FREETYPE_FOUND)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        misc/freetype/imgui_freetype.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        misc/freetype/imgui_freetype.h
    )
endif(FREETYPE_FOUND)

# SDL2 support
if (IMGUI_SDL2 AND SDL2_FOUND)
    add_definitions(-DIMGUI_SDL2)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${SDL2_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${SDL2_STATIC_LDFLAGS}
        ${SDL2_IMAGE_STATIC_LDFLAGS}
    )
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_sdl.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_sdl.h
    )
endif(IMGUI_SDL2 AND SDL2_FOUND)

# GLEW support
if (IMGUI_GLEW AND GLEW_FOUND)
    add_definitions(-DIMGUI_GLEW)
    add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${GLEW_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${GLEW_LDFLAGS}
    )
endif(IMGUI_GLEW AND GLEW_FOUND)

# GLFW support
if (IMGUI_GLFW AND GLFW_FOUND)
    add_definitions(-DIMGUI_GLFW)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${GLFW_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${GLFW_LDFLAGS}
    )
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_glfw.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_glfw.h
    )
endif(IMGUI_GLFW AND GLFW_FOUND)

# GLUT support
if (IMGUI_GLUT AND GLUT_FOUND)
    add_definitions(-DIMGUI_GLUT)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${GLUT_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${GLUT_LDFLAGS}
    )
    if (NOT APPLE AND NOT LINUX)
        set(LINK_LIBS
            ${LINK_LIBS}
            -lGL
        )
    endif(NOT APPLE AND NOT LINUX)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_glut.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_glut.h
    )
    # disable apple GL deprecated warring
    if (APPLE)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wno-error=deprecated-declarations -Wno-deprecated-declarations -Wmicrosoft-enum-forward-reference")
    endif(APPLE)
endif(IMGUI_GLUT AND GLUT_FOUND)

# Vilkan support
if (IMGUI_VULKAN AND VULKAN_FOUND)
    add_definitions(-DIMGUI_VULKAN)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${Vulkan_INCLUDE_DIR}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${Vulkan_LIBRARY}
    )
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_vulkan.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_vulkan.h
    )
endif(IMGUI_VULKAN AND VULKAN_FOUND)

# Apple metal support
if (APPLE AND IMGUI_MATAL)
    add_definitions(-DIMGUI_MATAL)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_metal.mm
        backends/imgui_impl_osx.mm
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_metal.h
        backends/imgui_impl_osx.h
    )
endif()

if(WIN32)
    if(IMGUI_DX9)
        add_definitions(-DIMGUI_DX9)
        set(LINK_LIBS
            ${LINK_LIBS}
            d3d9
        )
        set(IMGUI_SRC
            ${IMGUI_SRC}
            backends/imgui_impl_dx9.cpp
        )
        set(IMGUI_INCS
            ${IMGUI_INCS}
            backends/imgui_impl_dx9.h
        )
    endif(IMGUI_DX9)
    if(IMGUI_DX10)
        add_definitions(-DIMGUI_DX10)
        set(LINK_LIBS
            ${LINK_LIBS}
            d3d10
        )
        set(IMGUI_SRC
            ${IMGUI_SRC}
            backends/imgui_impl_dx10.cpp
        )
        set(IMGUI_INCS
            ${IMGUI_INCS}
            backends/imgui_impl_dx10.h
        )
    endif(IMGUI_DX10)
    if(IMGUI_DX11)
        add_definitions(-DIMGUI_DX11)
        set(LINK_LIBS
            ${LINK_LIBS}
            d3d11
        )
        set(IMGUI_SRC
            ${IMGUI_SRC}
            backends/imgui_impl_dx11.cpp
        )
        set(IMGUI_INCS
            ${IMGUI_INCS}
            backends/imgui_impl_dx11.h
        )
    endif(IMGUI_DX11)
    if(IMGUI_DX12)
        add_definitions(-DIMGUI_DX12)
        set(LINK_LIBS
            ${LINK_LIBS}
            d3d12
        )
        set(IMGUI_SRC
            ${IMGUI_SRC}
            backends/imgui_impl_dx12.cpp
        )
        set(IMGUI_INCS
            ${IMGUI_INCS}
            backends/imgui_impl_dx12.h
        )
    endif(IMGUI_DX12)
    set(LINK_LIBS
        ${LINK_LIBS}
        d3dcompiler
        dxgi
    )
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_win32.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_win32.h
    )
endif(WIN32)

if (IMGUI_FONTS)
    add_definitions(-DIMGUI_INTERNAL_FONTS)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        fonts/mono_hei.cpp
        fonts/mono_kai.cpp
        fonts/mono_song.cpp
        fonts/mono_yuan.cpp
    )
endif(IMGUI_FONTS)

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
        icons/NumberTexture.cpp
    )
endif(IMGUI_ICONS)

set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype
)

# ImGuiHelper
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiHelper/ImGuiHelper.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiHelper/ImGuiHelper.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiHelper
)

# ImGuiFileSystem
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiFileSystem/ImGuiFileSystem.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiFileSystem/ImGuiFileSystem.h
    addon/ImGuiFileSystem/dirent_portable.h
)

if (NOT WIN32)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/ImGuiFileSystem/minizip/zip.c
        addon/ImGuiFileSystem/minizip/unzip.c
        addon/ImGuiFileSystem/minizip/ioapi.c
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/ImGuiFileSystem/minizip/zip.h
        addon/ImGuiFileSystem/minizip/unzip.h
        addon/ImGuiFileSystem/minizip/ioapi.h
        addon/ImGuiFileSystem/minizip/crypt.h
    )
endif(NOT WIN32)

set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiFileSystem
)

# ImGuiFileDialog
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiFileDialog/ImGuiFileDialog.cpp
    addon/ImGuiFileDialog/ImGuiFileDialogDemo.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiFileDialog/ImGuiFileDialog.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiFileDialog
)

# implot
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

# Progress Indicators
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

# Splitter Window
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

# Colorful Text Edit
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

# Memory Editor
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/MemoryEditor/imgui_memory_editor.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/MemoryEditor
)

# Simple Markdown
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/markdown
)

# Simple dock window
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

# Tab Window
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiTabWindow/ImGuiTabWindow.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiTabWindow/ImGuiTabWindow.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiTabWindow
)

# Simple nodes
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

# Node Graph Edit
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiNodeGraphEditor/ImGuiNodeGraphEditor.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiNodeGraphEditor/ImGuiNodeGraphEditor.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiNodeGraphEditor
)

# Simple TimeLine
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/Timeline/Timeline.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/Timeline/Timeline.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/Timeline
)

# KNob
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/imgui_knob/imgui_knob.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/imgui_knob/imgui_knob.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/imgui_knob
)

# HotKeys
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/HotKey/HotKey.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/HotKey
)

# Date Chooser
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiDateChooser/ImGuiDateChooser.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiDateChooser/ImGuiDateChooser.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiDateChooser
)

# Node Editor
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/imgui_node_editor/crude_json.cpp
    addon/imgui_node_editor/imgui_canvas.cpp
    addon/imgui_node_editor/imgui_node_editor_api.cpp
    addon/imgui_node_editor/imgui_node_editor.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/imgui_node_editor/crude_json.h
    addon/imgui_node_editor/imgui_bezier_math.h
    addon/imgui_node_editor/imgui_bezier_math.inl
    addon/imgui_node_editor/imgui_canvas.h
    addon/imgui_node_editor/imgui_extra_math.h
    addon/imgui_node_editor/imgui_extra_math.inl
    addon/imgui_node_editor/imgui_node_editor_internal.h
    addon/imgui_node_editor/imgui_node_editor_internal.inl
    addon/imgui_node_editor/imgui_node_editor.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/imgui_node_editor
)

# Various Controls
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuiVariousControls/ImGuiVariousControls.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuiVariousControls/ImGuiVariousControls.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiVariousControls
)

# ImGuizmo
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/ImGuizmo/ImGuizmo.cpp
    addon/ImGuizmo/ImCurveEdit.cpp
    addon/ImGuizmo/ImGradient.cpp
    addon/ImGuizmo/ImSequencer.cpp
    addon/ImGuizmo/ImGuizmoDemo.cpp
)
set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/ImGuizmo/ImGuizmo.h
    addon/ImGuizmo/ImCurveEdit.h
    addon/ImGuizmo/ImGradient.h
    addon/ImGuizmo/ImSequencer.h
    addon/ImGuizmo/ImZoomSlider.h
)
set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuizmo
)

# Addon Demo
set(IMGUI_SRC
    ${IMGUI_SRC}
    addon/addons_demo.cpp
)

set(IMGUI_INCS
    ${IMGUI_INCS}
    addon/addons_demo.h
)

# Application Framework
if (IMGUI_APPS)
    if (WIN32 AND (IMGUI_DX11 OR IMGUI_DX9))
        if (IMGUI_DX11)
            set(IMGUI_SRC
                ${IMGUI_SRC}
                apps/application/source/dx11/entry.cpp
            )
        else(IMGUI_DX11)
            set(IMGUI_SRC
                ${IMGUI_SRC}
                apps/application/source/dx9/entry.cpp
            )
        endif(IMGUI_DX11)
    elseif(VULKAN_FOUND AND IMGUI_VULKAN AND SDL2_FOUND AND IMGUI_SDL2)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            apps/application/source/vulkan/entry_sdl.cpp
        )
    elseif(VULKAN_FOUND AND IMGUI_VULKAN AND GLFW_FOUND AND IMGUI_GLFW)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            apps/application/source/vulkan/entry_glfw.cpp
        )
    elseif(SDL2_FOUND AND IMGUI_SDL2 AND IMGUI_OPENGL)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            apps/application/source/sdl/entry.cpp
        )
    elseif(GLFW_FOUND AND IMGUI_GLFW)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            apps/application/source/glfw/entry.cpp
        )
    elseif(GLUT_FOUND AND IMGUI_GLUT)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            apps/application/source/glut/entry.cpp
        )
    endif(WIN32 AND (IMGUI_DX11 OR IMGUI_DX9))
    set(IMGUI_INCS
        ${IMGUI_INCS}
        apps/application/include/application.h
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/application/include
    )
endif(IMGUI_APPS)

# Build library
include_directories(${IMGUI_INC_DIRS})
add_library(imgui ${IMGUI_SRC} ${IMGUI_INCS})
set_property(TARGET imgui PROPERTY POSITION_INDEPENDENT_CODE ON)

# Build Tools
if (APPLE AND IMGUI_MATAL)
    target_compile_options(
        imgui 
        PUBLIC "-fobjc-arc"
    )
endif(APPLE AND IMGUI_MATAL)

if (SDL2_FOUND AND IMGUI_SDL2)
    # Build utils
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
        ${LINK_LIBS}
    )
    # Build examples
    add_executable(
        example_sdl_opengl2
        test/example_sdl_opengl2.cpp
    )
    target_link_libraries(
        example_sdl_opengl2
        imgui
        ${LINK_LIBS}
    )
    add_executable(
        example_sdl_opengl3
        test/example_sdl_opengl3.cpp
    )
    target_link_libraries(
        example_sdl_opengl3
        imgui
        ${LINK_LIBS}
    )
    add_executable(
        example_sdl_rendering
        test/example_sdl_rendering.cpp
    )
    target_link_libraries(
        example_sdl_rendering
        imgui
        ${LINK_LIBS}
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
        ${LINK_LIBS}
    )
    # Build Android examples
    if(ANDROID AND IMGUI_GLES)
        add_executable(
            example_sdl_opengles2
            test/example_sdl_opengles2.cpp
        )
        target_link_libraries(
            example_sdl_opengles2
            imgui
            ${LINK_LIBS}
        )
    endif(ANDROID AND IMGUI_GLES)
    # Build Vulkan examples
    if (VULKAN_FOUND)
        add_executable(
            example_sdl_vulkan
            test/example_sdl_vulkan.cpp
        )
        target_link_libraries(
            example_sdl_vulkan
            imgui
            ${LINK_LIBS}
        )
    endif(VULKAN_FOUND)
endif(SDL2_FOUND AND IMGUI_SDL2)

if (VULKAN_FOUND AND GLFW_FOUND)
    add_executable(
        example_glfw_vulkan
        test/example_glfw_vulkan.cpp
    )
    target_link_libraries(
        example_glfw_vulkan
        imgui
        ${LINK_LIBS}
    )
endif(VULKAN_FOUND AND GLFW_FOUND)

if (IMGUI_GLUT AND GLUT_FOUND)
    add_executable(
        example_glut_opengl2
        test/example_glut_opengl2.cpp
    )
    target_link_libraries(
        example_glut_opengl2
        imgui
        ${LINK_LIBS}
    )
endif(IMGUI_GLUT AND GLUT_FOUND)

if (GLFW_FOUND AND IMGUI_GLFW)
    add_executable(
        example_glfw_opengl3
        test/example_glfw_opengl3.cpp
    )
    target_link_libraries(
        example_glfw_opengl3
        imgui
        ${LINK_LIBS}
    )
endif(GLFW_FOUND AND IMGUI_GLFW)

if (APPLE AND IMGUI_MATAL AND IMGUI_APPLE_APP)
    add_executable(
        example_apple_metal
        MACOSX_BUNDLE
        test/example_apple_metal.mm
        test/metal.icns
    )
    set_target_properties(
        example_apple_metal
        PROPERTIES
        OUTPUT_NAME "example_apple_metal"
        MACOSX_BUNDLE_EXECUTABLE_NAME "example_apple_metal"
        MACOSX_BUNDLE_BUNDLE_NAME "example_apple_metal"
        FOLDER "example_apple_metal"
        MACOSX_BUNDLE_ICON_FILE "metal"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_LONG_VERSION_STRING "sample 1.0"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/test/Info-macOS.plist.in
    )
    target_link_libraries(
        example_apple_metal
        imgui
        "-framework Metal"
        "-framework MetalKit"
        "-framework Cocoa"
    )
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/test/metal.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )
    # Handle the Storyboard ourselves
    if(NOT ${CMAKE_GENERATOR} MATCHES "^Xcode.*")
        # Compile the storyboard file with the ibtool.
        add_custom_command(TARGET example_apple_metal POST_BUILD
            COMMAND ibtool --errors --warnings --notices --output-format human-readable-text
                --compile ${CMAKE_CURRENT_BINARY_DIR}/example_apple_metal.app/Contents/Resources/MainMenu.storyboardc
                ${CMAKE_CURRENT_SOURCE_DIR}/test/MainMenu.storyboard
                COMMENT "Compiling storyboard"
        )
        # Fix up the library search path in the executable to find (loader) libraries in the bundle.
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(${CMAKE_INSTALL_PREFIX}/example_apple_metal.app \"\" \"\")
            " COMPONENT Runtime
        )
    endif()

    if (GLFW_FOUND)
        add_executable(
            example_glfw_metal
            test/example_glfw_metal.mm
        )
        target_link_libraries(
            example_glfw_metal
            imgui
            ${LINK_LIBS}
        )
    endif(GLFW_FOUND)
endif(APPLE AND IMGUI_MATAL AND IMGUI_APPLE_APP)

if(WIN32)
    if(IMGUI_DX9)
        add_executable(
            example_win32_directx9
            test/example_win32_directx9.cpp
        )
        target_link_libraries(
            example_win32_directx9
            imgui
            ${LINK_LIBS}
        )
    endif(IMGUI_DX9)
    if(IMGUI_DX10)
        add_executable(
            example_win32_directx10
            test/example_win32_directx10.cpp
        )
        target_link_libraries(
            example_win32_directx10
            imgui
            ${LINK_LIBS}
        )
    endif(IMGUI_DX10)
    if(IMGUI_DX11)
        add_executable(
            example_win32_directx11
            test/example_win32_directx11.cpp
        )
        target_link_libraries(
            example_win32_directx11
            imgui
            ${LINK_LIBS}
        )
    endif(IMGUI_DX11)
    if(IMGUI_DX12)
        add_executable(
            example_win32_directx12
            test/example_win32_directx12.cpp
        )
        target_link_libraries(
            example_win32_directx12
            imgui
            ${LINK_LIBS}
        )
    endif(IMGUI_DX12)
endif(WIN32)

if (IMGUI_APPS)
    add_executable(
        example_node_edit_simple
        test/example_node_edit_simple.cpp
    )
    target_link_libraries(
        example_node_edit_simple
        imgui
        ${LINK_LIBS}
    )

    add_executable(
        example_node_edit_basic_interaction
        test/example_node_edit_basic_interaction.cpp
    )
    target_link_libraries(
        example_node_edit_basic_interaction
        imgui
        ${LINK_LIBS}
    )

    add_executable(
        example_node_edit_canvas
        test/example_node_edit_canvas.cpp
    )
    target_link_libraries(
        example_node_edit_canvas
        imgui
        ${LINK_LIBS}
    )

    add_executable(
        example_application
        test/example_application.cpp
    )
    target_link_libraries(
        example_application
        imgui
        ${LINK_LIBS}
    )
endif(IMGUI_APPS)

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(IMGUI_LIBRARYS imgui ${LINK_LIBS} PARENT_SCOPE )
    set(IMGUI_INCLUDES ${IMGUI_INCS} PARENT_SCOPE )
    set(IMGUI_INCLUDE_DIRS ${IMGUI_INC_DIRS} PARENT_SCOPE )
endif()