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

macro(_OPTION variable description value)
    set(__value ${value})
    set(__condition "")
    set(__varname "__value")
    foreach(arg ${ARGN})
        if(arg STREQUAL "IF" OR arg STREQUAL "if")
            set(__varname "__condition")
        else()
            list(APPEND ${__varname} ${arg})
        endif()
    endforeach()
    unset(__varname)
    if(__condition STREQUAL "")
        set(__condition 2 GREATER 1)
    endif()

    if(${__condition})
        if(__value MATCHES ";")
            if(${__value})
                option(${variable} "${description}" ON)
            else()
                option(${variable} "${description}" OFF)
            endif()
        elseif(DEFINED ${__value})
            if(${__value})
                option(${variable} "${description}" ON)
            else()
                option(${variable} "${description}" OFF)
            endif()
        else()
            option(${variable} "${description}" ${__value})
        endif()
    else()
        unset(${variable} CACHE)
    endif()
    unset(__condition)
    unset(__value)
endmacro()

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

_OPTION(IMGUI_GL2                "Build ImGui OpenGL2 backends" ON)
_OPTION(IMGUI_GL3                "Build ImGui OpenGL3 backends" ON)
_OPTION(IMGUI_SDL2               "Build ImGui SDL2 backends" ON)
_OPTION(IMGUI_GLFW               "Build ImGui GLFW backends" ON)
_OPTION(IMGUI_GLUT               "Build ImGui GLUT backends" OFF)
_OPTION(IMGUI_GLEW               "Build ImGui GLEW backends" ON)
_OPTION(IMGUI_GLES               "Build ImGui GLES backends" ON)
_OPTION(IMGUI_MATAL              "Build ImGui Apple Metal backends(Apple only)" ON IF APPLE)
_OPTION(IMGUI_VULKAN             "Build ImGui Vulkan backends" ON)
_OPTION(IMGUI_VULKAN_SHADER      "Build ImGui With Vulkan compute support" ON)
_OPTION(IMGUI_VULKAN_BENCHMARK   "Build ImGui With Vulkan Shader Benchmark" OFF)
_OPTION(IMGUI_DX9                "Build ImGui Direct9 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_DX10               "Build ImGui Direct10 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_DX11               "Build ImGui Direct11 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_DX12               "Build ImGui Direct12 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_FREETYPE           "Build ImGui with FreeType support" ON)
_OPTION(IMGUI_FONTS              "Internal Fonts build in library" OFF)
_OPTION(IMGUI_ICONS              "Internal Icons build in library" ON)
_OPTION(IMGUI_ADDONS             "build addons base on imgui" ON)
_OPTION(IMGUI_APPS               "build apps base on imgui" ON)
_OPTION(IMGUI_APPLE_APP          "build apple app base on imgui(Apple only)" OFF IF APPLE)

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
    if(NOT VULKAN_FOUND)
        pkg_search_module(VULKAN REQUIRED vulkan)
        if (VULKAN_FOUND)
            message("vulkan found")
            set (Vulkan_LIBRARY vulkan)
        endif()
    endif()
    if(NOT OPENGL_FOUND)
        pkg_search_module(OPENGL REQUIRED gl)
        if (OPENGL_FOUND)
            message("opengl found")
        endif()
    endif()
    if(NOT FREETYPE_FOUND)
        pkg_search_module(FREETYPE REQUIRED freetype2)
        if (FREETYPE_FOUND)
            message("freetype found")
        endif()
    endif()
    pkg_check_modules(FFMPEG IMPORTED_TARGET
        libavcodec
        libavformat
        libavutil
        libswresample
        libavfilter
        libswscale
        libavdevice
    )
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

if(IMGUI_FREETYPE AND FREETYPE_FOUND)
    #add_definitions(-DIMGUI_ENABLE_FREETYPE)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${FREETYPE_LIBRARIES}
    )
endif(IMGUI_FREETYPE AND FREETYPE_FOUND)

if (FFMPEG_FOUND)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${FFMPEG_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        PkgConfig::FFMPEG
    )
endif(FFMPEG_FOUND)

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
    misc/stb_image.h
    misc/stb_image_write.h
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
    )
endif(IMGUI_ICONS)

set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/misc
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype
)

if(IMGUI_ADDONS)
    add_definitions(-DIMGUI_ADDONS)
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

    if (IMGUI_VULKAN AND VULKAN_FOUND AND IMGUI_VULKAN_SHADER)
        if(WIN32)
            set(GLSLANG_TARGET_DIR "C:\\msys64\\mingw64\\lib\\cmake" CACHE PATH "Absolute path to glslangTargets.cmake directory")
        elseif(LINUX AND NOT APPLE)
            set(GLSLANG_TARGET_DIR "/usr/lib/x86_64-linux-gnu/cmake/glslang")
        elseif(DEFINED ENV{PREFIX})
            set(GLSLANG_TARGET_DIR $ENV{PREFIX}/lib/cmake CACHE PATH "Absolute path to glslangTargets.cmake directory")
            message(STATUS "found ENV at ${GLSLANG_TARGET_DIR}")
        else()
            set(GLSLANG_TARGET_DIR "/usr/local/lib/cmake" CACHE PATH "Absolute path to glslangTargets.cmake directory")
        endif()
        if(NOT GLSLANG_TARGET_DIR AND NOT DEFINED ENV{GLSLANG_TARGET_DIR})
            message(WARNING "GLSLANG_TARGET_DIR must be defined! IMGUI_VULKAN_SHADER will be turned off.")
            set(IMGUI_VULKAN_SHADER OFF)
        else()
            message(STATUS "Using glslang install located at ${GLSLANG_TARGET_DIR}")
            find_package(Threads)
            include("${GLSLANG_TARGET_DIR}/OSDependentTargets.cmake")
            include("${GLSLANG_TARGET_DIR}/OGLCompilerTargets.cmake")
            if(EXISTS "${GLSLANG_TARGET_DIR}/HLSLTargets.cmake")
                # hlsl support can be optional
                include("${GLSLANG_TARGET_DIR}/HLSLTargets.cmake")
            endif()
            include("${GLSLANG_TARGET_DIR}/glslangTargets.cmake")
            include("${GLSLANG_TARGET_DIR}/SPIRVTargets.cmake")

            if (NOT TARGET glslang OR NOT TARGET SPIRV)
                message(WARNING "glslang or SPIRV target not found! IMGUI_VULKAN_SHADER will be turned off.")
                set(IMGUI_VULKAN_SHADER OFF)
            else()
                add_definitions(-DIMGUI_VULKAN_SHADER)
                if(IMGUI_VULKAN_BENCHMARK)
                    add_definitions(-DSHADER_BENCHMARK)
                endif(IMGUI_VULKAN_BENCHMARK)
                set(IMGUI_SRC
                    ${IMGUI_SRC}
                    addon/ImVulkanShader/option.cpp
                    addon/ImVulkanShader/allocator.cpp
                    addon/ImVulkanShader/gpu.cpp
                    addon/ImVulkanShader/command.cpp
                    addon/ImVulkanShader/colorspace_table.cpp
                    addon/ImVulkanShader/pipeline.cpp
                    addon/ImVulkanShader/pipelinecache.cpp
                    addon/ImVulkanShader/Cast_vulkan.cpp
                    addon/ImVulkanShader/Packing_vulkan.cpp
                    addon/ImVulkanShader/ColorConvert_vulkan.cpp
                    addon/ImVulkanShader/Resize_vulkan.cpp
                    addon/ImVulkanShader/ImVulkanShader.cpp
                )
                set(IMGUI_INCS
                    ${IMGUI_INCS}
                    addon/ImVulkanShader/platform.h
                    addon/ImVulkanShader/vulkan_header_fix.h
                    addon/ImVulkanShader/option.h
                    addon/ImVulkanShader/allocator.h
                    addon/ImVulkanShader/gpu.h
                    addon/ImVulkanShader/command.h
                    addon/ImVulkanShader/buffer.h
                    addon/ImVulkanShader/pipeline.h
                    addon/ImVulkanShader/pipelinecache.h
                    addon/ImVulkanShader/Cast_shader.h
                    addon/ImVulkanShader/Cast_vulkan.h
                    addon/ImVulkanShader/Packing_shader.h
                    addon/ImVulkanShader/Packing_vulkan.h
                    addon/ImVulkanShader/ColorConvert_shader.h
                    addon/ImVulkanShader/ColorConvert_vulkan.h
                    addon/ImVulkanShader/Resize_shader.h
                    addon/ImVulkanShader/Resize_vulkan.h
                    addon/ImVulkanShader/ImVulkanShader.h
                )
                set(IMGUI_INC_DIRS
                    ${IMGUI_INC_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImVulkanShader
                )
                set(LINK_LIBS
                    glslang
                    SPIRV
                    ${LINK_LIBS}
                )
            endif()
        endif()
    endif(IMGUI_VULKAN AND VULKAN_FOUND AND IMGUI_VULKAN_SHADER)

    # Addon Demo
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/addons_demo.cpp
        addon/NumberTexture.cpp
    )

    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/addons_demo.h
        addon/NumberTexture.h
    )
endif(IMGUI_ADDONS)

# Application Framework
set(IMGUI_APP_SRC)
if (IMGUI_APPS)
    if(VULKAN_FOUND AND IMGUI_VULKAN AND SDL2_FOUND AND IMGUI_SDL2)
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/vulkan/entry_sdl.cpp
        )
    elseif(VULKAN_FOUND AND IMGUI_VULKAN AND GLFW_FOUND AND IMGUI_GLFW)
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/vulkan/entry_glfw.cpp
        )
    elseif (WIN32 AND (IMGUI_DX11 OR IMGUI_DX9))
        if (IMGUI_DX11)
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/dx11/entry.cpp
            )
        else(IMGUI_DX11)
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/dx9/entry.cpp
            )
        endif(IMGUI_DX11)
    elseif(GLFW_FOUND AND IMGUI_GLFW)
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/glfw/entry.cpp
        )
    elseif(SDL2_FOUND AND IMGUI_SDL2 AND IMGUI_OPENGL)
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/sdl/entry.cpp
        )
    elseif(GLUT_FOUND AND IMGUI_GLUT)
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/glut/entry.cpp
        )
    endif()
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
    if(IMGUI_ADDONS)
        add_executable(
            example_sdl_rendering
            test/example_sdl_rendering.cpp
        )
        target_link_libraries(
            example_sdl_rendering
            imgui
            ${LINK_LIBS}
        )
    endif(IMGUI_ADDONS)
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

if (VULKAN_FOUND AND GLFW_FOUND AND IMGUI_GLFW)
    add_executable(
        example_glfw_vulkan
        test/example_glfw_vulkan.cpp
    )
    target_link_libraries(
        example_glfw_vulkan
        imgui
        ${LINK_LIBS}
    )
endif(VULKAN_FOUND AND GLFW_FOUND AND IMGUI_GLFW)

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

if (APPLE AND IMGUI_MATAL)
    if (IMGUI_APPLE_APP)
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
            "-framework QuartzCore"
        )
        target_compile_options(
            example_apple_metal 
            PUBLIC "-fobjc-arc"
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
    endif(IMGUI_APPLE_APP)

    if (GLFW_FOUND)
        add_executable(
            example_glfw_metal
            test/example_glfw_metal.mm
        )
        target_compile_options(
            example_glfw_metal 
            PUBLIC "-fobjc-arc"
        )
        target_link_libraries(
            example_glfw_metal
            imgui
            ${LINK_LIBS}
            "-framework Metal"
            "-framework MetalKit"
            "-framework Cocoa"
            "-framework QuartzCore"
        )
    endif(GLFW_FOUND)
endif(APPLE AND IMGUI_MATAL)

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
    if (IMGUI_ADDONS)
        add_executable(
            example_node_edit_simple
            test/example_node_edit_simple.cpp
            ${IMGUI_APP_SRC}
        )
        target_link_libraries(
            example_node_edit_simple
            imgui
            ${LINK_LIBS}
        )

        add_executable(
            example_node_edit_basic_interaction
            test/example_node_edit_basic_interaction.cpp
            ${IMGUI_APP_SRC}
        )
        target_link_libraries(
            example_node_edit_basic_interaction
            imgui
            ${LINK_LIBS}
        )

        add_executable(
            example_node_edit_canvas
            test/example_node_edit_canvas.cpp
            ${IMGUI_APP_SRC}
        )
        target_link_libraries(
            example_node_edit_canvas
            imgui
            ${LINK_LIBS}
        )
    endif (IMGUI_ADDONS)
    add_executable(
        example_application
        test/example_application.cpp
        ${IMGUI_APP_SRC}
    )
    target_link_libraries(
        example_application
        imgui
        ${LINK_LIBS}
    )
    if (FFMPEG_FOUND AND IMGUI_ADDONS AND IMGUI_SDL2 AND SDL2_FOUND)
        add_executable(
            media_player
            apps/MediaPlayer/clock.cpp
            apps/MediaPlayer/clock.h
            apps/MediaPlayer/demux.cpp
            apps/MediaPlayer/demux.h
            apps/MediaPlayer/decoder.cpp
            apps/MediaPlayer/decoder.h
            apps/MediaPlayer/packet.cpp
            apps/MediaPlayer/packet.h
            apps/MediaPlayer/frame.cpp
            apps/MediaPlayer/frame.h
            apps/MediaPlayer/audio.cpp
            apps/MediaPlayer/audio.h
            apps/MediaPlayer/video.cpp
            apps/MediaPlayer/video.h
            apps/MediaPlayer/subtitle.cpp
            apps/MediaPlayer/subtitle.h
            apps/MediaPlayer/player.cpp
            apps/MediaPlayer/media_player.h
            apps/MediaPlayer/media_player.cpp
            ${IMGUI_APP_SRC}
        )
        target_link_libraries(
            media_player
            imgui
            ${LINK_LIBS}
        )
    endif(FFMPEG_FOUND AND IMGUI_ADDONS AND IMGUI_SDL2 AND SDL2_FOUND)
endif(IMGUI_APPS)

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(IMGUI_LIBRARYS imgui ${LINK_LIBS} PARENT_SCOPE )
    set(IMGUI_INCLUDES ${IMGUI_INCS} PARENT_SCOPE )
    set(IMGUI_INCLUDE_DIRS ${IMGUI_INC_DIRS} PARENT_SCOPE )
endif()