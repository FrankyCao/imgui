#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "immat.h"

namespace ImGui 
{
class VKSHADER_API Crop_vulkan
{
public:
    Crop_vulkan(int gpu = -1);
    ~Crop_vulkan();

    virtual void crop(const ImMat& src, ImMat& dst, int _x, int _y, int _w, int _h) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    std::vector<uint32_t> spirv_data;
    void upload_param(const VkMat& src, VkMat& dst, int _x, int _y, int _w, int _h) const;
};
} // namespace ImGui 