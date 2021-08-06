#pragma once
#include "gpu.h"
#include "pipeline.h"

namespace ImGui 
{
class IMGUI_API Resize_vulkan
{
public:
    Resize_vulkan(int gpu = -1);
    ~Resize_vulkan();

    // input CPU Buffer and output to RGBA8888 CPU buffer
    virtual void Resize(const ImMat& src, ImMat& dst, float fx, float fy = 0.f, ImInterpolateMode type = IM_INTERPOLATE_BICUBIC) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void Resize(const ImMat& src, VkMat& dst, float fx, float fy = 0.f, ImInterpolateMode type = IM_INTERPOLATE_BICUBIC) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void Resize(const VkMat& src, VkMat& dst, float fx, float fy = 0.f, ImInterpolateMode type = IM_INTERPOLATE_BICUBIC) const;
public:
    const VulkanDevice* vkdev;
    Pipeline * pipe = nullptr;
    VkCompute * cmd = nullptr;
    Option opt;

public:
    std::vector<uint32_t> spirv_data;
};
} // namespace ImGui 