#pragma once
#include "gpu.h"
#include "pipeline.h"

namespace ImGui 
{
class IMGUI_API ColorConvert_vulkan
{
public:
    ColorConvert_vulkan(int gpu = -1);
    ~ColorConvert_vulkan();

    // input YUV planer from CPU buffer and output to RGBA8888 CPU buffer
    virtual void YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input YUV planer from CPU buffer and output to float RGBA GPU buffer
    virtual void YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, VkMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input YUV planer from CPU buffer and output to float RGBA GPU Image3D
    virtual void YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, VkImageMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;

    // input Gray from CPU buffer and output to RGBA8888 CPU buffer
    virtual void GRAY2RGBA(const ImMat& im, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input Gray from CPU buffer and output to float RGBA GPU buffer
    virtual void GRAY2RGBA(const ImMat& im, VkMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    // input Gray from CPU buffer and output to float RGBA GPU Image3D
    virtual void GRAY2RGBA(const ImMat& im, VkImageMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;

    virtual void FloatToInt8(const ImMat& im, ImMat & om) const;
    virtual void FloatToInt8(const ImMat& im, VkMat & om) const;
    virtual void FloatToInt8(const VkMat& im, ImMat & om) const;
    virtual void FloatToInt8(const VkMat& im, VkMat & om) const;

    virtual void FloatToInt16(const ImMat& im, ImMat & om) const;
    virtual void FloatToInt16(const ImMat& im, VkMat & om) const;
    virtual void FloatToInt16(const VkMat& im, ImMat & om) const;
    virtual void FloatToInt16(const VkMat& im, VkMat & om) const;

public:
    const VulkanDevice* vkdev;
    Pipeline * pipeline_yuv_rgb_8 = nullptr;
    Pipeline * pipeline_yuv_rgb_16 = nullptr;
    Pipeline * pipeline_gray_rgb_8 = nullptr;
    Pipeline * pipeline_gray_rgb_16 = nullptr;
    Pipeline * pipeline_float_int8 = nullptr;
    Pipeline * pipeline_float_int16 = nullptr;
    VkCompute * cmd = nullptr;
    Option opt;

public:
    std::vector<uint32_t> spirv_yuv_rgb_8;
    std::vector<uint32_t> spirv_yuv_rgb_16;
    std::vector<uint32_t> spirv_gray_rgb_8;
    std::vector<uint32_t> spirv_gray_rgb_16;
    std::vector<uint32_t> spirv_float_int8;
    std::vector<uint32_t> spirv_float_int16;

private:
    void upload_param(const VkMat& Im_Y, const VkMat& Im_U, const VkMat& Im_V, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    void upload_param(const VkMat& Im, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    void upload_param(const VkMat& Im, VkMat& dst, ImColorFormat color_format, int video_depth) const;
};
} // namespace ImGui 