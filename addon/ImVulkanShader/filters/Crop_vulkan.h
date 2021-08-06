#pragma once
#include <imgui.h>
#include "gpu.h"
#include "pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API Crop_vulkan
{
public:
    Crop_vulkan(int gpu = -1);
    ~Crop_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    virtual void crop(const ImMat& src, ImMat& dst, int _x, int _y, int _w, int _h) const;
    // input CPU Buffer and output to RGBA GPU buffer
    virtual void crop(const ImMat& src, VkMat& dst, int _x, int _y, int _w, int _h) const;
    // input GPU Buffer and output to RGBA CPU buffer
    virtual void crop(const VkMat& src, ImMat& dst, int _x, int _y, int _w, int _h) const;
    // input GPU Buffer and output to RGBA GPU buffer
    virtual void crop(const VkMat& src, VkMat& dst, int _x, int _y, int _w, int _h) const;

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