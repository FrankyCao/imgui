#pragma once
#include <string>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"

namespace ImGui 
{
class VKSHADER_API ColorConvert_vulkan
{
public:
    ColorConvert_vulkan(int gpu = -1);
    ~ColorConvert_vulkan();

    bool ConvertColorFormat(const ImMat& srcMat, ImMat& dstMat, ImInterpolateMode type = IM_INTERPOLATE_BICUBIC);
    std::string GetError() const { return mErrMsg; }

    virtual void YUV2RGBA(const ImMat& im_YUV, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    virtual void RGBA2YUV(const ImMat& im_RGB, ImMat & im_YUV, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const;
    virtual void GRAY2RGBA(const ImMat& im, ImMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const;
    virtual void Conv(const ImMat& im, ImMat & om) const;

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

    bool UploadParam(const VkMat& src, VkMat& dst, ImInterpolateMode type);

    std::string mErrMsg;
};
} // namespace ImGui 