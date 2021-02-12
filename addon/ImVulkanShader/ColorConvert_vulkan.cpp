#include "ColorConvert_vulkan.h"
#include "ColorConvert_shader.h"
#include "ImVulkanShader.h"

namespace ImVulkan 
{
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
    pipeline_yuv_rgb_8->set_optimal_local_size_xyz(8, 8, 4);
    pipeline_yuv_rgb_8->create(spirv_yuv_rgb_8.data(), spirv_yuv_rgb_8.size() * 4, specializations);
    compile_spirv_module(YUV2RGB16_data, opt, spirv_yuv_rgb_16);
    pipeline_yuv_rgb_16 = new Pipeline(vkdev);
    pipeline_yuv_rgb_16->set_optimal_local_size_xyz(8, 8, 4);
    pipeline_yuv_rgb_16->create(spirv_yuv_rgb_16.data(), spirv_yuv_rgb_16.size() * 4, specializations);
    cmd->reset();
}

ColorConvert_vulkan::~ColorConvert_vulkan()
{
    if (vkdev)
    {
        if (pipeline_yuv_rgb_8) { delete pipeline_yuv_rgb_8; pipeline_yuv_rgb_8 = nullptr; }
        if (pipeline_yuv_rgb_16) { delete pipeline_yuv_rgb_16; pipeline_yuv_rgb_16 = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void ColorConvert_vulkan::YUV2RGBA(const ImageBuffer& im_Y, const ImageBuffer& im_U, const ImageBuffer& im_V, ImageBuffer & im_RGB, ColorFormat color_format, ColorSpace color_space, ColorRange color_range, int video_depth, int video_shift) const
{
    VkImageBuffer matix_y2r_gpu;
    const ImageBuffer conv_mat_y2r = *color_table[0][color_range][color_space];
    cmd->record_clone(conv_mat_y2r, matix_y2r_gpu, opt);

    ImVulkan::VkImageBuffer vk_Y, vk_U, vk_V;
    ImVulkan::VkImageBuffer vk_RGB;
    im_RGB.create_type(im_Y.w, im_Y.h, 4, ImVulkan::INT8);
    vk_RGB.create_like(im_RGB, opt.blob_vkallocator);
    cmd->record_clone(im_Y, vk_Y, opt);
    cmd->record_clone(im_U, vk_U, opt);
    if (color_format != NV12)
    {
        cmd->record_clone(im_V, vk_V, opt);
    }
    std::vector<ImVulkan::VkImageBuffer> bindings(6);
    bindings[0] = vk_Y;
    bindings[1] = vk_U;
    if (color_format != NV12)
        bindings[2] = vk_V;
    bindings[4] = vk_RGB;
    bindings[5] = matix_y2r_gpu;
    std::vector<ImVulkan::vk_constant_type> constants(8);
    constants[0].i = vk_RGB.w;
    constants[1].i = vk_RGB.h;
    constants[2].i = vk_RGB.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = 1;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_yuv_rgb_16, bindings, constants, vk_RGB);
    }
    else
    {
        cmd->record_pipeline(pipeline_yuv_rgb_8, bindings, constants, vk_RGB);
    }
    cmd->record_clone(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->flash();
}

void ColorConvert_vulkan::YUV2RGBA(const ImageBuffer& im_Y, const ImageBuffer& im_U, const ImageBuffer& im_V, VkImageBuffer & im_RGB, ColorFormat color_format, ColorSpace color_space, ColorRange color_range, int video_depth, int video_shift) const
{
    VkImageBuffer matix_y2r_gpu;
    const ImageBuffer conv_mat_y2r = *color_table[0][color_range][color_space];
    cmd->record_clone(conv_mat_y2r, matix_y2r_gpu, opt);
    ImVulkan::VkImageBuffer vk_Y, vk_U, vk_V;
    im_RGB.create_type(im_Y.w, im_Y.h, 4, ImVulkan::FLOAT32, opt.blob_vkallocator);
    cmd->record_clone(im_Y, vk_Y, opt);
    cmd->record_clone(im_U, vk_U, opt);
    if (color_format != NV12)
    {
        cmd->record_clone(im_V, vk_V, opt);
    }
    std::vector<ImVulkan::VkImageBuffer> bindings(6);
    bindings[0] = vk_Y;
    bindings[1] = vk_U;
    if (color_format != NV12)
        bindings[2] = vk_V;
    bindings[3] = im_RGB;
    bindings[5] = matix_y2r_gpu;
    std::vector<ImVulkan::vk_constant_type> constants(8);
    constants[0].i = im_RGB.w;
    constants[1].i = im_RGB.h;
    constants[2].i = im_RGB.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = 0;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_yuv_rgb_16, bindings, constants, im_RGB);
    }
    else
    {
        cmd->record_pipeline(pipeline_yuv_rgb_8, bindings, constants, im_RGB);
    }
    cmd->submit_and_wait();
    cmd->flash();
}

void ColorConvert_vulkan::YUV2RGBA(const ImageBuffer& im_Y, const ImageBuffer& im_U, const ImageBuffer& im_V, VkImageMat & im_RGB, ColorFormat color_format, ColorSpace color_space, ColorRange color_range, int video_depth, int video_shift) const
{
    VkImageBuffer matix_y2r_gpu;
    const ImageBuffer conv_mat_y2r = *color_table[0][color_range][color_space];
    cmd->record_clone(conv_mat_y2r, matix_y2r_gpu, opt);
    ImVulkan::VkImageBuffer vk_Y, vk_U, vk_V;
    ImVulkan::VkImageBuffer vk_RGB;
    vk_RGB.create_type(im_Y.w, im_Y.h, 4, ImVulkan::FLOAT32, opt.blob_vkallocator);
    cmd->record_clone(im_Y, vk_Y, opt);
    cmd->record_clone(im_U, vk_U, opt);
    if (color_format != NV12)
    {
        cmd->record_clone(im_V, vk_V, opt);
    }
    std::vector<ImVulkan::VkImageBuffer> bindings(6);
    bindings[0] = vk_Y;
    bindings[1] = vk_U;
    if (color_format != NV12)
        bindings[2] = vk_V;
    bindings[3] = vk_RGB;
    bindings[5] = matix_y2r_gpu;
    std::vector<ImVulkan::vk_constant_type> constants(8);
    constants[0].i = vk_RGB.w;
    constants[1].i = vk_RGB.h;
    constants[2].i = vk_RGB.c;
    constants[3].i = color_format;
    constants[4].i = color_space;
    constants[5].i = color_range;
    constants[6].f = (float)(1 << video_shift);
    constants[7].i = 0;
    if (video_depth > 8)
    {
        cmd->record_pipeline(pipeline_yuv_rgb_16, bindings, constants, vk_RGB);
    }
    else
    {
        cmd->record_pipeline(pipeline_yuv_rgb_8, bindings, constants, vk_RGB);
    }
    cmd->record_buffer_to_image(vk_RGB, im_RGB, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

} // namespace ImVulkan 