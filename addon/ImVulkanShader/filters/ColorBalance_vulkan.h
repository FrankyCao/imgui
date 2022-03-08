#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "immat.h"

namespace ImGui 
{
class VKSHADER_API ColorBalance_vulkan
{
public:
    ColorBalance_vulkan(int gpu = -1);
    ~ColorBalance_vulkan();

    virtual void filter(const ImMat& src, ImMat& dst, std::vector<float>& shadows, std::vector<float>& midtones, std::vector<float>& highlights, bool preserve_lightness = false) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src, VkMat& dst, std::vector<float>& shadows, std::vector<float>& midtones, std::vector<float>& highlights, bool preserve_lightness) const;
};
} // namespace ImGui 