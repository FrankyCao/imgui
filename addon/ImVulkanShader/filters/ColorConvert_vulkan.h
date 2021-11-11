#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"

namespace ImGui 
{
class VKSHADER_API ColorConvert_vulkan
{
public:
    ColorConvert_vulkan(int gpu = -1);
    ~ColorConvert_vulkan();

    // input YUV planer from CPU buffer and output to RGBA8888 CPU buffer
    virtual void YUV2RGBA(const ImMat& im_YUV, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input YUV planer from CPU buffer and output to float RGBA GPU buffer
    virtual void YUV2RGBA(const ImMat& im_YUV, VkMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input YUV planer from GPU buffer and output to float RGBA CPU buffer
    virtual void YUV2RGBA(const VkMat& im_YUV, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input YUV planer from GPU buffer and output to float RGBA GPU buffer
    virtual void YUV2RGBA(const VkMat& im_YUV, VkMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input YUV planer from CPU buffer and output to float RGBA GPU Image3D
    virtual void YUV2RGBA(const ImMat& im_YUV, VkImageMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;

    // input RGBA planer from CPU buffer and output to YUV CPU buffer
    virtual void RGBA2YUV(const ImMat& im_RGB, ImMat & im_YUV, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const;
    // input RGBA planer from CPU buffer and output to YUV GPU buffer
    virtual void RGBA2YUV(const ImMat& im_RGB, VkMat & im_YUV, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const;
    // input RGBA planer from GPU buffer and output to YUV CPU buffer
    virtual void RGBA2YUV(const VkMat& im_RGB, ImMat & im_YUV, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const;
    // input RGBA planer from GPU buffer and output to YUV GPU buffer
    virtual void RGBA2YUV(const VkMat& im_RGB, VkMat & im_YUV, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const;

    // input Gray from CPU buffer and output to RGBA CPU buffer
    virtual void GRAY2RGBA(const ImMat& im, ImMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input Gray from CPU buffer and output to float RGBA GPU buffer
    virtual void GRAY2RGBA(const ImMat& im, VkMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input Gray from CPU buffer and output to float RGBA GPU Image3D
    virtual void GRAY2RGBA(const ImMat& im, VkImageMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input Gray from GPU buffer and output to float RGBA CPU buffer
    virtual void GRAY2RGBA(const VkMat& im, ImMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input Gray from GPU buffer and output to GPU buffer
    virtual void GRAY2RGBA(const VkMat& im, VkMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input Gray from GPU buffer and output to float RGBA GPU Image3D
    virtual void GRAY2RGBA(const VkMat& im, VkImageMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;


    virtual void Conv(const ImMat& im, ImMat & om) const;
    virtual void Conv(const ImMat& im, VkMat & om) const;
    virtual void Conv(const VkMat& im, ImMat & om) const;
    virtual void Conv(const VkMat& im, VkMat & om) const;

public:
    const VulkanDevice* vkdev;
    Pipeline * pipeline_yuv_rgb = nullptr;
    Pipeline * pipeline_rgb_yuv = nullptr;
    Pipeline * pipeline_gray_rgb = nullptr;
    Pipeline * pipeline_conv = nullptr;
    VkCompute * cmd = nullptr;
    Option opt;

private:
    void upload_param(const VkMat& Im_YUV, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    void upload_param(const VkMat& Im_RGB, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const;
    void upload_param(const VkMat& Im, VkMat& dst, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    void upload_param(const VkMat& Im, VkMat& dst) const;
};
} // namespace ImGui 