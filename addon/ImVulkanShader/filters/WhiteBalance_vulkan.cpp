#include "WhiteBalance_vulkan.h"
#include "WhiteBalance_shader.h"
#include "ImVulkanShader.h"

namespace ImGui 
{
WhiteBalance_vulkan::WhiteBalance_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);

    std::vector<vk_specialization_type> specializations(0);

    compile_spirv_module(Filter_data, opt, spirv_data);
    pipe = new Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(8, 8, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    
    cmd->reset();
}

WhiteBalance_vulkan::~WhiteBalance_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void WhiteBalance_vulkan::upload_param(const VkMat& src, VkMat& dst, float temperature) const
{
    float _temperature = 0.0004 * (temperature - 5000);
    if (temperature > 5000) _temperature = 0.0006 * (temperature - 5000);
    std::vector<VkMat> bindings(2);
    bindings[0] = src;
    bindings[1] = dst;
    std::vector<vk_constant_type> constants(5);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].f = _temperature;
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void WhiteBalance_vulkan::filter(const ImMat& src, ImMat& dst, float temperature) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, temperature);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void WhiteBalance_vulkan::filter(const ImMat& src, VkMat& dst, float temperature) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, temperature);

    cmd->submit_and_wait();
    cmd->reset();
}

void WhiteBalance_vulkan::filter(const VkMat& src, ImMat& dst, float temperature) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, temperature);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void WhiteBalance_vulkan::filter(const VkMat& src, VkMat& dst, float temperature) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;
    
    upload_param(src, dst, temperature);

    cmd->submit_and_wait();
    cmd->reset();
}
} //namespace ImGui 
