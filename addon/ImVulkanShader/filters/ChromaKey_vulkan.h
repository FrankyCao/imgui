#pragma once
#include <imgui.h>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include <imgui_mat.h>

#define FILTER_2DS_BLUR 0

namespace ImGui 
{
class IMGUI_API ChromaKey_vulkan
{
public:
    ChromaKey_vulkan(int gpu = -1);
    ~ChromaKey_vulkan();
    // 比较差异,确定使用亮度与颜色比例,值需大于0,值越大,亮度所占比例越大
    // lumaMask  {1.0f}
    // 需要扣除的颜色
    // chromaColor {}
    // 比较差异相差的最少值(少于这值会放弃alpha)
    // alphaCutoffMin {0.2f}
    // 比较后的alpha系数增亮
    // alphaScale {10.0f}
    // 比较后的alpha指数增亮
    // alphaExponent {0.1f}

    // input CPU Buffer and output to RGBA CPU buffer
    void filter(const ImMat& src, ImMat& dst,
                float lumaMask, ImVec4 chromaColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                bool alpha_only);
    // input CPU Buffer and output to RGBA GPU buffer
    void filter(const ImMat& src, VkMat& dst,
                float lumaMask, ImVec4 chromaColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                bool alpha_only);
    // input GPU Buffer and output to RGBA CPU buffer
    void filter(const VkMat& src, ImMat& dst,
                float lumaMask, ImVec4 chromaColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                bool alpha_only);
    // input GPU Buffer and output to RGBA GPU buffer
    void filter(const VkMat& src, VkMat& dst,
                float lumaMask, ImVec4 chromaColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                bool alpha_only);

public:
    const VulkanDevice* vkdev   {nullptr};
    Pipeline * pipe             {nullptr};
#if FILTER_2DS_BLUR
    Pipeline * pipe_blur_column {nullptr};
    Pipeline * pipe_blur_row    {nullptr};
#else
    Pipeline * pipe_blur        {nullptr};
    Pipeline * pipe_sharpen     {nullptr};
#endif
    Pipeline * pipe_despill     {nullptr};
    VkCompute * cmd             {nullptr};
    Option opt;

private:
    ImMat kernel;
    VkMat vk_kernel;
    int blurRadius      {1};
    int ksize           {3};
    float sigma         {0};
    void prepare_kernel();

private:
    
    void upload_param(const VkMat& src, VkMat& dst,
                    float lumaMask, ImVec4 chromaColor, 
                    float alphaCutoffMin, float alphaScale, float alphaExponent,
                    bool alpha_only);
};
} // namespace ImGui 