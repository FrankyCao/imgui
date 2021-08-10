#pragma once
#include <imgui.h>
#include "gpu.h"
#include "pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API CopyTo_vulkan
{
public:
    CopyTo_vulkan(int gpu = -1);
    ~CopyTo_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void copyTo(const ImMat& src, ImMat& dst, int x, int y, float alpha = 1.f) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void copyTo(const ImMat& src, VkMat& dst, int x, int y, float alpha = 1.f) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void copyTo(const VkMat& src, ImMat& dst, int x, int y, float alpha = 1.f) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void copyTo(const VkMat& src, VkMat& dst, int x, int y, float alpha = 1.f) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    std::vector<uint32_t> spirv_data;
    void upload_param(const VkMat& src, VkMat& dst, int x, int y, float alpha) const;
};
} // namespace ImGui 