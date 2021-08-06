#include "Resize_vulkan.h"
#include "Resize_shader.h"
#include "ImVulkanShader.h"

namespace ImGui 
{
Resize_vulkan::Resize_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);
    compile_spirv_module(Resize_data, opt, spirv_data);
    pipe = new Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(8, 8, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    cmd->reset();
}

Resize_vulkan::~Resize_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Resize_vulkan::upload_param(const VkMat& src, VkMat& dst, ImInterpolateMode type) const
{
    std::vector<VkMat> bindings(2);
    bindings[0] = src;
    bindings[1] = dst;
    std::vector<vk_constant_type> constants(7);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = dst.w;
    constants[4].i = dst.h;
    constants[5].i = type;
    constants[6].i = dst.color_format;
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

// input CPU Buffer and output to RGBA8888 CPU buffer
void Resize_vulkan::Resize(const ImMat& src, ImMat& dst, float fx, float fy, ImInterpolateMode type) const
{
    VkMat dst_buffer;
    VkMat vk_src;
    cmd->record_clone(src, vk_src, opt);
    int dst_width = Im_AlignSize((fx == 0.f ? src.w : src.w * fx), 4);
    int dst_height = Im_AlignSize((fx == 0.f ? src.h : fy == 0.f ? src.h * fx : src.h * fy), 4);
    dst_buffer.create_type(dst_width, dst_height, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst_buffer.color_format = IM_CF_ABGR;   // for render

    upload_param(vk_src, dst_buffer, type);

    cmd->record_clone(dst_buffer, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

// input CPU Buffer and output to RGBA GPU buffer
void Resize_vulkan::Resize(const ImMat& src, VkMat& dst, float fx, float fy, ImInterpolateMode type) const
{
    VkMat vk_src;
    cmd->record_clone(src, vk_src, opt);
    int dst_width = Im_AlignSize((fx == 0.f ? src.w : src.w * fx), 4);
    int dst_height = Im_AlignSize((fx == 0.f ? src.h : fy == 0.f ? src.h * fx : src.h * fy), 4);
    dst.create_type(dst_width, dst_height, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    upload_param(vk_src, dst, type);

    cmd->submit_and_wait();
    cmd->reset();
}

// input GPU Buffer and output to RGBA CPU buffer
void Resize_vulkan::Resize(const VkMat& src, ImMat& dst, float fx, float fy, ImInterpolateMode type) const
{
    VkMat dst_buffer;
    int dst_width = Im_AlignSize((fx == 0.f ? src.w : src.w * fx), 4);
    int dst_height = Im_AlignSize((fx == 0.f ? src.h : fy == 0.f ? src.h * fx : src.h * fy), 4);
    dst_buffer.create_type(dst_width, dst_height, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst_buffer.color_format = IM_CF_ABGR;   // for render

    upload_param(src, dst_buffer, type);

    cmd->record_clone(dst_buffer, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

// input GPU Buffer and output to RGBA GPU buffer
void Resize_vulkan::Resize(const VkMat& src, VkMat& dst, float fx, float fy, ImInterpolateMode type) const
{
    int dst_width = Im_AlignSize((fx == 0.f ? src.w : src.w * fx), 4);
    int dst_height = Im_AlignSize((fx == 0.f ? src.h : fy == 0.f ? src.h * fx : src.h * fy), 4);
    dst.create_type(dst_width, dst_height, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    upload_param(src, dst, type);

    cmd->submit_and_wait();
    cmd->reset();
}
} //namespace ImGui
