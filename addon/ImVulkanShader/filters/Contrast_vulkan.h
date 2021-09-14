#pragma once
#include <imgui.h>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API Contrast_vulkan
{
public:
    Contrast_vulkan(int gpu = -1);
    ~Contrast_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void filter(const ImMat& src, ImMat& dst, float contrast) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void filter(const ImMat& src, VkMat& dst, float contrast) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void filter(const VkMat& src, ImMat& dst, float contrast) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void filter(const VkMat& src, VkMat& dst, float contrast) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, float contrast) const;
};
} // namespace ImGui 