#include "Resize_vulkan.h"
#include "Resize_shader.h"

namespace ImVulkan 
{
Resize_vulkan::Resize_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);
    compile_spirv_module(Resize8_data, opt, spirv_rgb_8);
    pipeline_rgb_8 = new Pipeline(vkdev);
    pipeline_rgb_8->set_optimal_local_size_xyz(8, 8, 4);
    pipeline_rgb_8->create(spirv_rgb_8.data(), spirv_rgb_8.size() * 4, specializations);
    compile_spirv_module(Resize16_data, opt, spirv_rgb_16);
    pipeline_rgb_16 = new Pipeline(vkdev);
    pipeline_rgb_16->set_optimal_local_size_xyz(8, 8, 4);
    pipeline_rgb_16->create(spirv_rgb_16.data(), spirv_rgb_16.size() * 4, specializations);
    compile_spirv_module(ResizeFloat32_data, opt, spirv_rgb_f);
    pipeline_rgb_f = new Pipeline(vkdev);
    pipeline_rgb_f->set_optimal_local_size_xyz(8, 8, 4);
    pipeline_rgb_f->create(spirv_rgb_f.data(), spirv_rgb_f.size() * 4, specializations);
    cmd->reset();
}

Resize_vulkan::~Resize_vulkan()
{
    if (vkdev)
    {
        if (pipeline_rgb_8) { delete pipeline_rgb_8; pipeline_rgb_8 = nullptr; }
        if (pipeline_rgb_16) { delete pipeline_rgb_16; pipeline_rgb_16 = nullptr; }
        if (pipeline_rgb_f) { delete pipeline_rgb_f; pipeline_rgb_f = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

// input CPU Buffer and output to RGBA8888 CPU buffer
void Resize_vulkan::Resize(const ImageBuffer& src, ImageBuffer& dst, float fx, float fy, InterpolateMode type) const
{
    VkImageBuffer dst_buffer;
    VkImageBuffer vk_src;
    cmd->record_clone(src, vk_src, opt);
    int dst_width = src.w * fx;
    int dst_height = fy == 0.f ? src.h * fx : src.h * fy;
    dst_buffer.create_type(dst_width, dst_height, 4, INT8, opt.blob_vkallocator);
    std::vector<VkImageBuffer> bindings(3);
    bindings[0] = vk_src;
    bindings[1] = dst_buffer;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.cstep;
    constants[3].i = dst_width;
    constants[4].i = dst_height;
    constants[5].i = type;
    constants[6].f = src.type == FLOAT32 || src.type == FLOAT16 ? 1.0f : src.type == INT16 ? (float)(1 << 16) : (float)(1 << 8);
    constants[7].i = 1;
    if (src.type == FLOAT32)
        cmd->record_pipeline(pipeline_rgb_f, bindings, constants, dst_buffer);
    else if (src.type == INT16)
        cmd->record_pipeline(pipeline_rgb_16, bindings, constants, dst_buffer);
    else
        cmd->record_pipeline(pipeline_rgb_8, bindings, constants, dst_buffer);
    cmd->record_clone(dst_buffer, dst, opt);
    cmd->submit_and_wait();
    cmd->flash();
}

// input CPU Buffer and output to RGBA GPU buffer
void Resize_vulkan::Resize(const ImageBuffer& src, VkImageBuffer& dst, float fx, float fy, InterpolateMode type) const
{
    VkImageBuffer vk_src;
    cmd->record_clone(src, vk_src, opt);
    int dst_width = src.w * fx;
    int dst_height = fy == 0.f ? src.h * fx : src.h * fy;
    dst.create_type(dst_width, dst_height, 4, FLOAT32, opt.blob_vkallocator);
    std::vector<VkImageBuffer> bindings(3);
    bindings[0] = vk_src;
    bindings[1] = dst;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.cstep;
    constants[3].i = dst_width;
    constants[4].i = dst_height;
    constants[5].i = type;
    constants[6].f = src.type == FLOAT32 || src.type == FLOAT16 ? 1.0f : src.type == INT16 ? (float)(1 << 16) : (float)(1 << 8);
    constants[7].i = 0;
    if (src.type == FLOAT32)
        cmd->record_pipeline(pipeline_rgb_f, bindings, constants, dst);
    else if (src.type == INT16)
        cmd->record_pipeline(pipeline_rgb_16, bindings, constants, dst);
    else
        cmd->record_pipeline(pipeline_rgb_8, bindings, constants, dst);
    cmd->submit_and_wait();
    cmd->flash();
}

// input GPU Buffer and output to RGBA GPU buffer
void Resize_vulkan::Resize(const VkImageBuffer& src, VkImageBuffer& dst, float fx, float fy, InterpolateMode type) const
{
    int dst_width = src.w * fx;
    int dst_height = fy == 0.f ? src.h * fx : src.h * fy;
    dst.create_type(dst_width, dst_height, 4, FLOAT32, opt.blob_vkallocator);
    std::vector<VkImageBuffer> bindings(3);
    bindings[0] = src;
    bindings[1] = dst;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.cstep;
    constants[3].i = dst_width;
    constants[4].i = dst_height;
    constants[5].i = type;
    constants[6].f = src.type == FLOAT32 || src.type == FLOAT16 ? 1.0f : src.type == INT16 ? (float)(1 << 16) : (float)(1 << 8);
    constants[7].i = 0;
    if (src.type == FLOAT32)
        cmd->record_pipeline(pipeline_rgb_f, bindings, constants, dst);
    else if (src.type == INT16)
        cmd->record_pipeline(pipeline_rgb_16, bindings, constants, dst);
    else
        cmd->record_pipeline(pipeline_rgb_8, bindings, constants, dst);
    cmd->submit_and_wait();
    cmd->flash();
}

// input GPU Buffer and output to GPU Image3D
void Resize_vulkan::Resize(const VkImageBuffer& src, VkImageMat& dst, float fx, float fy, InterpolateMode type) const
{
    VkImageBuffer dst_buffer;
    int dst_width = src.w * fx;
    int dst_height = fy == 0.f ? src.h * fx : src.h * fy;
    dst_buffer.create_type(dst_width, dst_height, 4, FLOAT32, opt.blob_vkallocator);
    std::vector<VkImageBuffer> bindings(3);
    bindings[0] = src;
    bindings[1] = dst_buffer;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.cstep;
    constants[3].i = dst_width;
    constants[4].i = dst_height;
    constants[5].i = type;
    constants[6].f = src.type == FLOAT32 || src.type == FLOAT16 ? 1.0f : src.type == INT16 ? (float)(1 << 16) : (float)(1 << 8);
    constants[7].i = 0;
    if (src.type == FLOAT32)
        cmd->record_pipeline(pipeline_rgb_f, bindings, constants, dst_buffer);
    else if (src.type == INT16)
        cmd->record_pipeline(pipeline_rgb_16, bindings, constants, dst_buffer);
    else
        cmd->record_pipeline(pipeline_rgb_8, bindings, constants, dst_buffer);
    cmd->record_buffer_to_image(dst_buffer, dst, opt);
    cmd->submit_and_wait();
    cmd->flash();
}

// input CPU Buffer and output to GPU Image3D
void Resize_vulkan::Resize(const ImageBuffer& src, VkImageMat& dst, float fx, float fy, InterpolateMode type) const
{
    VkImageBuffer vk_src;
    cmd->record_clone(src, vk_src, opt);
    VkImageBuffer dst_buffer;
    int dst_width = src.w * fx;
    int dst_height = fy == 0.f ? src.h * fx : src.h * fy;
    dst_buffer.create_type(dst_width, dst_height, 4, FLOAT32, opt.blob_vkallocator);
    std::vector<VkImageBuffer> bindings(3);
    bindings[0] = vk_src;
    bindings[1] = dst_buffer;
    std::vector<vk_constant_type> constants(8);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.cstep;
    constants[3].i = dst_width;
    constants[4].i = dst_height;
    constants[5].i = type;
    constants[6].f = src.type == FLOAT32 || src.type == FLOAT16 ? 1.0f : src.type == INT16 ? (float)(1 << 16) : (float)(1 << 8);
    constants[7].i = 0;
    if (src.type == FLOAT32)
        cmd->record_pipeline(pipeline_rgb_f, bindings, constants, dst_buffer);
    else if (src.type == INT16)
        cmd->record_pipeline(pipeline_rgb_16, bindings, constants, dst_buffer);
    else
        cmd->record_pipeline(pipeline_rgb_8, bindings, constants, dst_buffer);
    cmd->record_buffer_to_image(dst_buffer, dst, opt);
    cmd->submit_and_wait();
    cmd->flash();
}

} //namespace ImVulkan 