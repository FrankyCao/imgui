#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "immat.h"

namespace ImGui 
{
class VKSHADER_API AlphaBlending_vulkan
{
public:
    AlphaBlending_vulkan(int gpu = -1);
    ~AlphaBlending_vulkan();

    // alpha blending src1 with src2 to dst, which dst is same size as src2, algorithm is 
    // src2.rgb * (1 - src1.a) + src1.rgb * src1.a, dst alpha is src2.a
    // src1 will copy to dst at (x,y), -src1.w < x src2.w -src1.h < y < src2.h 
    virtual void blend(const ImMat& src1, const ImMat& src2, ImMat& dst, int x = 0, int y = 0) const;

    // alpha blending src1 with src2 to dst, which dst is same size as src2, algorithm is
    // src2.rgb * (1 - alpha) + src1.rgb * src1.a * aplha, dst alpha is src2.a
    virtual void blend(const ImMat& src1, const ImMat& src2, ImMat& dst, float alpha, int x = 0, int y = 0) const;

public:
    const VulkanDevice* vkdev {nullptr};
    Pipeline * pipe           {nullptr};
    Pipeline * pipe_alpha     {nullptr};
    VkCompute * cmd           {nullptr};
    Option opt;

private:
    void upload_param(const VkMat& src1, const VkMat& src2, VkMat& dst, int x, int y) const;
    void upload_param(const VkMat& src1, const VkMat& src2, VkMat& dst, float alpha, int x, int y) const;
};
} // namespace ImGui 