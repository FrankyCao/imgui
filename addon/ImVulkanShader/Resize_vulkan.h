#pragma once
#include "gpu.h"
#include "pipeline.h"

namespace ImVulkan 
{
class Resize_vulkan
{
public:
    Resize_vulkan(int gpu = 0);
    ~Resize_vulkan();

    // input CPU Buffer and output to RGBA8888 CPU buffer
    virtual void Resize(const ImageBuffer& src, ImageBuffer& dst, float fx, float fy = 0.f, InterpolateMode type = INTERPOLATE_BICUBIC) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void Resize(const ImageBuffer& src, VkImageBuffer& dst, float fx, float fy = 0.f, InterpolateMode type = INTERPOLATE_BICUBIC) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void Resize(const VkImageBuffer& src, VkImageBuffer& dst, float fx, float fy = 0.f, InterpolateMode type = INTERPOLATE_BICUBIC) const;
    // input GPU Buffer and output to GPU Image3D
    virtual void Resize(const VkImageBuffer& src, VkImageMat& dst, float fx, float fy = 0.f, InterpolateMode type = INTERPOLATE_BICUBIC) const;
    // input CPU Buffer and output to GPU Image3D
    virtual void Resize(const ImageBuffer& src, VkImageMat& dst, float fx, float fy = 0.f, InterpolateMode type = INTERPOLATE_BICUBIC) const;

public:
    const VulkanDevice* vkdev;
    Pipeline * pipeline_rgb_8 = nullptr;
    Pipeline * pipeline_rgb_16 = nullptr;
    Pipeline * pipeline_rgb_f = nullptr;
    VkCompute * cmd = nullptr;
    Option opt;

public:
    std::vector<uint32_t> spirv_rgb_8;
    std::vector<uint32_t> spirv_rgb_16;
    std::vector<uint32_t> spirv_rgb_f;
};
} // namespace ImVulkan 