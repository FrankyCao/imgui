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
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;

    compile_spirv_module(YUV2RGB_data, opt, spirv_data);
    pipeline_yuv_rgb = new Pipeline(vkdev);
    pipeline_yuv_rgb->set_optimal_local_size_xyz(16, 16, 1);
    pipeline_yuv_rgb->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();

    compile_spirv_module(GRAY2RGB_data, opt, spirv_data);
    pipeline_gray_rgb = new Pipeline(vkdev);
    pipeline_gray_rgb->set_optimal_local_size_xyz(16, 16, 1);
    pipeline_gray_rgb->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();

    compile_spirv_module(Conv_data, opt, spirv_data);
    pipeline_conv = new Pipeline(vkdev);
    pipeline_conv->set_optimal_local_size_xyz(16, 16, 1);
    pipeline_conv->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();

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

void ColorConvert_vulkan::upload_param(const VkMat& Im_Y, const VkMat& Im_U, const VkMat& Im_V, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    VkMat matrix_y2r_gpu;
    const ImMat conv_mat_y2r = *color_table[0][color_range][color_space];
    cmd->record_clone(conv_mat_y2r, matrix_y2r_gpu, opt);
    std::vector<VkMat> bindings(17);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (Im_Y.type == IM_DT_INT8)    bindings[4] = Im_Y;
    else if (Im_Y.type == IM_DT_INT16)   bindings[5] = Im_Y;
    else if (Im_Y.type == IM_DT_FLOAT16) bindings[6] = Im_Y;
    else if (Im_Y.type == IM_DT_FLOAT32) bindings[7] = Im_Y;

    if      (Im_U.type == IM_DT_INT8)    bindings[ 8] = Im_U;
    else if (Im_U.type == IM_DT_INT16)   bindings[ 9] = Im_U;
    else if (Im_U.type == IM_DT_FLOAT16) bindings[10] = Im_U;
    else if (Im_U.type == IM_DT_FLOAT32) bindings[11] = Im_U;

    if (color_format != IM_CF_NV12)
    {
        if      (Im_V.type == IM_DT_INT8)    bindings[12] = Im_V;
        else if (Im_V.type == IM_DT_INT16)   bindings[13] = Im_V;
        else if (Im_V.type == IM_DT_FLOAT16) bindings[14] = Im_V;
        else if (Im_V.type == IM_DT_FLOAT32) bindings[15] = Im_V;
    }

    bindings[16] = matrix_y2r_gpu;

    std::vector<vk_constant_type> constants(10);
    constants[0].i = dst.w;
    constants[1].i = dst.h;
    constants[2].i = dst.c;
    constants[3].i = color_format;
    constants[4].i = Im_Y.type;
    constants[5].i = color_space;
    constants[6].i = color_range;
    constants[7].f = (float)(1 << video_shift);
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    cmd->record_pipeline(pipeline_yuv_rgb, bindings, constants, dst);
}

// input YUV planer from CPU buffer and output to RGBA8888 CPU buffer
void ColorConvert_vulkan::YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_yuv_rgb || !cmd)
    {
        return;
    }
    VkMat matrix_y2r_gpu;
    VkMat vk_Y, vk_U, vk_V;
    VkMat vk_RGB;
    im_RGB.create_type(im_Y.w, im_Y.h, 4, im_RGB.type);
    vk_RGB.create_like(im_RGB, opt.blob_vkallocator);
    cmd->record_clone(im_Y, vk_Y, opt);
    cmd->record_clone(im_U, vk_U, opt);
    if (color_format != IM_CF_NV12)
    {
        cmd->record_clone(im_V, vk_V, opt);
    }

    upload_param(vk_Y, vk_U, vk_V, vk_RGB, color_format, color_space, color_range, video_depth, video_shift);

    cmd->record_clone(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

// input YUV planer from CPU buffer and output to float RGBA GPU buffer
void ColorConvert_vulkan::YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, VkMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_yuv_rgb || !cmd)
    {
        return;
    }
    VkMat matrix_y2r_gpu;
    VkMat vk_Y, vk_U, vk_V;
    im_RGB.create_type(im_Y.w, im_Y.h, 4, im_RGB.type, opt.blob_vkallocator);
    cmd->record_clone(im_Y, vk_Y, opt);
    cmd->record_clone(im_U, vk_U, opt);
    if (color_format != IM_CF_NV12)
    {
        cmd->record_clone(im_V, vk_V, opt);
    }

    upload_param(vk_Y, vk_U, vk_V, im_RGB, color_format, color_space, color_range, video_depth, video_shift);

    cmd->submit_and_wait();
    cmd->reset();
}

// input YUV planer from CPU buffer and output to float RGBA GPU Image3D
void ColorConvert_vulkan::YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, VkImageMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_yuv_rgb || !cmd)
    {
        return;
    }
    VkMat matrix_y2r_gpu;
    VkMat vk_Y, vk_U, vk_V;
    VkMat vk_RGB;
    vk_RGB.create_type(im_Y.w, im_Y.h, 4, im_RGB.type, opt.blob_vkallocator);
    cmd->record_clone(im_Y, vk_Y, opt);
    cmd->record_clone(im_U, vk_U, opt);
    if (color_format != IM_CF_NV12)
    {
        cmd->record_clone(im_V, vk_V, opt);
    }

    upload_param(vk_Y, vk_U, vk_V, vk_RGB, color_format, color_space, color_range, video_depth, video_shift);

    cmd->record_buffer_to_image(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ColorConvert_vulkan::upload_param(const VkMat& Im, VkMat& dst, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
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
    constants[3].i = color_format;
    constants[4].i = Im.type;
    constants[5].i = dst.w;
    constants[6].i = dst.h;
    constants[7].i = dst.c;
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    constants[10].f = (float)(1 << video_shift);
    cmd->record_pipeline(pipeline_gray_rgb, bindings, constants, dst);
}
// input Gray from CPU buffer and output to RGBA8888 CPU buffer
void ColorConvert_vulkan::GRAY2RGBA(const ImMat& im, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_gray_rgb || !cmd)
    {
        return;
    }
    VkMat vk_Data;
    VkMat vk_RGB;
    im_RGB.create_type(im.w, im.h, 4, im_RGB.type);
    vk_RGB.create_like(im_RGB, opt.blob_vkallocator);
    cmd->record_clone(im, vk_Data, opt);

    upload_param(vk_Data, vk_RGB, color_format, color_space, color_range, video_depth, video_shift);

    cmd->record_clone(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

// input Gray from CPU buffer and output to float RGBA GPU buffer
void ColorConvert_vulkan::GRAY2RGBA(const ImMat& im, VkMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_gray_rgb || !cmd)
    {
        return;
    }
    VkMat vk_Data;
    im_RGB.create_type(im.w, im.h, 4, im_RGB.type, opt.blob_vkallocator);
    cmd->record_clone(im, vk_Data, opt);

    upload_param(vk_Data, im_RGB, color_format, color_space, color_range, video_depth, video_shift);

    cmd->submit_and_wait();
    cmd->reset();
}

// input Gray from CPU buffer and output to float RGBA GPU Image3D
void ColorConvert_vulkan::GRAY2RGBA(const ImMat& im, VkImageMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    if (!vkdev || !pipeline_gray_rgb || !cmd)
    {
        return;
    }
    VkMat vk_Data;
    VkMat vk_RGB;
    vk_RGB.create_type(im.w, im.h, 4, im_RGB.type, opt.blob_vkallocator); // ？
    cmd->record_clone(im, vk_Data, opt);

    upload_param(vk_Data, vk_RGB, color_format, color_space, color_range, video_depth, video_shift);

    cmd->record_buffer_to_image(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

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
    om.create_type(im.w, im.h, 4, om.type);
    VkMat out_gpu;
    out_gpu.create_like(om, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(im, in_gpu, opt);

    upload_param(in_gpu, out_gpu);

    // download
    cmd->record_clone(out_gpu, om, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ColorConvert_vulkan::Conv(const ImMat& im, VkMat & om) const
{
    if (!vkdev || !pipeline_conv || !cmd)
    {
        return;
    }
    om.create_type(im.w, im.h, 4, om.type, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(im, in_gpu, opt);

    upload_param(in_gpu, om);

    cmd->submit_and_wait();
    cmd->reset();
}

void ColorConvert_vulkan::Conv(const VkMat& im, ImMat & om) const
{
    if (!vkdev || !pipeline_conv || !cmd)
    {
        return;
    }
    om.create_type(im.w, im.h, 4, om.type);
    VkMat out_gpu;
    out_gpu.create_like(om, opt.blob_vkallocator);

    upload_param(im, out_gpu);

    // download
    cmd->record_clone(out_gpu, om, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ColorConvert_vulkan::Conv(const VkMat& im, VkMat & om) const
{
    if (!vkdev || !pipeline_conv || !cmd)
    {
        return;
    }
    om.create_type(im.w, im.h, 4, om.type, opt.blob_vkallocator);

    upload_param(im, om);

    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui 