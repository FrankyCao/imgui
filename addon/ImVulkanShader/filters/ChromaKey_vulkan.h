#pragma once
#include <imgui.h>
#include "gpu.h"
#include "pipeline.h"
#include <imgui_mat.h>

namespace ImGui 
{
class IMGUI_API ChromaKey_vulkan
{
public:
    ChromaKey_vulkan(int gpu = -1);
    ~ChromaKey_vulkan();

    // input CPU Buffer and output to RGBA CPU buffer
    void filter(const ImMat& src, ImMat& dst, 
                float lumaMask, ImVec4 chromaColor, ImVec4 ambientColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                float ambientScale, float despillScale, float despillExponent);
    // input CPU Buffer and output to RGBA GPU buffer
    void filter(const ImMat& src, VkMat& dst,
                float lumaMask, ImVec4 chromaColor, ImVec4 ambientColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                float ambientScale, float despillScale, float despillExponent);
    // input GPU Buffer and output to RGBA CPU buffer
    void filter(const VkMat& src, ImMat& dst,
                float lumaMask, ImVec4 chromaColor, ImVec4 ambientColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                float ambientScale, float despillScale, float despillExponent);
    // input GPU Buffer and output to RGBA GPU buffer
    void filter(const VkMat& src, VkMat& dst,
                float lumaMask, ImVec4 chromaColor, ImVec4 ambientColor,
                float alphaCutoffMin, float alphaScale, float alphaExponent,
                float ambientScale, float despillScale, float despillExponent);

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    std::vector<uint32_t> spirv_data;
    void upload_param(const VkMat& src, VkMat& dst,
                        float lumaMask, ImVec4 chromaColor, ImVec4 ambientColor,
                        float alphaCutoffMin, float alphaScale, float alphaExponent,
                        float ambientScale, float despillScale, float despillExponent);
};
} // namespace ImGui 