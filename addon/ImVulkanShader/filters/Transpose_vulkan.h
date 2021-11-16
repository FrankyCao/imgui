#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "immat.h"

namespace ImGui 
{
class VKSHADER_API Transpose_vulkan
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
    void upload_param(const VkMat& src, VkMat& dst, bool bFlipX, bool bFlipY) const;
};
} // namespace ImGui 