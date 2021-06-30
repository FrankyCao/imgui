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
if (POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)
endif()
if(POLICY CMP0025)
    # reference from https://cmake.org/cmake/help/latest/policy/CMP0025.html
    cmake_policy(SET CMP0025 NEW)
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
SET(SDK_DOC_DIR ${PROJECT_SOURCE_DIR}/docs/)
CONFIGURE_FILE(Config.h.in Config.h)

include_directories( 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}
)

find_package(PkgConfig)

_OPTION(IMGUI_STATIC                "Build ImGui as static library" OFF)
_OPTION(IMGUI_BUILD_EXAMPLE         "Build ImGui example" OFF)
_OPTION(IMGUI_GL2                   "Build ImGui OpenGL2 backends" ON)
_OPTION(IMGUI_GL3                   "Build ImGui OpenGL3 backends" ON)
_OPTION(IMGUI_SDL2                  "Build ImGui SDL2 backends" ON)
_OPTION(IMGUI_GLFW                  "Build ImGui GLFW backends" ON)
_OPTION(IMGUI_GLUT                  "Build ImGui GLUT backends" OFF)
_OPTION(IMGUI_GLEW                  "Build ImGui GLEW backends" ON)
_OPTION(IMGUI_GLES                  "Build ImGui GLES backends" ON)
_OPTION(IMGUI_MATAL                 "Build ImGui Apple Metal backends(Apple only)" ON IF APPLE)
_OPTION(IMGUI_VULKAN                "Build ImGui Vulkan backends" ON)
_OPTION(IMGUI_DX9                   "Build ImGui Direct9 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_DX10                  "Build ImGui Direct10 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_DX11                  "Build ImGui Direct11 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_DX12                  "Build ImGui Direct12 backends(Windows only)" ON IF WIN32)
_OPTION(IMGUI_FREETYPE              "Build ImGui with FreeType support" OFF)
_OPTION(IMGUI_FONT_SARASA           "Internal Mono Sarasa Fonts build in library" ON)
_OPTION(IMGUI_FONT_HEI              "Internal Mono Hei Fonts build in library" ON)
_OPTION(IMGUI_FONT_KAI              "Internal Mono Kai Fonts build in library" OFF)
_OPTION(IMGUI_FONT_SONG             "Internal Mono Song Fonts build in library" OFF)
_OPTION(IMGUI_FONT_YUAN             "Internal Mono Yuan Fonts build in library" OFF)
_OPTION(IMGUI_ICONS                 "Internal Icons build in library" ON)
_OPTION(IMGUI_APPS                  "build apps base on imgui" ON)
_OPTION(IMGUI_APPLE_APP             "build apple app base on imgui(Apple only)" OFF IF APPLE)

_OPTION(IMGUI_ADDON_DEAR_WIDGETS    "build Dear Widgets addon" OFF)
_OPTION(IMGUI_ADDON_KNOB            "build ImGui Knob addon" OFF)
_OPTION(IMGUI_ADDON_DATE_CHOOSER    "build Date chooser addon" OFF)
_OPTION(IMGUI_ADDON_DOCK            "build ImGuiDock addon" OFF)
_OPTION(IMGUI_ADDON_FILE_DIALOG     "build ImGuiFileDialog addon" OFF)
_OPTION(IMGUI_ADDON_FILE_SYSTEM     "build ImGuiFileSystem addon" OFF)
_OPTION(IMGUI_ADDON_TABWINDOW       "build ImGuiTabWindow addon" OFF)
_OPTION(IMGUI_ADDON_ZMO             "build ImGuizmo addon" OFF)
_OPTION(IMGUI_ADDON_ZMOQUAT         "build ImGuiZMO.quat addon" OFF)
_OPTION(IMGUI_ADDON_IMNODES         "build imnodes base on imgui" OFF)
_OPTION(IMGUI_ADDON_NODE_GRAPH      "build node graph editor base on imgui" OFF)
_OPTION(IMGUI_ADDON_VARIOUS         "build Various Controls addon" OFF)
_OPTION(IMGUI_ADDON_IMPLOTS         "build Implots addon" OFF)
_OPTION(IMGUI_ADDON_MARKDOWN        "build ImGui Markdown addon" OFF)
_OPTION(IMGUI_ADDON_MEMORY_EDITOR   "build ImGui Memory editor addon" OFF)
_OPTION(IMGUI_ADDON_PROGRESSES      "build ImGui Progress indicators addon" OFF)
_OPTION(IMGUI_ADDON_TEXT_EDITOR     "build ImGui Text editor addon" OFF)
_OPTION(IMGUI_ADDON_TIMELINE        "build ImGui Timeline addon" OFF)
_OPTION(IMGUI_ADDON_TINYFILE        "build ImGui tiny file dialog addon" OFF)
_OPTION(IMGUI_ADDON_HOTKEY          "build ImGui Hot key addon" OFF)
_OPTION(IMGUI_VULKAN_SHADER         "Build ImGui With Vulkan compute support" ON)
_OPTION(IMGUI_VULKAN_BENCHMARK      "Build ImGui With Vulkan Shader Benchmark" OFF)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
find_package(OpenGL)

if (IMGUI_VULKAN)
    find_package(Vulkan)
endif(IMGUI_VULKAN)

if (IMGUI_FREETYPE)
    find_package(Freetype)
endif(IMGUI_FREETYPE)

if (IMGUI_GLUT)
    find_package(GLUT)
endif (IMGUI_GLUT)

if(PKG_CONFIG_FOUND)
    pkg_search_module(ZLIB REQUIRED zlib)
    if(ZLIB_FOUND)
        message("zlib found")
    endif(ZLIB_FOUND)
    if(IMGUI_GLEW)
        pkg_search_module(GLEW REQUIRED glew)
    endif(IMGUI_GLEW)
    if(IMGUI_GLFW)
        pkg_search_module(GLFW REQUIRED glfw3)
    endif(IMGUI_GLFW)
    if(IMGUI_GLUT AND NOT GLUT_FOUND)
        pkg_search_module(GLUT REQUIRED glut)
        if (GLUT_FOUND)
            message("glut found")
        endif (GLUT_FOUND)
    endif(IMGUI_GLUT AND NOT GLUT_FOUND)
    if(IMGUI_VULKAN AND NOT VULKAN_FOUND)
        pkg_search_module(VULKAN REQUIRED vulkan)
        if (VULKAN_FOUND)
            message("vulkan found")
            set (Vulkan_LIBRARY vulkan)
        endif(VULKAN_FOUND)
    endif(IMGUI_VULKAN AND NOT VULKAN_FOUND)
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
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image>=2.0.0)
    if(SDL2_FOUND)
        message("sdl2 found")
    endif()
endif(PKG_CONFIG_FOUND)
else()
set(IMGUI_STATIC ON CACHE BOOL "Enable Static ImGui Build by Force" FORCE)
set(IMGUI_OPENGL ON CACHE BOOL "Enable ImGui OpenGL by Force" FORCE)
set(IMGUI_SDL2 ON CACHE BOOL "Build ImGui SDL2 backends by Force" FORCE)
set(IMGUI_GL2 OFF CACHE BOOL "Build ImGui OpenGL2 backends by Force" FORCE)
set(IMGUI_GL3 ON CACHE BOOL "Build ImGui OpenGL3 backends by Force" FORCE)
set(IMGUI_VULKAN OFF CACHE BOOL "Disable ImGui Vulkan backends" FORCE)
set(IMGUI_VULKAN_SHADER OFF CACHE BOOL "Disable ImGui With Vulkan compute support" FORCE)
set(IMGUI_GLFW OFF CACHE BOOL "Disable ImGui GLFW support" FORCE)
set(IMGUI_GLEW OFF CACHE BOOL "Disable ImGui GLEW support" FORCE)
set(IMGUI_GLUT OFF CACHE BOOL "Disable ImGui GLUT support" FORCE)
set(IMGUI_MATAL OFF CACHE BOOL "Disable ImGui Apple Matal support" FORCE)
set(IMGUI_DX9 OFF CACHE BOOL "Disable ImGui Windows DX9 support" FORCE)
set(IMGUI_DX10 OFF CACHE BOOL "Disable ImGui Windows DX10 support" FORCE)
set(IMGUI_DX11 OFF CACHE BOOL "Disable ImGui Windows DX11 support" FORCE)
set(IMGUI_DX12 OFF CACHE BOOL "Disable ImGui Windows DX12 support" FORCE)
set(IMGUI_FREETYPE OFF CACHE BOOL "Disable ImGui with FreeType support" FORCE)
set(IMGUI_APPS OFF CACHE BOOL "Disable apps base on imgui" FORCE)
set(IMGUI_APPLE_APP OFF CACHE BOOL "Disable apple app base on imgui" FORCE)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=128MB -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1")
endif()

# base include
set(IMGUI_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# base library
set(LINK_LIBS)

if(OPENGL_FOUND)
    set(IMGUI_OPENGL ON)
    message(STATUS "    [ImGui with OpenGL support]")
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
    set(IMGUI_ENABLE_FREETYPE ON)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${FREETYPE_LIBRARIES}
    )
endif(IMGUI_FREETYPE AND FREETYPE_FOUND)

# base sources
set(IMGUI_SRC
    imgui.cpp 
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_helper.cpp
    fonts/SourceCodePro-Regular.cpp
    fonts/Monaco.cpp
    fonts/Cousine-Regular.cpp
    fonts/DroidSans.cpp
    fonts/Karla-Regular.cpp
    fonts/ProggyTiny.cpp
    fonts/ProggyClean.cpp
    fonts/Roboto-Medium.cpp
    misc/cpp/imgui_stdlib.cpp
    misc/cpp/dir_iterate.cpp
)

# base headers
set(IMGUI_INCS
    imgui.h
    imgui_internal.h
    imgui_helper.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imgui_user.h
    misc/cpp/imgui_stdlib.h
    misc/cpp/dir_iterate.h
    misc/stb_image.h
    misc/stb_image_write.h
    misc/DynObjectLoader.h
)

if(IMGUI_BUILD_EXAMPLE)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        imgui_demo.cpp 
    )
endif(IMGUI_BUILD_EXAMPLE)

if(WIN32)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        misc/cpp/dlfcn_win.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        misc/dirent_portable.h
        misc/cpp/dlfcn_win.h
    )
endif(WIN32)

if(FREETYPE_FOUND AND IMGUI_FREETYPE)
    message(STATUS "    [ImGui with Freetype support]")
    set(IMGUI_SRC
        ${IMGUI_SRC}
        misc/freetype/imgui_freetype.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        misc/freetype/imgui_freetype.h
    )
endif(FREETYPE_FOUND AND IMGUI_FREETYPE)

# GLEW Loader support
if (IMGUI_GLEW AND GLEW_FOUND)
    message(STATUS "    [ImGui with GLEW support]")
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

# SDL2 support
if(SDL2_FOUND)
    message(STATUS "    [ImGui with SDL2 support]")
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${SDL2_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${SDL2_STATIC_LDFLAGS}
        ${SDL2_IMAGE_STATIC_LDFLAGS}
    )
endif(SDL2_FOUND)

# Find Backend Rendering
set(BACKEND_RENDERING NONE)

# Vulkan support
if (IMGUI_VULKAN AND VULKAN_FOUND)
    message(STATUS "    [ImGui backend rendering with Vulkan]")
    set(IMGUI_RENDERING_VULKAN ON)
    set(BACKEND_RENDERING VULKAN)
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

# OpenGL
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
if(OPENGL_FOUND AND BACKEND_RENDERING MATCHES NONE)
    if(IMGUI_GL3)
        message(STATUS "    [ImGui backend rendering with OpenGL3]")
        set(IMGUI_RENDERING_GL3 ON)
        set(BACKEND_RENDERING OPENGL3)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            backends/imgui_impl_opengl3.cpp
        )
        set(IMGUI_INCS
            ${IMGUI_INCS}
            backends/imgui_impl_opengl3.h
        )
    elseif(IMGUI_GL2)
        message(STATUS "    [ImGui backend rendering with OpenGL2]")
        set(IMGUI_RENDERING_GL2 ON)
        set(BACKEND_RENDERING OPENGL2)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            backends/imgui_impl_opengl2.cpp
        )
        set(IMGUI_INCS
            ${IMGUI_INCS}
            backends/imgui_impl_opengl2.h
        )
    endif()
endif()
else()
    set(IMGUI_RENDERING_GL3 ON)
    set(BACKEND_RENDERING OPENGL3)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_opengl3.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_opengl3.h
    )
endif()

# DirectX support
if(WIN32 AND BACKEND_RENDERING MATCHES NONE)
    if(IMGUI_DX12)
        message(STATUS "    [ImGui backend rendering with DX12]")
        set(IMGUI_RENDERING_DX12 ON)
        set(BACKEND_RENDERING DX12)
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
    elseif(IMGUI_DX11)
        message(STATUS "    [ImGui backend rendering with DX11]")
        set(IMGUI_RENDERING_DX11 ON)
        set(BACKEND_RENDERING DX11)
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
    elseif(IMGUI_DX10)
        message(STATUS "    [ImGui backend rendering with DX10]")
        set(IMGUI_RENDERING_DX10 ON)
        set(BACKEND_RENDERING DX10)
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
    elseif(IMGUI_DX9)
        message(STATUS "    [ImGui backend rendering with DX9]")
        set(IMGUI_RENDERING_DX9 ON)
        set(BACKEND_RENDERING DX9)
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
    endif(IMGUI_DX12)
    if(NOT BACKEND_RENDERING MATCHES NONE)
        set(LINK_LIBS
            ${LINK_LIBS}
            d3dcompiler
            dxgi
        )
    endif()
endif()

# Apple metal support
if (APPLE AND IMGUI_MATAL AND BACKEND_RENDERING MATCHES NONE)
    message(STATUS "    [ImGui backend rendering with Matal]")
    set(IMGUI_RENDERING_MATAL ON)
    set(BACKEND_RENDERING MATAL)
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

# Find Backend platform
# SDL2 Support
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
if (IMGUI_SDL2 AND SDL2_FOUND)
    message(STATUS "    [ImGui backend platform with SDL2]")
    set(IMGUI_PLATFORM_SDL2 ON)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_sdl.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_sdl.h
    )
endif(IMGUI_SDL2 AND SDL2_FOUND)
else()
set(IMGUI_PLATFORM_SDL2 ON)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_sdl.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_sdl.h
    )
endif()

# GLFW support
if (IMGUI_GLFW AND GLFW_FOUND)
    message(STATUS "    [ImGui backend platform with GLFW]")
    set(IMGUI_PLATFORM_GLFW ON)
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
    message(STATUS "    [ImGui backend platform with GLUT]")
    set(IMGUI_PLATFORM_GLUT ON)
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${GLUT_INCLUDE_DIRS}
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${GLUT_LDFLAGS}
        ${GLUT_LIBRARY}
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
endif(IMGUI_GLUT AND GLUT_FOUND)

# Win32 Support
if(WIN32)
    message(STATUS "    [ImGui backend platform with Win32]")
    set(IMGUI_PLATFORM_WIN32 ON)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        backends/imgui_impl_win32.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        backends/imgui_impl_win32.h
    )
endif(WIN32)

# Add Fonts
if (IMGUI_FONT_HEI)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        fonts/mono_hei.cpp
    )
endif (IMGUI_FONT_HEI)
if (IMGUI_FONT_KAI)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        fonts/mono_kai.cpp
    )
endif (IMGUI_FONT_KAI)
if (IMGUI_FONT_SONG)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        fonts/mono_song.cpp
    )
endif (IMGUI_FONT_SONG)
if (IMGUI_FONT_YUAN)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        fonts/mono_yuan.cpp
    )
endif (IMGUI_FONT_YUAN)
if (IMGUI_FONT_SARASA)
    set(IMGUI_SRC
        ${IMGUI_SRC}
        fonts/mono_sarasa.cpp
    )
endif (IMGUI_FONT_SARASA)

# Add Icons
if (IMGUI_ICONS)
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
        icons/kenney.cpp
        icons/StepMath.cpp
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/icons
    )
endif(IMGUI_ICONS)

set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/misc
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype
)

if (IMGUI_ADDON_FILE_SYSTEM)
    # ImGuiFileSystem
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/ImGuiFileSystem/ImGuiFileSystem.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/ImGuiFileSystem/ImGuiFileSystem.h
    )

    if (NOT WIN32 AND ZLIB_FOUND)
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
    endif(NOT WIN32 AND ZLIB_FOUND)

    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiFileSystem
    )
endif(IMGUI_ADDON_FILE_SYSTEM)

if (IMGUI_ADDON_FILE_DIALOG)
    # ImGuiFileDialog
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
    if (IMGUI_BUILD_EXAMPLE)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            addon/ImGuiFileDialog/ImGuiFileDialogDemo.cpp
        )
    endif(IMGUI_BUILD_EXAMPLE)
endif(IMGUI_ADDON_FILE_DIALOG)

if (IMGUI_ADDON_TINYFILE)
    # TinyFileDialog
    if (IMGUI_BUILD_EXAMPLE)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            addon/tinyfiledialog/tinyfiledialogs.c
            addon/tinyfiledialog/tinyfiledialogs.h
        )
        set(IMGUI_INCS
            ${IMGUI_INCS}
            addon/tinyfiledialog/tinyfiledialogs.h
        )
        set(IMGUI_INC_DIRS
            ${IMGUI_INC_DIRS}
            ${CMAKE_CURRENT_SOURCE_DIR}/addon/tinyfiledialog
        )
    endif(IMGUI_BUILD_EXAMPLE)
endif(IMGUI_ADDON_TINYFILE)

if (IMGUI_ADDON_IMPLOTS)
    # implot
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/implot/implot.cpp
        addon/implot/implot_items.cpp
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
    if(IMGUI_BUILD_EXAMPLE)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            addon/implot/implot_demo.cpp
        )
    endif(IMGUI_BUILD_EXAMPLE)
endif(IMGUI_ADDON_IMPLOTS)

if (IMGUI_ADDON_PROGRESSES)
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
endif(IMGUI_ADDON_PROGRESSES)

if (IMGUI_ADDON_TEXT_EDITOR)
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
endif(IMGUI_ADDON_TEXT_EDITOR)

if (IMGUI_ADDON_MEMORY_EDITOR)
    # Memory Editor
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/MemoryEditor/imgui_memory_editor.h
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/MemoryEditor
    )
endif(IMGUI_ADDON_MEMORY_EDITOR)

if (IMGUI_ADDON_MARKDOWN)
    # Simple Markdown
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/markdown
    )
endif(IMGUI_ADDON_MARKDOWN)

if (IMGUI_ADDON_DOCK)
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
endif(IMGUI_ADDON_DOCK)

if (IMGUI_ADDON_TABWINDOW)
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
endif(IMGUI_ADDON_TABWINDOW)

    
if (IMGUI_ADDON_IMNODES)
    # Simple nodes
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/imnodes/imnodes.cpp
    )
    if (IMGUI_BUILD_EXAMPLE)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            addon/imnodes/imnodes_demo.cpp
        )
    endif(IMGUI_BUILD_EXAMPLE)
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/imnodes/imnodes.h
        addon/imnodes/imnodes_internal.h
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/imnodes
    )
endif(IMGUI_ADDON_IMNODES)

    
if (IMGUI_ADDON_NODE_GRAPH)
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
endif(IMGUI_ADDON_NODE_GRAPH)

if (IMGUI_ADDON_TIMELINE)
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
endif(IMGUI_ADDON_TIMELINE)

if (IMGUI_ADDON_KNOB)
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
endif(IMGUI_ADDON_KNOB)

if (IMGUI_ADDON_HOTKEY)
    # HotKeys
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/HotKey/HotKey.h
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/HotKey
    )
endif(IMGUI_ADDON_HOTKEY)

if (IMGUI_ADDON_DATE_CHOOSER)
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
endif(IMGUI_ADDON_DATE_CHOOSER)

if (IMGUI_ADDON_VARIOUS)
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
endif(IMGUI_ADDON_VARIOUS)

if (IMGUI_ADDON_ZMO)
    # ImGuizmo
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/ImGuizmo/ImGuizmo.cpp
        addon/ImGuizmo/ImCurveEdit.cpp
        addon/ImGuizmo/ImGradient.cpp
        addon/ImGuizmo/ImSequencer.cpp
        addon/ImGuizmo/GraphEditor.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/ImGuizmo/ImGuizmo.h
        addon/ImGuizmo/ImCurveEdit.h
        addon/ImGuizmo/ImGradient.h
        addon/ImGuizmo/ImSequencer.h
        addon/ImGuizmo/ImZoomSlider.h
        addon/ImGuizmo/GraphEditor.h
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuizmo
    )
    if (IMGUI_BUILD_EXAMPLE)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            addon/ImGuizmo/ImGuizmoDemo.cpp
        )
    endif(IMGUI_BUILD_EXAMPLE)
endif(IMGUI_ADDON_ZMO)

if (IMGUI_ADDON_ZMOQUAT)
    # ImGuiQuat
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/ImGuiQuat/imGuIZMOquat.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/ImGuiQuat/imGuIZMOquat.h
        addon/ImGuiQuat/vgConfig.h
        addon/ImGuiQuat/vGizmo.h
        addon/ImGuiQuat/vGizmoMath.h
        addon/ImGuiQuat/vgMath.h
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImGuiQuat
    )
    if (IMGUI_BUILD_EXAMPLE)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            addon/ImGuiQuat/imGuIZMOquat_demo.cpp
        )
    endif(IMGUI_BUILD_EXAMPLE)
endif(IMGUI_ADDON_ZMOQUAT)

if (IMGUI_ADDON_DEAR_WIDGETS)
    # Dear Widgets
    set(IMGUI_SRC
        ${IMGUI_SRC}
        addon/DearWidgets/dear_widgets.cpp
    )
    set(IMGUI_INCS
        ${IMGUI_INCS}
        addon/DearWidgets/dear_widgets.h
        addon/DearWidgets/dear_widgets.hpp
    )
    set(IMGUI_INC_DIRS
        ${IMGUI_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/DearWidgets
    )
    if (IMGUI_BUILD_EXAMPLE)
        set(IMGUI_SRC
            ${IMGUI_SRC}
            addon/DearWidgets/dear_widgets_demo.cpp
        )
    endif(IMGUI_BUILD_EXAMPLE)
endif(IMGUI_ADDON_DEAR_WIDGETS)

if (IMGUI_VULKAN AND VULKAN_FOUND AND IMGUI_VULKAN_SHADER)
    # Vulkan Shader/Glslang
    option(BUILD_EXTERNAL "" OFF)
    option(ENABLE_SPVREMAPPER "" OFF)
    option(ENABLE_GLSLANG_BINARIES "" OFF)
    option(ENABLE_HLSL "" OFF)
    option(ENABLE_RTTI "" OFF)
    option(ENABLE_EXCEPTIONS "" OFF)
    option(ENABLE_OPT "" ON)
    option(ENABLE_PCH "" OFF)
    option(ENABLE_CTEST "" OFF)
    option(SKIP_GLSLANG_INSTALL "" ON)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/addon/ImVulkanShader/glslang)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_SIMULATE_ID MATCHES "MSVC"))
        target_compile_options(glslang PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
        target_compile_options(OGLCompiler PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
        target_compile_options(OSDependent PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
        target_compile_options(SPIRV PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
    endif()
    if(IMGUI_VULKAN_BENCHMARK)
        set(SHADER_BENCHMARK ON)
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
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImVulkanShader/glslang
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImVulkanShader/glslang/glslang/include
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImVulkanShader/glslang/glslang/Public
        ${CMAKE_CURRENT_SOURCE_DIR}/addon/ImVulkanShader/glslang/SPIRV
    )
    set(GLSLANG_LIBRARY
        glslang
        OGLCompiler
        OSDependent
        SPIRV
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${GLSLANG_LIBRARY}
    )
endif(IMGUI_VULKAN AND VULKAN_FOUND AND IMGUI_VULKAN_SHADER)

# Addon Demo
if (IMGUI_BUILD_EXAMPLE)
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
endif(IMGUI_BUILD_EXAMPLE)
if(ZLIB_FOUND)
    set(LINK_LIBS
        ${LINK_LIBS}
        ${ZLIB_LDFLAGS}
    )
endif(ZLIB_FOUND)

# Application Framework
set(IMGUI_APP_SRC)
if (IMGUI_APPS)
    if(BACKEND_RENDERING MATCHES VULKAN AND SDL2_FOUND AND IMGUI_SDL2)
        set(IMGUI_APPLICATION_RENDERING_VULKAN ON)
        set(IMGUI_APPLICATION_PLATFORM_SDL2 ON)
        message(STATUS "    [ImGui application platform with SDL2]")
        message(STATUS "    [ImGui application rendering with Vulkan]")
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/vulkan/entry_sdl.cpp
        )
    elseif(BACKEND_RENDERING MATCHES VULKAN AND GLFW_FOUND AND IMGUI_GLFW)
        set(IMGUI_APPLICATION_RENDERING_VULKAN ON)
        set(IMGUI_APPLICATION_PLATFORM_GLFW ON)
        message(STATUS "    [ImGui application platform with GLFW]")
        message(STATUS "    [ImGui application rendering with Vulkan]")
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/vulkan/entry_glfw.cpp
        )
    elseif(BACKEND_RENDERING MATCHES VULKAN AND WIN32)
        set(IMGUI_APPLICATION_RENDERING_VULKAN ON)
        set(IMGUI_APPLICATION_PLATFORM_WIN32 ON)
        message(STATUS "    [ImGui application platform with Win32]")
        message(STATUS "    [ImGui application rendering with Vulkan]")
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/vulkan/entry_win32.cpp
        )
    elseif(BACKEND_RENDERING MATCHES DX11)
        set(IMGUI_APPLICATION_RENDERING_DX11 ON)
        set(IMGUI_APPLICATION_PLATFORM_WIN32 ON)
        message(STATUS "    [ImGui application platform with Win32]")
        message(STATUS "    [ImGui application rendering with DX11]")
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/dx11/entry.cpp
        )
    elseif(BACKEND_RENDERING MATCHES DX9)
        set(IMGUI_APPLICATION_RENDERING_DX9 ON)
        set(IMGUI_APPLICATION_PLATFORM_WIN32 ON)
        message(STATUS "    [ImGui application platform with Win32]")
        message(STATUS "    [ImGui application rendering with Dx9]")
        set(IMGUI_APP_SRC
            ${IMGUI_APP_SRC}
            apps/application/source/dx9/entry.cpp
        )
    elseif(BACKEND_RENDERING MATCHES OPENGL3)
        set(IMGUI_APPLICATION_RENDERING_GL3 ON)
        if(IMGUI_SDL2 AND SDL2_FOUND)
            set(IMGUI_APPLICATION_PLATFORM_SDL2 ON)
            message(STATUS "    [ImGui application platform with SDL2]")
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/opengl3/entry_sdl.cpp
            )
        elseif(IMGUI_GLFW AND GLFW_FOUND)
            set(IMGUI_APPLICATION_PLATFORM_GLFW ON)
            message(STATUS "    [ImGui application platform with GLFW]")
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/opengl3/entry_glfw.cpp
            )
        elseif(WIN32)
            set(IMGUI_APPLICATION_PLATFORM_WIN32 ON)
            message(STATUS "    [ImGui application platform with Win32]")
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/opengl3/entry_win32.cpp
            )
        endif()
        message(STATUS "    [ImGui application rendering with OpenGL3]")
    elseif(BACKEND_RENDERING MATCHES OPENGL2)
        set(IMGUI_APPLICATION_RENDERING_GL2 ON)
        if(SDL2_FOUND AND IMGUI_SDL2)
            set(IMGUI_APPLICATION_PLATFORM_SDL2 ON)
            message(STATUS "    [ImGui application platform with SDL2]")
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/opengl2/entry_sdl.cpp
            )   
        elseif(IMGUI_GLFW AND GLFW_FOUND)
            set(IMGUI_APPLICATION_PLATFORM_GLFW ON)
            message(STATUS "    [ImGui application platform with GLFW]")
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/opengl2/entry_glfw.cpp
            )
        elseif(WIN32)
            set(IMGUI_APPLICATION_PLATFORM_WIN32 ON)
            message(STATUS "    [ImGui application platform with Win32]")
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/opengl2/entry_win32.cpp
            )
        elseif(IMGUI_GLUT AND GLUT_FOUND)
            set(IMGUI_APPLICATION_PLATFORM_GLUT ON)
            message(STATUS "    [ImGui application platform with GLUT]")
            set(IMGUI_APP_SRC
                ${IMGUI_APP_SRC}
                apps/application/source/opengl2/entry_glut.cpp
            )
        endif()
        message(STATUS "    [ImGui application rendering with OpenGL2]")
    else()
        message(WARNING "No Application Backend Found! IMGUI_APPS will be turned off.")
        set(IMGUI_APPS OFF)
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

if(NOT IMGUI_STATIC)
set(IMGUI_SHARED_LIBRARY ON)
endif(NOT IMGUI_STATIC)
# Build library
configure_file(imconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/imconfig.h)

set(IMGUI_INC_DIRS
    ${IMGUI_INC_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)
include_directories(${IMGUI_INC_DIRS})

if(IMGUI_STATIC)
set(LIBRARY STATIC)
else(IMGUI_STATIC)
set(LIBRARY SHARED)
endif(IMGUI_STATIC)

add_library(imgui ${LIBRARY} ${IMGUI_SRC} ${IMGUI_INCS})
set_property(TARGET imgui PROPERTY POSITION_INDEPENDENT_CODE ON)
if(NOT IMGUI_STATIC)
target_link_libraries(imgui ${LINK_LIBS})
endif(NOT IMGUI_STATIC)
# Build Tools
if (APPLE AND IMGUI_MATAL)
    target_compile_options(
        imgui 
        PUBLIC "-fobjc-arc"
    )
endif(APPLE AND IMGUI_MATAL)

if (IMGUI_BUILD_EXAMPLE AND SDL2_FOUND AND IMGUI_SDL2)
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
    if(BACKEND_RENDERING MATCHES OPENGL3)
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
    elseif(BACKEND_RENDERING MATCHES OPENGL2)
        add_executable(
            example_sdl_opengl2
            test/example_sdl_opengl2.cpp
        )
        target_link_libraries(
            example_sdl_opengl2
            imgui
            ${LINK_LIBS}
        )
    endif(BACKEND_RENDERING MATCHES OPENGL3)

    # Build Vulkan examples
    if (BACKEND_RENDERING MATCHES VULKAN)
        add_executable(
            example_sdl_vulkan
            test/example_sdl_vulkan.cpp
        )
        target_link_libraries(
            example_sdl_vulkan
            imgui
            ${LINK_LIBS}
        )
    endif(BACKEND_RENDERING MATCHES VULKAN)
endif(IMGUI_BUILD_EXAMPLE AND SDL2_FOUND AND IMGUI_SDL2)

if (IMGUI_BUILD_EXAMPLE AND GLFW_FOUND AND IMGUI_GLFW AND BACKEND_RENDERING MATCHES VULKAN)
    add_executable(
        example_glfw_vulkan
        test/example_glfw_vulkan.cpp
    )
    target_link_libraries(
        example_glfw_vulkan
        imgui
        ${LINK_LIBS}
    )
endif(IMGUI_BUILD_EXAMPLE AND GLFW_FOUND AND IMGUI_GLFW AND BACKEND_RENDERING MATCHES VULKAN)

if (IMGUI_BUILD_EXAMPLE AND IMGUI_GLUT AND GLUT_FOUND AND BACKEND_RENDERING MATCHES OPENGL2)
    add_executable(
        example_glut_opengl2
        test/example_glut_opengl2.cpp
    )
    target_link_libraries(
        example_glut_opengl2
        imgui
        ${LINK_LIBS}
    )
endif(IMGUI_BUILD_EXAMPLE AND IMGUI_GLUT AND GLUT_FOUND AND BACKEND_RENDERING MATCHES OPENGL2)

if (IMGUI_BUILD_EXAMPLE AND GLFW_FOUND AND IMGUI_GLFW AND BACKEND_RENDERING MATCHES OPENGL3)
    add_executable(
        example_glfw_opengl3
        test/example_glfw_opengl3.cpp
    )
    target_link_libraries(
        example_glfw_opengl3
        imgui
        ${LINK_LIBS}
    )
endif(IMGUI_BUILD_EXAMPLE AND GLFW_FOUND AND IMGUI_GLFW AND BACKEND_RENDERING MATCHES OPENGL3)

if (IMGUI_BUILD_EXAMPLE AND APPLE AND IMGUI_MATAL AND BACKEND_RENDERING MATCHES MATAL)
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

    if (IMGUI_GLFW AND GLFW_FOUND)
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
    endif(IMGUI_GLFW AND GLFW_FOUND)
endif(IMGUI_BUILD_EXAMPLE AND APPLE AND IMGUI_MATAL AND BACKEND_RENDERING MATCHES MATAL)

if(IMGUI_BUILD_EXAMPLE AND WIN32)
    if(BACKEND_RENDERING MATCHES DX12)
        add_executable(
            example_win32_directx12
            test/example_win32_directx12.cpp
        )
        target_link_libraries(
            example_win32_directx12
            imgui
            ${LINK_LIBS}
        )
    elseif(BACKEND_RENDERING MATCHES DX11)
        add_executable(
            example_win32_directx11
            test/example_win32_directx11.cpp
        )
        target_link_libraries(
            example_win32_directx11
            imgui
            ${LINK_LIBS}
        )
    elseif(BACKEND_RENDERING MATCHES DX10)
        add_executable(
            example_win32_directx10
            test/example_win32_directx10.cpp
        )
        target_link_libraries(
            example_win32_directx10
            imgui
            ${LINK_LIBS}
        )
    elseif(BACKEND_RENDERING MATCHES DX9)
        add_executable(
            example_win32_directx9
            test/example_win32_directx9.cpp
        )
        target_link_libraries(
            example_win32_directx9
            imgui
            ${LINK_LIBS}
        )
    endif()
endif(IMGUI_BUILD_EXAMPLE AND WIN32)

if (IMGUI_BUILD_EXAMPLE AND IMGUI_APPS)
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
endif(IMGUI_BUILD_EXAMPLE AND IMGUI_APPS)

if (IMGUI_BUILD_EXAMPLE AND ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(IMGUI_WEB_DIR ${PROJECT_BINARY_DIR}/web/)
    SET(IMGUI_SHELL_FILE ${PROJECT_SOURCE_DIR}/test/shell_minimal.html)
    add_executable(
        example_application
        test/example_emscripten_opengl3.cpp
    )
    target_link_libraries(
        example_application
        imgui
    )
    set_target_properties(
        example_application
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${IMGUI_WEB_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${IMGUI_WEB_DIR}
        OUTPUT_NAME "index"
        PREFIX ""
        SUFFIX ".html"
        LINK_FLAGS "--shell-file ${IMGUI_SHELL_FILE}"
    )
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(IMGUI_LIBRARYS imgui ${LINK_LIBS} PARENT_SCOPE )
    set(IMGUI_INCLUDES ${IMGUI_INCS} PARENT_SCOPE )
    set(IMGUI_INCLUDE_DIRS ${IMGUI_INC_DIRS} PARENT_SCOPE )
    set(IMGUI_APP_ENTRY_SRC imgui/${IMGUI_APP_SRC} PARENT_SCOPE )
endif()