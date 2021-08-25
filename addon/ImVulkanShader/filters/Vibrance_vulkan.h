#pragma once
#include <imgui.h>
#include "gpu.h"
#include "pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API Vibrance_vulkan
{
public:
    Vibrance_vulkan(int gpu = -1);
    ~Vibrance_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void filter(const ImMat& src, ImMat& dst, float vibrance) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void filter(const ImMat& src, VkMat& dst, float vibrance) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void filter(const VkMat& src, ImMat& dst, float vibrance) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void filter(const VkMat& src, VkMat& dst, float vibrance) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, float vibrance) const;
};
} // namespace ImGui 