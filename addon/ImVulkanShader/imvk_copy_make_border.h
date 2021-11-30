#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include <vector>

namespace ImGui 
{
class VKSHADER_API Copy_Make_Border_vulkan
{
public:
    Copy_Make_Border_vulkan(int gpu = -1);
    ~Copy_Make_Border_vulkan();

    void forward(const ImMat& bottom_blob, ImMat& top_blob, int top, int bottom, int left, int right, float value);
    void forward(const VkMat& bottom_blob, VkMat& top_blob, int top, int bottom, int left, int right, float value);

public:
    const VulkanDevice* vkdev   {nullptr};
    Pipeline* pipe              {nullptr};
    VkCompute * cmd             {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, int top, int bottom, int left, int right, float value);
};
} // namespace ImGui
