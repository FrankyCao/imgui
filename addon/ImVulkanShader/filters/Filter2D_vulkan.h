#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "im_mat.h"

namespace ImGui 
{
class VKSHADER_API Filter2D_vulkan
{
public:
    Filter2D_vulkan(int gpu = -1);
    ~Filter2D_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void filter(const ImMat& src, ImMat& dst) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void filter(const ImMat& src, VkMat& dst) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void filter(const VkMat& src, ImMat& dst) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void filter(const VkMat& src, VkMat& dst) const;

public:
    ImMat kernel;
    VkMat vk_kernel;
    int xksize;
    int yksize;
    int xanchor;
    int yanchor;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    std::vector<uint32_t> spirv_data;
    void upload_param(const VkMat& src, VkMat& dst) const;
};
} // namespace ImGui 