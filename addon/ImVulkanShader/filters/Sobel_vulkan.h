#pragma once
#include <imgui.h>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include <imgui_mat.h>
#include <string>

namespace ImGui
{
class IMGUI_API Sobel_vulkan
{
public:
    Sobel_vulkan(int gpu = 0);
    ~Sobel_vulkan();
    void SetParam(float _edgeStrength);
    void filter(const ImMat& src, ImMat& dst, float edgeStrength);
    void filter(const ImMat& src, VkMat& dst, float edgeStrength);
    void filter(const VkMat& src, ImMat& dst, float edgeStrength);
    void filter(const VkMat& src, VkMat& dst, float edgeStrength);
private:
    const VulkanDevice* vkdev   {nullptr};
    Option opt;
    Pipeline* pipe              {nullptr};
    VkCompute * cmd             {nullptr};

private:
    void upload_param(const VkMat& src, VkMat& dst, float edgeStrength);
};
} // namespace ImGui
