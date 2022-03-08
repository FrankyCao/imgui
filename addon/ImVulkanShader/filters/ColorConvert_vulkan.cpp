#include <sstream>
#include "ColorConvert_vulkan.h"
#include "ColorConvert_shader.h"
#include "ImVulkanShader.h"

namespace ImGui 
{
extern const ImMat * color_table[2][2][4];
ColorConvert_vulkan::ColorConvert_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    opt.use_fp16_storage = false;   // fp16 has accuracy issue for int16 convert
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;

    if (compile_spirv_module(YUV2RGB_data, opt, spirv_data) == 0)
    {
        pipeline_yuv_rgb = new Pipeline(vkdev);
        pipeline_yuv_rgb->set_optimal_local_size_xyz(16, 16, 1);
        pipeline_yuv_rgb->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    if (compile_spirv_module(RGB2YUV_data, opt, spirv_data) == 0)
    {
        pipeline_rgb_yuv = new Pipeline(vkdev);
        pipeline_rgb_yuv->set_optimal_local_size_xyz(16, 16, 1);
        pipeline_rgb_yuv->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    if (compile_spirv_module(GRAY2RGB_data, opt, spirv_data) == 0)
    {
        pipeline_gray_rgb = new Pipeline(vkdev);
        pipeline_gray_rgb->set_optimal_local_size_xyz(16, 16, 1);
        pipeline_gray_rgb->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    if (compile_spirv_module(Conv_data, opt, spirv_data) == 0)
    {
        pipeline_conv = new Pipeline(vkdev);
        pipeline_conv->set_optimal_local_size_xyz(16, 16, 1);
        pipeline_conv->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    cmd->reset();
}

ColorConvert_vulkan::~ColorConvert_vulkan()
{
    if (vkdev)
    {
        if (pipeline_yuv_rgb) { delete pipeline_yuv_rgb; pipeline_yuv_rgb = nullptr; }
        if (pipeline_gray_rgb) { delete pipeline_gray_rgb; pipeline_gray_rgb = nullptr; }
        if (pipeline_conv) { delete pipeline_conv; pipeline_conv = nullptr; }

        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

bool ColorConvert_vulkan::ConvertColorFormat(const ImMat& srcMat, ImMat& dstMat)
{
    if (dstMat.color_format < IM_CF_BGR)
    {
        std::ostringstream oss;
        oss << "Argument 'dstMat' has UNSUPPORTED color format " << srcMat.color_format << "!";
        mErrMsg = oss.str();
        return false;
    }

    int srcClrCatg = GetColorFormatCategory(srcMat.color_format);
    int dstClrCatg = GetColorFormatCategory(dstMat.color_format);
    // source is RGB
    if (srcClrCatg == 1)
    {
        // TODO: add other rgb format support
        // only support input format ABGR/ARGB.
        if (srcMat.color_format != IM_CF_ABGR && srcMat.color_format != IM_CF_ARGB)
        {
            mErrMsg = "Only support rgb input format ABGR/ARGB!";
            return false;
        }
    }
    // destination is RGB
    if (dstClrCatg == 1)
    {
        // only support output format ABGR/ARGB.
        if (dstMat.color_format != IM_CF_ABGR && dstMat.color_format != IM_CF_ARGB)
        {
            mErrMsg = "Only support rgb output format ABGR/ARGB!";
            return false;
        }
        dstMat.color_range = IM_CR_FULL_RANGE;
    }

    // prepare source vulkan mat
    VkMat srcVkMat;
    if (srcMat.device == IM_DD_VULKAN)
        srcVkMat = srcMat;
    else
        cmd->record_clone(srcMat, srcVkMat, opt);

    // prepare destination vulkan mat
    VkMat dstVkMat;
    if (dstMat.device == IM_DD_VULKAN)
    {
        dstVkMat.create_type(srcMat.w, srcMat.h, GetChannelCountByColorFormat(dstMat.color_format), dstMat.type, opt.blob_vkallocator);
        dstVkMat.color_format = dstMat.color_format;
        dstVkMat.color_range = dstMat.color_range;
        dstVkMat.color_space = srcMat.color_space;
        dstMat = dstVkMat;
    }
    else
    {
        ImMat tmp;
        tmp.create_type(srcMat.w, srcMat.h, GetChannelCountByColorFormat(dstMat.color_format), dstMat.type);
        tmp.color_format = dstMat.color_format;
        tmp.color_range = dstMat.color_range;
        tmp.color_space = srcMat.color_space;
        dstMat = tmp;
        dstVkMat.create_like(dstMat, opt.blob_vkallocator);
    }

    if (!UploadParam(srcVkMat, dstVkMat))
        return false;

    if (dstMat.device == IM_DD_CPU)
        cmd->record_clone(dstVkMat, dstMat, opt);
    else if (dstMat.device == IM_DD_VULKAN_IMAGE)
    {
        VkImageMat* pVkiMat = dynamic_cast<VkImageMat*>(&dstMat);
        cmd->record_buffer_to_image(dstVkMat, *pVkiMat, opt);
    }

    cmd->submit_and_wait();
    cmd->reset();

    return true;
}

bool ColorConvert_vulkan::UploadParam(const VkMat& src, VkMat& dst)
{
    int srcClrCatg = GetColorFormatCategory(src.color_format);
    int dstClrCatg = GetColorFormatCategory(dst.color_format);
    if (srcClrCatg < 0 || dstClrCatg < 0)
    {
        std::ostringstream oss;
        oss << "Unknown color format category! 'src.color_format' is " << src.color_format
            << ", 'dst.color_format' is " << dst.color_format << ".";
        mErrMsg = oss.str();
        return false;
    }

    // GRAY -> RGB
    if (srcClrCatg == 0 && dstClrCatg == 1)
    {
        int bitDepth = src.depth != 0 ? src.depth : src.type == IM_DT_INT8 ? 8 : src.type == IM_DT_INT16 ? 16 : 8;

        std::vector<VkMat> bindings(8);
        if      (dst.type == IM_DT_INT8)    bindings[0] = dst;
        else if (dst.type == IM_DT_INT16)   bindings[1] = dst;
        else if (dst.type == IM_DT_FLOAT16) bindings[2] = dst;
        else if (dst.type == IM_DT_FLOAT32) bindings[3] = dst;
        if      (src.type == IM_DT_INT8)    bindings[4] = src;
        else if (src.type == IM_DT_INT16)   bindings[5] = src;
        else if (src.type == IM_DT_FLOAT16) bindings[6] = src;
        else if (src.type == IM_DT_FLOAT32) bindings[7] = src;

        std::vector<vk_constant_type> constants(11);
        constants[ 0].i = src.w;
        constants[ 1].i = src.h;
        constants[ 2].i = src.c;
        constants[ 3].i = src.color_format;
        constants[ 4].i = src.type;
        constants[ 5].i = dst.w;
        constants[ 6].i = dst.h;
        constants[ 7].i = dst.c;
        constants[ 8].i = dst.color_format;
        constants[ 9].i = dst.type;
        constants[10].f = (float)((1 << bitDepth) - 1);

        cmd->record_pipeline(pipeline_gray_rgb, bindings, constants, dst);
    }
    // YUV -> RGB
    else if (srcClrCatg == 2 && dstClrCatg == 1)
    {
        VkMat vkCscCoefs;
        const ImMat cscCoefs = *color_table[0][src.color_range][src.color_space];
        cmd->record_clone(cscCoefs, vkCscCoefs, opt);
        int bitDepth = src.depth != 0 ? src.depth : src.type == IM_DT_INT8 ? 8 : src.type == IM_DT_INT16 ? 16 : 8;

        std::vector<VkMat> bindings(9);
        if      (dst.type == IM_DT_INT8)    bindings[0] = dst;
        else if (dst.type == IM_DT_INT16)   bindings[1] = dst;
        else if (dst.type == IM_DT_FLOAT16) bindings[2] = dst;
        else if (dst.type == IM_DT_FLOAT32) bindings[3] = dst;
        if      (src.type == IM_DT_INT8)    bindings[4] = src;
        else if (src.type == IM_DT_INT16)   bindings[5] = src;
        else if (src.type == IM_DT_FLOAT16) bindings[6] = src;
        else if (src.type == IM_DT_FLOAT32) bindings[7] = src;
        bindings[8] = vkCscCoefs;

        std::vector<vk_constant_type> constants(10);
        constants[0].i = src.w;
        constants[1].i = src.h;
        constants[2].i = dst.c;
        constants[3].i = src.color_format;
        constants[4].i = src.type;
        constants[5].i = src.color_space;
        constants[6].i = src.color_range;
        constants[7].f = (float)((1 << bitDepth) - 1);
        constants[8].i = dst.color_format;
        constants[9].i = dst.type;

        cmd->record_pipeline(pipeline_yuv_rgb, bindings, constants, dst);
    }
    // RGB -> YUV
    else if (srcClrCatg == 1 && dstClrCatg == 2)
    {
        VkMat vkCscCoefs;
        const ImMat cscCoefs = *color_table[1][dst.color_range][dst.color_space];
        cmd->record_clone(cscCoefs, vkCscCoefs, opt);
        int bitDepth = dst.depth != 0 ? dst.depth : dst.type == IM_DT_INT8 ? 8 : dst.type == IM_DT_INT16 ? 16 : 8;

        std::vector<VkMat> bindings(9);
        if      (dst.type == IM_DT_INT8)    bindings[0] = dst;
        else if (dst.type == IM_DT_INT16)   bindings[1] = dst;
        else if (dst.type == IM_DT_FLOAT16) bindings[2] = dst;
        else if (dst.type == IM_DT_FLOAT32) bindings[3] = dst;
        if      (src.type == IM_DT_INT8)    bindings[4] = src;
        else if (src.type == IM_DT_INT16)   bindings[5] = src;
        else if (src.type == IM_DT_FLOAT16) bindings[6] = src;
        else if (src.type == IM_DT_FLOAT32) bindings[7] = src;
        bindings[8] = vkCscCoefs;

        std::vector<vk_constant_type> constants(13);
        constants[ 0].i = src.w;
        constants[ 1].i = src.h;
        constants[ 2].i = src.c;
        constants[ 3].i = src.color_format;
        constants[ 4].i = src.type;
        constants[ 5].i = src.color_space;
        constants[ 6].i = src.color_range;
        constants[ 7].i = dst.cstep;
        constants[ 8].i = dst.color_format;
        constants[ 9].i = dst.type;
        constants[10].i = dst.color_space;
        constants[11].i = dst.color_range;
        constants[12].f = (float)((1 << bitDepth) - 1);

        cmd->record_pipeline(pipeline_rgb_yuv, bindings, constants, dst);
    }
    // conversion in same color format category
    else if (srcClrCatg == dstClrCatg)
    {
        std::vector<VkMat> bindings(8);
        if      (dst.type == IM_DT_INT8)    bindings[0] = dst;
        else if (dst.type == IM_DT_INT16)   bindings[1] = dst;
        else if (dst.type == IM_DT_FLOAT16) bindings[2] = dst;
        else if (dst.type == IM_DT_FLOAT32) bindings[3] = dst;
        if      (src.type == IM_DT_INT8)    bindings[4] = src;
        else if (src.type == IM_DT_INT16)   bindings[5] = src;
        else if (src.type == IM_DT_FLOAT16) bindings[6] = src;
        else if (src.type == IM_DT_FLOAT32) bindings[7] = src;

        std::vector<vk_constant_type> constants(10);
        constants[0].i = src.w;
        constants[1].i = src.h;
        constants[2].i = src.c;
        constants[3].i = src.color_format;
        constants[4].i = src.type;
        constants[5].i = dst.w;
        constants[6].i = dst.h;
        constants[7].i = dst.c;
        constants[8].i = dst.color_format;
        constants[9].i = dst.type;

        cmd->record_pipeline(pipeline_conv, bindings, constants, dst);
    }
    else
    {
        std::ostringstream oss;
        oss << "UNSUPPORTED color format conversion! From " << src.color_format << " to " << dst.color_format << ".";
        mErrMsg = oss.str();
        return false;
    }

    return true;
}

// YUV to RGBA functions
void ColorConvert_vulkan::upload_param(const VkMat& Im_YUV, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    VkMat matrix_y2r_gpu;
    const ImMat conv_mat_y2r = *color_table[0][color_range][color_space];
    cmd->record_clone(conv_mat_y2r, matrix_y2r_gpu, opt);
    std::vector<VkMat> bindings(9);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (Im_YUV.type == IM_DT_INT8)    bindings[4] = Im_YUV;
    else if (Im_YUV.type == IM_DT_INT16)   bindings[5] = Im_YUV;
    else if (Im_YUV.type == IM_DT_FLOAT16) bindings[6] = Im_YUV;
    else if (Im_YUV.type == IM_DT_FLOAT32) bindings[7] = Im_YUV;

    bindings[8] = matrix_y2r_gpu;

    std::vector<vk_constant_type> constants(10);
    constants[0].i = Im_YUV.w;
    constants[1].i = Im_YUV.h;
    constants[2].i = dst.c;
    constants[3].i = color_format;
    constants[4].i = Im_YUV.type;
    constants[5].i = color_space;
    constants[6].i = color_range;
    constants[7].f = (float)((1 << video_shift) - 1);
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    cmd->record_pipeline(pipeline_yuv_rgb, bindings, constants, dst);
}

void ColorConvert_vulkan::YUV2RGBA(const ImMat& im_YUV, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_yuv_rgb || !cmd)
    {
        return;
    }

    VkMat dst_gpu;
    dst_gpu.create_type(im_YUV.w, im_YUV.h, 4, im_RGB.type, opt.blob_vkallocator);

    VkMat src_gpu;
    if (im_YUV.device == IM_DD_VULKAN)
    {
        src_gpu = im_YUV;
    }
    else if (im_YUV.device == IM_DD_CPU)
    {
        cmd->record_clone(im_YUV, src_gpu, opt);
    }

    upload_param(src_gpu, dst_gpu, color_format, color_space, color_range, video_depth, video_shift);

    // download
    if (im_RGB.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, im_RGB, opt);
    else if (im_RGB.device == IM_DD_VULKAN)
        im_RGB = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}

// RGBA to YUV functions
void ColorConvert_vulkan::upload_param(const VkMat& Im_RGB, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const
{
    VkMat matrix_r2y_gpu;
    const ImMat conv_mat_r2y = *color_table[1][color_range][color_space];
    cmd->record_clone(conv_mat_r2y, matrix_r2y_gpu, opt);
    std::vector<VkMat> bindings(9);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (Im_RGB.type == IM_DT_INT8)    bindings[4] = Im_RGB;
    else if (Im_RGB.type == IM_DT_INT16)   bindings[5] = Im_RGB;
    else if (Im_RGB.type == IM_DT_FLOAT16) bindings[6] = Im_RGB;
    else if (Im_RGB.type == IM_DT_FLOAT32) bindings[7] = Im_RGB;

    bindings[8] = matrix_r2y_gpu;
    std::vector<vk_constant_type> constants(13);
    constants[0].i = Im_RGB.w;
    constants[1].i = Im_RGB.h;
    constants[2].i = Im_RGB.c;
    constants[3].i = Im_RGB.color_format;
    constants[4].i = Im_RGB.type;
    constants[5].i = Im_RGB.color_space;
    constants[6].i = Im_RGB.color_range;
    constants[7].i = dst.cstep;
    constants[8].i = color_format;
    constants[9].i = dst.type;
    constants[10].i = color_space;
    constants[11].i = color_range;
    constants[12].f = (float)((1 << video_shift) - 1);
    cmd->record_pipeline(pipeline_rgb_yuv, bindings, constants, dst);
}

void ColorConvert_vulkan::RGBA2YUV(const ImMat& im_RGB, ImMat & im_YUV, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_shift) const
{
    if (!vkdev || !pipeline_rgb_yuv || !cmd)
    {
        return;
    }
    VkMat dst_gpu;
    dst_gpu.create_type(im_RGB.w, im_RGB.h, 4, im_YUV.type, opt.blob_vkallocator);

    VkMat src_gpu;
    if (im_RGB.device == IM_DD_VULKAN)
    {
        src_gpu = im_RGB;
    }
    else if (im_RGB.device == IM_DD_CPU)
    {
        cmd->record_clone(im_RGB, src_gpu, opt);
    }

    upload_param(src_gpu, dst_gpu, color_format, color_space, color_range, video_shift);

    // download
    if (im_YUV.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, im_YUV, opt);
    else if (im_YUV.device == IM_DD_VULKAN)
        im_YUV = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}

// Gray to RGBA functions
void ColorConvert_vulkan::upload_param(const VkMat& Im, VkMat& dst, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    std::vector<VkMat> bindings(8);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (Im.type == IM_DT_INT8)      bindings[4] = Im;
    else if (Im.type == IM_DT_INT16)     bindings[5] = Im;
    else if (Im.type == IM_DT_FLOAT16)   bindings[6] = Im;
    else if (Im.type == IM_DT_FLOAT32)   bindings[7] = Im;

    std::vector<vk_constant_type> constants(11);
    constants[0].i = Im.w;
    constants[1].i = Im.h;
    constants[2].i = Im.c;
    constants[3].i = IM_CF_GRAY;
    constants[4].i = Im.type;
    constants[5].i = dst.w;
    constants[6].i = dst.h;
    constants[7].i = dst.c;
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    constants[10].f = (float)((1 << video_shift) - 1);
    cmd->record_pipeline(pipeline_gray_rgb, bindings, constants, dst);
}

void ColorConvert_vulkan::GRAY2RGBA(const ImMat& im, ImMat & im_RGB, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_gray_rgb || !cmd)
    {
        return;
    }
    VkMat dst_gpu;
    dst_gpu.create_type(im.w, im.h, 4, im_RGB.type, opt.blob_vkallocator);

    VkMat src_gpu;
    if (im.device == IM_DD_VULKAN)
    {
        src_gpu = im;
    }
    else if (im.device == IM_DD_CPU)
    {
        cmd->record_clone(im, src_gpu, opt);
    }

    upload_param(src_gpu, dst_gpu, color_space, color_range, video_depth, video_shift);

    // download
    if (im_RGB.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, im_RGB, opt);
    else if (im_RGB.device == IM_DD_VULKAN)
        im_RGB = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}

// Conv Functions
void ColorConvert_vulkan::upload_param(const VkMat& Im, VkMat& dst) const
{
    std::vector<VkMat> bindings(8);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (Im.type == IM_DT_INT8)      bindings[4] = Im;
    else if (Im.type == IM_DT_INT16)     bindings[5] = Im;
    else if (Im.type == IM_DT_FLOAT16)   bindings[6] = Im;
    else if (Im.type == IM_DT_FLOAT32)   bindings[7] = Im;

    std::vector<vk_constant_type> constants(10);
    constants[0].i = Im.w;
    constants[1].i = Im.h;
    constants[2].i = Im.c;
    constants[3].i = Im.color_format;
    constants[4].i = Im.type;
    constants[5].i = dst.w;
    constants[6].i = dst.h;
    constants[7].i = dst.c;
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    cmd->record_pipeline(pipeline_conv, bindings, constants, dst);
}

void ColorConvert_vulkan::Conv(const ImMat& im, ImMat & om) const
{
    if (!vkdev || !pipeline_conv || !cmd)
    {
        return;
    }

    VkMat dst_gpu;
    dst_gpu.create_type(im.w, im.h, 4, om.type, opt.blob_vkallocator);

    VkMat src_gpu;
    if (im.device == IM_DD_VULKAN)
    {
        src_gpu = im;
    }
    else if (im.device == IM_DD_CPU)
    {
        cmd->record_clone(im, src_gpu, opt);
    }

    upload_param(src_gpu, dst_gpu);

    // download
    if (om.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, om, opt);
    else if (om.device == IM_DD_VULKAN)
        om = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui 