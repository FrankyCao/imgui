#
# Cmake for building ImVulkanShader
#
cmake_minimum_required(VERSION 3.5.1)
project(ImVulkanShader)
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
if (POLICY CMP0111)
    cmake_policy(SET CMP0111 NEW)
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(VKSHADER_VULKAN_BENCHMARK      "Enable Vulkan Shader Benchmark" OFF)
option(VKSHADER_STATIC                "Build Vulkan Shader as static library" OFF)

find_package(PkgConfig)
find_package(Vulkan)

if(PKG_CONFIG_FOUND)
    if(NOT VULKAN_FOUND)
        pkg_search_module(VULKAN REQUIRED vulkan)
        if (VULKAN_FOUND)
            message("[ImVulkanShader] vulkan found")
            set (Vulkan_LIBRARY vulkan)
        endif(VULKAN_FOUND)
    endif(NOT VULKAN_FOUND)
endif(PKG_CONFIG_FOUND)

set(LINK_LIBS)

set(VKSHADER_INC_DIRS
    ${Vulkan_INCLUDE_DIR}
)

set(LINK_LIBS
    ${LINK_LIBS}
    ${Vulkan_LIBRARY}
)

find_package(Glslang COMPONENTS Glslang SPIRV)
find_package(SpirvTools)
if(TARGET Glslang::Glslang)
    message(STATUS "VkShader Found system glslang library")
    set(GLSLANG_LIBRARY
        ${GLSLANG_LIBRARY}
        Glslang::Glslang
    )
endif()
if(TARGET Glslang::SPIRV)
    message(STATUS "VkShader Found system SPIRV library")
    set(GLSLANG_LIBRARY
        ${GLSLANG_LIBRARY}
        Glslang::SPIRV
    )
endif()
if (TARGET SpirvTools::SpirvTools)
    message(STATUS "VkShader Found system SPRIV-Tools library")
    set(GLSLANG_LIBRARY
        ${GLSLANG_LIBRARY}
        SpirvTools::SpirvTools
        SpirvTools::Opt
    )
endif()

if(VKSHADER_VULKAN_BENCHMARK)
    add_definitions(-DVULKAN_SHADER_BENCHMARK)
endif(VKSHADER_VULKAN_BENCHMARK)

set(VKSHADER_SRCS
    imvk_option.cpp
    imvk_allocator.cpp
    imvk_gpu.cpp
    imvk_command.cpp
    imvk_pipeline.cpp
    imvk_pipelinecache.cpp
    imvk_Cast_vulkan.cpp
    imvk_Packing_vulkan.cpp
    imvk_substract_mean_normalize.cpp
    imvk_copy_make_border.cpp
    ImVulkanShader.cpp
)
set(VKSHADER_INCS
    ${VKSHADER_INCS}
    imvk_platform.h
    imvk_vulkan_header_fix.h
    imvk_option.h
    imvk_allocator.h
    imvk_gpu.h
    imvk_command.h
    imvk_mat.h
    imvk_mat_shader.h
    imvk_image_mat.h
    imvk_pipeline.h
    imvk_pipelinecache.h
    imvk_Cast_shader.h
    imvk_Cast_vulkan.h
    imvk_Packing_shader.h
    imvk_Packing_vulkan.h
    imvk_substract_mean_normalize_shader.h
    imvk_substract_mean_normalize.h
    imvk_copy_make_border_shader.h
    imvk_copy_make_border.h
    ImVulkanShader.h
)
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    filters/AlphaBlending_vulkan.cpp
    filters/colorspace_table.cpp
    filters/ColorConvert_vulkan.cpp
    filters/Resize_vulkan.cpp
    filters/CopyTo_vulkan.cpp
    filters/Filter2D_vulkan.cpp
    filters/Filter2DS_vulkan.cpp
    filters/Flip_vulkan.cpp
    filters/Crop_vulkan.cpp
    filters/Transpose_vulkan.cpp
    filters/Brightness_vulkan.cpp
    filters/Contrast_vulkan.cpp
    filters/Exposure_vulkan.cpp
    filters/Gamma_vulkan.cpp
    filters/Saturation_vulkan.cpp
    filters/WhiteBalance_vulkan.cpp
    filters/Hue_vulkan.cpp
    filters/Vibrance_vulkan.cpp
    filters/ColorInvert_vulkan.cpp
    filters/ColorBalance_vulkan.cpp
    filters/Bilateral_vulkan.cpp
    filters/Sobel_vulkan.cpp
    filters/Canny_vulkan.cpp
    filters/Harris_vulkan.cpp
    filters/USM_vulkan.cpp
    filters/ChromaKey_vulkan.cpp
    filters/GaussianBlur.cpp
    filters/Box.cpp
    filters/Laplacian.cpp
    filters/Concat_vulkan.cpp
    filters/ALM_vulkan.cpp
    filters/CAS_vulkan.cpp
    filters/HQDN3D_vulkan.cpp
    filters/DeBand_vulkan.cpp
    filters/DeInterlace_vulkan.cpp
    filters/CIE_vulkan.cpp
    filters/Histogram_vulkan.cpp
    filters/Waveform_vulkan.cpp
    filters/Vector_vulkan.cpp
)
set(VKSHADER_INCS
    ${VKSHADER_INCS}
    filters/AlphaBlending_shader.h
    filters/AlphaBlending_vulkan.h
    filters/ColorConvert_shader.h
    filters/ColorConvert_vulkan.h
    filters/Resize_shader.h
    filters/Resize_vulkan.h
    filters/CopyTo_shader.h
    filters/CopyTo_vulkan.h
    filters/Filter2D_shader.h
    filters/Filter2D_vulkan.h
    filters/Filter2DS_shader.h
    filters/Filter2DS_vulkan.h
    filters/Flip_shader.h
    filters/Flip_vulkan.h
    filters/Crop_shader.h
    filters/Crop_vulkan.h
    filters/Transpose_shader.h
    filters/Transpose_vulkan.h
    filters/Brightness_shader.h
    filters/Brightness_vulkan.h
    filters/Contrast_shader.h
    filters/Contrast_vulkan.h
    filters/Exposure_shader.h
    filters/Exposure_vulkan.h
    filters/Gamma_shader.h
    filters/Gamma_vulkan.h
    filters/Saturation_shader.h
    filters/Saturation_vulkan.h
    filters/WhiteBalance_shader.h
    filters/WhiteBalance_vulkan.h
    filters/Hue_shader.h
    filters/Hue_vulkan.h
    filters/Vibrance_shader.h
    filters/Vibrance_vulkan.h
    filters/ColorInvert_shader.h
    filters/ColorInvert_vulkan.h
    filters/ColorBalance_shader.h
    filters/ColorBalance_vulkan.h
    filters/Bilateral_shader.h
    filters/Bilateral_vulkan.h
    filters/Sobel_shader.h
    filters/Sobel_vulkan.h
    filters/Canny_shader.h
    filters/Canny_vulkan.h
    filters/Harris_vulkan.h
    filters/Harris_shader.h
    filters/USM_shader.h
    filters/USM_vulkan.h
    filters/ChromaKey_shader.h
    filters/ChromaKey_vulkan.h
    filters/GaussianBlur.h
    filters/Box.h
    filters/Laplacian.h
    filters/Concat_shader.h
    filters/Concat_vulkan.h
    filters/ALM_shader.h
    filters/ALM_vulkan.h
    filters/CAS_shader.h
    filters/CAS_vulkan.h
    filters/HQDN3D_shader.h
    filters/HQDN3D_vulkan.h
    filters/DeBand_shader.h
    filters/DeBand_vulkan.h
    filters/DeInterlace_shader.h
    filters/DeInterlace_vulkan.h
    filters/CIE_shader.h
    filters/CIE_vulkan.h
    filters/Histogram_shader.h
    filters/Histogram_vulkan.h
    filters/Waveform_shader.h
    filters/Waveform_vulkan.h
    filters/Vector_shader.h
    filters/Vector_vulkan.h
)
set(VKSHADER_SRCS
    ${VKSHADER_SRCS}
    filters/lut3d/Lut3D.cpp
)
set(VKSHADER_INCS
    ${VKSHADER_INCS}
    filters/lut3d/Lut3D.h
    filters/lut3d/Lut3D_Shader.h
    filters/lut3d/SDR709_HDR2020_HLG.h
    filters/lut3d/SDR709_HDR2020_PQ.h
    filters/lut3d/HDR2020_HLG_SDR709.h
    filters/lut3d/HDR2020_PQ_SDR709.h
    filters/lut3d/HDR2020HLG_HDR2020PQ.h
    filters/lut3d/HDR2020PQ_HDR2020HLG.h
)
set(VKSHADER_INC_DIRS
    ${VKSHADER_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/filters
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/lut3d
)

include_directories(${VKSHADER_INC_DIRS})

if(VKSHADER_STATIC)
    set(LIBRARY STATIC)
    add_definitions(-DVKSHADER_STATIC_LIBRARY)
else(VKSHADER_STATIC)
    set(LIBRARY SHARED)
    add_definitions(-DVKSHADER_SHARED_LIBRARY)
endif(VKSHADER_STATIC)

add_library(
    VkShader
    ${LIBRARY}
    ${VKSHADER_SRCS}
    ${VKSHADER_INCS}
)

set(VKSHADER_VERSION_MAJOR 1)
set(VKSHADER_VERSION_MINOR 1)
set(VKSHADER_VERSION_PATCH 8)
set(VKSHADER_VERSION_STRING ${VKSHADER_VERSION_MAJOR}.${VKSHADER_VERSION_MINOR}.${VKSHADER_VERSION_PATCH})

if(NOT VKSHADER_STATIC)
set_property(TARGET VkShader PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(VkShader PROPERTIES VERSION ${VKSHADER_VERSION_STRING} SOVERSION ${VKSHADER_VERSION_MAJOR})
endif()
target_link_libraries(VkShader ${LINK_LIBS} ${GLSLANG_LIBRARY})

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(VKSHADER_LIBRARYS VkShader ${LINK_LIBS} PARENT_SCOPE )
    set(VKSHADER_INCLUDES ${VKSHADER_INCS} PARENT_SCOPE )
    set(VKSHADER_INCLUDE_DIRS ${VKSHADER_INC_DIRS} PARENT_SCOPE )
endif()