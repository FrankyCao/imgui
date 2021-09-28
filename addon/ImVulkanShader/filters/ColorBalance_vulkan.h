#pragma once
#include <imgui.h>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API ColorBalance_vulkan
{
public:
    ColorBalance_vulkan(int gpu = -1);
    ~ColorBalance_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void filter(const ImMat& src, ImMat& dst, ImVec4& shadows, ImVec4& midtones, ImVec4& highlights, bool preserve_lightness = false) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void filter(const ImMat& src, VkMat& dst, ImVec4& shadows, ImVec4& midtones, ImVec4& highlights, bool preserve_lightness = false) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void filter(const VkMat& src, ImMat& dst, ImVec4& shadows, ImVec4& midtones, ImVec4& highlights, bool preserve_lightness = false) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void filter(const VkMat& src, VkMat& dst, ImVec4& shadows, ImVec4& midtones, ImVec4& highlights, bool preserve_lightness = false) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, ImVec4& shadows, ImVec4& midtones, ImVec4& highlights, bool preserve_lightness) const;
};
} // namespace ImGui 