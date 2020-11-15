#
# Cmake for building imgui
#

# build misc
find_package(Freetype REQUIRED)
find_package(SDL2 REQUIRED sdl2)
find_package(SDL2_image REQUIRED SDL2_image>=2.0.0)

set(IMGUI_INC_DIRS
    ${SDL2_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(IMGUI_SRC
    imgui.cpp 
    imgui_demo.cpp 
    imgui_draw.cpp
    imgui_widgets.cpp 
    monohei.cpp
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
    GLEW::glew
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
    GLEW::glew
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
    GLEW::glew
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
    GLEW::glew
)

set(IMGUI_LIBRARYS imgui ${LINK_LIBS} PARENT_SCOPE )
set(IMGUI_INCLUDES ${IMGUI_INCS} PARENT_SCOPE )
set(IMGUI_INCLUDE_DIRS ${IMGUI_INC_DIRS} PARENT_SCOPE )