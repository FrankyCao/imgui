#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "immat.h"

namespace ImGui 
{
class VKSHADER_API DeInterlace_vulkan
{
public:
    DeInterlace_vulkan(int gpu = 0);
    ~DeInterlace_vulkan();
    void filter(const ImMat& src, ImMat& dst);
    void filter(const ImMat& src, VkMat& dst);
    void filter(const VkMat& src, ImMat& dst);
    void filter(const VkMat& src, VkMat& dst);

private:
    VulkanDevice* vkdev {nullptr};
    Option opt;
    Pipeline* pipe {nullptr};
    VkCompute * cmd {nullptr};

private:
    ImMat fCropTbl;
    VkMat vfCropTbl;
    void upload_param(const VkMat& src, VkMat& dst);
};
} // namespace ImGui 