#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "im_mat.h"

namespace ImGui 
{
class VKSHADER_API WhiteBalance_vulkan
{
public:
    WhiteBalance_vulkan(int gpu = -1);
    ~WhiteBalance_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void filter(const ImMat& src, ImMat& dst, float temperature) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void filter(const ImMat& src, VkMat& dst, float temperature) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void filter(const VkMat& src, ImMat& dst, float temperature) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void filter(const VkMat& src, VkMat& dst, float temperature) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, float temperature) const;
};
} // namespace ImGui 