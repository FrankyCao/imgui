#pragma once
#include <imgui.h>
#include "gpu.h"
#include "pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API Transpose_vulkan
{
public:
    Transpose_vulkan(int gpu = -1);
    ~Transpose_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void transpose(const ImMat& src, ImMat& dst, bool bFlipX, bool bFlipY) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void transpose(const ImMat& src, VkMat& dst, bool bFlipX, bool bFlipY) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void transpose(const VkMat& src, ImMat& dst, bool bFlipX, bool bFlipY) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void transpose(const VkMat& src, VkMat& dst, bool bFlipX, bool bFlipY) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    std::vector<uint32_t> spirv_data;
    void upload_param(const VkMat& src, VkMat& dst, bool bFlipX, bool bFlipY) const;
};
} // namespace ImGui 