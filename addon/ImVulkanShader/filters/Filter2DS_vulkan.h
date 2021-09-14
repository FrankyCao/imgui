#pragma once
#include <imgui.h>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API Filter2DS_vulkan
{
public:
    Filter2DS_vulkan(int gpu = -1);
    ~Filter2DS_vulkan();

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
    const VulkanDevice* vkdev   {nullptr};
    Pipeline * pipe_column      {nullptr};
    Pipeline * pipe_row         {nullptr};
    VkCompute * cmd             {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst) const;
};
} // namespace ImGui 
