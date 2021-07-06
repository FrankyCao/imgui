#pragma once
#include "gpu.h"
#include "pipeline.h"

namespace ImVulkan 
{
class IMGUI_API ColorConvert_vulkan
{
public:
    ColorConvert_vulkan(int gpu = -1);
    ~ColorConvert_vulkan();

    // input from CPU buffer and output to RGBA8888 CPU buffer
    virtual void YUV2RGBA(const ImGui::ImMat& im_Y, const ImGui::ImMat& im_U, const ImGui::ImMat& im_V, ImGui::ImMat & im_RGB, ImGui::ImMatColorFormat color_format, ImGui::ImMatColorSpace color_space, ImGui::ImMatColorRange color_range, int video_depth, int video_shift) const;
    // input from CPU buffer and output to float RGBA GPU buffer
    virtual void YUV2RGBA(const ImGui::ImMat& im_Y, const ImGui::ImMat& im_U, const ImGui::ImMat& im_V, VkMat & im_RGB, ImGui::ImMatColorFormat color_format, ImGui::ImMatColorSpace color_space, ImGui::ImMatColorRange color_range, int video_depth, int video_shift) const;
    // input from CPU buffer and output to float RGBA GPU Image3D
    virtual void YUV2RGBA(const ImGui::ImMat& im_Y, const ImGui::ImMat& im_U, const ImGui::ImMat& im_V, VkImageMat & im_RGB, ImGui::ImMatColorFormat color_format, ImGui::ImMatColorSpace color_space, ImGui::ImMatColorRange color_range, int video_depth, int video_shift) const;

    // TODO::Dicky maybe need add GPU to GPU?
    // TODO::Dicky need add other convert

public:
    const VulkanDevice* vkdev;
    Pipeline * pipeline_yuv_rgb_8 = nullptr;
    Pipeline * pipeline_yuv_rgb_16 = nullptr;
    VkCompute * cmd = nullptr;
    Option opt;

public:
    std::vector<uint32_t> spirv_yuv_rgb_8;
    std::vector<uint32_t> spirv_yuv_rgb_16;
};
} // namespace ImVulkan 