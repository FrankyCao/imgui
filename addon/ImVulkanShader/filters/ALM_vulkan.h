#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "im_mat.h"

namespace ImGui 
{
class VKSHADER_API ALM_vulkan
{
public:
    ALM_vulkan(int gpu = 0);
    ~ALM_vulkan();
    void SetParam(float _strength, float _bias = 0.7, float _gamma = 2.2);
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
    int rgba_map[4];
    float strength {0.5};
    float bias {0.7};
    float gamma {2.2};
    void upload_param(const VkMat& src, VkMat& dst);
};
} // namespace ImGui 