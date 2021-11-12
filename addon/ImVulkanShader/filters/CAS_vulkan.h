#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "im_mat.h"

namespace ImGui 
{
class VKSHADER_API CAS_vulkan
{
public:
    CAS_vulkan(int gpu = 0);
    ~CAS_vulkan();
    void filter(const ImMat& src, ImMat& dst, float strength);
    void filter(const ImMat& src, VkMat& dst, float strength);
    void filter(const VkMat& src, ImMat& dst, float strength);
    void filter(const VkMat& src, VkMat& dst, float strength);

private:
    VulkanDevice* vkdev {nullptr};
    Option opt;
    Pipeline* pipe {nullptr};
    VkCompute * cmd {nullptr};

private:
    void upload_param(const VkMat& src, VkMat& dst, float strength);
};
} // namespace ImGui