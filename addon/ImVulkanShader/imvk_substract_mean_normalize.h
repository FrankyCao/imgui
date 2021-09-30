#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"

namespace ImGui 
{
class IMGUI_API Substract_Mean_Normalize_vulkan
{
public:
    Substract_Mean_Normalize_vulkan(int gpu = -1);
    ~Substract_Mean_Normalize_vulkan();

    void forward(const ImMat& bottom_blob, ImMat& top_blob, ImVec4 mean_vals, ImVec4 norm_vals);
    void forward(const VkMat& bottom_blob, VkMat& top_blob, ImVec4 mean_vals, ImVec4 norm_vals);

public:
    const VulkanDevice* vkdev   {nullptr};
    Pipeline* pipe              {nullptr};
    VkCompute * cmd             {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, ImVec4 mean_vals, ImVec4 norm_vals);
};
} // namespace ImGui
