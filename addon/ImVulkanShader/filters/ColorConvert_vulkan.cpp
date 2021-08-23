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

    compile_spirv_module(YUV2RGB8_data, opt, spirv_yuv_rgb_8);
    pipeline_yuv_rgb_8 = new Pipeline(vkdev);
    pipeline_yuv_rgb_8->set_optimal_local_size_xyz(8, 8, 1);
    pipeline_yuv_rgb_8->create(spirv_yuv_rgb_8.data(), spirv_yuv_rgb_8.size() * 4, specializations);

    compile_spirv_module(YUV2RGB16_data, opt, spirv_yuv_rgb_16);
    pipeline_yuv_rgb_16 = new Pipeline(vkdev);
    pipeline_yuv_rgb_16->set_optimal_local_size_xyz(8, 8, 1);
    pipeline_yuv_rgb_16->create(spirv_yuv_rgb_16.data(), spirv_yuv_rgb_16.size() * 4, specializations);

    compile_spirv_module(GRAY2RGB8_data, opt, spirv_gray_rgb_8);
    pipeline_gray_rgb_8 = new Pipeline(vkdev);
    pipeline_gray_rgb_8->set_optimal_local_size_xyz(4, 4, 1);
    pipeline_gray_rgb_8->create(spirv_gray_rgb_8.data(), spirv_gray_rgb_8.size() * 4, specializations);

    compile_spirv_module(GRAY2RGB16_data, opt, spirv_gray_rgb_16);
    pipeline_gray_rgb_16 = new Pipeline(vkdev);
    pipeline_gray_rgb_16->set_optimal_local_size_xyz(8, 8, 1);
    pipeline_gray_rgb_16->create(spirv_gray_rgb_16.data(), spirv_gray_rgb_16.size() * 4, specializations);

    compile_spirv_module(FloatInt8_data, opt, spirv_float_int8);
    pipeline_float_int8 = new Pipeline(vkdev);
    pipeline_float_int8->set_optimal_local_size_xyz(8, 8, 1);
    pipeline_float_int8->create(spirv_float_int8.data(), spirv_float_int8.size() * 4, specializations);

    compile_spirv_module(FloatInt16_data, opt, spirv_float_int16);
    pipeline_float_int16 = new Pipeline(vkdev);
    pipeline_float_int16->set_optimal_local_size_xyz(8, 8, 1);
    pipeline_float_int16->create(spirv_float_int16.data(), spirv_float_int16.size() * 4, specializations);

    cmd->reset();
}

ColorConvert_vulkan::~ColorConvert_vulkan()
{
    if (vkdev)
    {
        if (pipeline_yuv_rgb_8) { delete pipeline_yuv_rgb_8; pipeline_yuv_rgb_8 = nullptr; }
        if (pipeline_yuv_rgb_16) { delete pipeline_yuv_rgb_16; pipeline_yuv_rgb_16 = nullptr; }
        if (pipeline_gray_rgb_8) { delete pipeline_gray_rgb_8; pipeline_gray_rgb_8 = nullptr; }
        if (pipeline_gray_rgb_16) { delete pipeline_gray_rgb_16; pipeline_gray_rgb_16 = nullptr; }
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
    std::vector<VkMat> bindings(5);
    bindings[0] = Im_Y;
    bindings[1] = Im_U;
    if (color_format != IM_CF_NV12)
        bindings[2] = Im_V;
    bindings[3] = dst;
    bindings[4] = matrix_y2r_gpu;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = dst.w;
    constants[1].i = dst.h;
    constants[2].i = dst.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = dst.color_format;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_yuv_rgb_16, bindings, constants, dst);
    }
    else
    {
        cmd->record_pipeline(pipeline_yuv_rgb_8, bindings, constants, dst);
    }
}

// input YUV planer from CPU buffer and output to RGBA8888 CPU buffer
void ColorConvert_vulkan::YUV2RGBA(const ImMat& im_Y, const ImMat& im_U, const ImMat& im_V, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    VkMat matrix_y2r_gpu;
    VkMat vk_Y, vk_U, vk_V;
    VkMat vk_RGB;
    im_RGB.create_type(im_Y.w, im_Y.h, 4, IM_DT_FLOAT32);
    im_RGB.color_format = IM_CF_ABGR;   // for render
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
    VkMat matrix_y2r_gpu;
    VkMat vk_Y, vk_U, vk_V;
    im_RGB.create_type(im_Y.w, im_Y.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator);
    im_RGB.color_format = IM_CF_ABGR;   // for render
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
    VkMat matrix_y2r_gpu;
    VkMat vk_Y, vk_U, vk_V;
    VkMat vk_RGB;
    vk_RGB.create_type(im_Y.w, im_Y.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator); // ？
    vk_RGB.color_format = IM_CF_ABGR;   // for render
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
    std::vector<VkMat> bindings(2);
    bindings[0] = Im;
    bindings[1] = dst;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = Im.w;
    constants[1].i = Im.h;
    constants[2].i = dst.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = dst.color_format;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_gray_rgb_16, bindings, constants, dst);
    }
    else
    {
        cmd->record_pipeline(pipeline_gray_rgb_8, bindings, constants, dst);
    }
}
// input Gray from CPU buffer and output to RGBA8888 CPU buffer
void ColorConvert_vulkan::GRAY2RGBA(const ImMat& im, ImMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    VkMat vk_Data;
    VkMat vk_RGB;
    im_RGB.create_type(im.w, im.h, 4, IM_DT_FLOAT32);
    im_RGB.color_format = IM_CF_ABGR;   // for render
    vk_RGB.create_like(im_RGB, opt.blob_vkallocator);
    cmd->record_clone(im, vk_Data, opt);

    upload_param(vk_Data, vk_RGB, color_format, color_space, color_range, video_depth, video_shift);
    /*
    std::vector<VkMat> bindings(3);
    bindings[0] = vk_Data;
    bindings[2] = vk_RGB;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = vk_RGB.w;
    constants[1].i = vk_RGB.h;
    constants[2].i = vk_RGB.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = im_RGB.color_format;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_gray_rgb_16, bindings, constants, vk_RGB);
    }
    else
    {
        cmd->record_pipeline(pipeline_gray_rgb_8, bindings, constants, vk_RGB);
    }
    */
    cmd->record_clone(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

// input Gray from CPU buffer and output to float RGBA GPU buffer
void ColorConvert_vulkan::GRAY2RGBA(const ImMat& im, VkMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    VkMat vk_Data;
    im_RGB.create_type(im.w, im.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator);
    im_RGB.color_format = IM_CF_ABGR;   // for render
    cmd->record_clone(im, vk_Data, opt);
    upload_param(vk_Data, im_RGB, color_format, color_space, color_range, video_depth, video_shift);
    /*
    std::vector<VkMat> bindings(3);
    bindings[0] = vk_Data;
    bindings[2] = im_RGB;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = im_RGB.w;
    constants[1].i = im_RGB.h;
    constants[2].i = im_RGB.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = im_RGB.color_format;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_gray_rgb_16, bindings, constants, im_RGB);
    }
    else
    {
        cmd->record_pipeline(pipeline_gray_rgb_8, bindings, constants, im_RGB);
    }
    */
    cmd->submit_and_wait();
    cmd->reset();
}

// input Gray from CPU buffer and output to float RGBA GPU Image3D
void ColorConvert_vulkan::GRAY2RGBA(const ImMat& im, VkImageMat & im_RGB, ImColorFormat color_format, ImColorSpace color_space, ImColorRange color_range, int video_depth, int video_shift) const
{
    VkMat vk_Data;
    VkMat vk_RGB;
    vk_RGB.create_type(im.w, im.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator); // ？
    vk_RGB.color_format = IM_CF_ABGR;   // for render
    cmd->record_clone(im, vk_Data, opt);
    upload_param(vk_Data, vk_RGB, color_format, color_space, color_range, video_depth, video_shift);
    /*
    std::vector<VkMat> bindings(3);
    bindings[0] = vk_Data;
    bindings[1] = vk_RGB;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = vk_RGB.w;
    constants[1].i = vk_RGB.h;
    constants[2].i = vk_RGB.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = vk_RGB.color_format;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_gray_rgb_16, bindings, constants, vk_RGB);
    }
    else
    {
        cmd->record_pipeline(pipeline_gray_rgb_8, bindings, constants, vk_RGB);
    }
    */
    cmd->record_buffer_to_image(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ColorConvert_vulkan::upload_param(const VkMat& Im, VkMat& dst, ImColorFormat color_format, int video_depth) const
{
    std::vector<VkMat> bindings(2);
    bindings[0] = Im;
    bindings[1] = dst;
    std::vector<vk_constant_type> constants(4);
    constants[0].i = Im.w;
    constants[1].i = Im.h;
    constants[2].i = Im.c;
    constants[3].i = color_format;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_float_int16, bindings, constants, dst);
    }
    else
    {
        cmd->record_pipeline(pipeline_float_int8, bindings, constants, dst);
    }
}

void ColorConvert_vulkan::FloatToInt8(const ImMat& im, ImMat & om) const
{

}

void ColorConvert_vulkan::FloatToInt8(const ImMat& im, VkMat & om) const
{

}

void ColorConvert_vulkan::FloatToInt8(const VkMat& im, ImMat & om) const
{
    om.create_type(im.w, im.h, im.c, IM_DT_INT8);
    om.color_format = IM_CF_ABGR;
    VkMat om_gpu;
    om_gpu.create_like(om, opt.blob_vkallocator);
    upload_param(im, om_gpu, om.color_format, 8);
    cmd->record_clone(om_gpu, om, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ColorConvert_vulkan::FloatToInt8(const VkMat& im, VkMat & om) const
{
    om.create_type(im.w, im.h, im.c, IM_DT_INT8, opt.blob_vkallocator);
    om.color_format = IM_CF_ABGR;
    om.elempack = 4;
    upload_param(im, om, om.color_format, 8);
    cmd->submit_and_wait();
    cmd->reset();
}

// TODO::Dicky
void ColorConvert_vulkan::FloatToInt16(const ImMat& im, ImMat & om) const
{

}

void ColorConvert_vulkan::FloatToInt16(const ImMat& im, VkMat & om) const
{

}

void ColorConvert_vulkan::FloatToInt16(const VkMat& im, ImMat & om) const
{

}

void ColorConvert_vulkan::FloatToInt16(const VkMat& im, VkMat & om) const
{

}


} // namespace ImGui 