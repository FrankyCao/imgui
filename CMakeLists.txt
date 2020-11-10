#
# Cmake for building imgui
#

# build misc
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# subdirectories to build
include_directories(${SDL2_INCLUDE_DIR})
include_directories(ImGuiFileDialog)
include_directories(implot)

add_library(imgui 
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
    ImGuiFileDialog/ImGuiFileDialog.cpp
    implot/implot.cpp
    implot/implot_items.cpp
    implot/implot_demo.cpp
)

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
