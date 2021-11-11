#include "ColorBalance_vulkan.h"
#include "ColorBalance_shader.h"
#include "ImVulkanShader.h"

namespace ImGui 
{
ColorBalance_vulkan::ColorBalance_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    opt.use_fp16_storage = true;
    cmd = new VkCompute(vkdev);

    std::vector<vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;

    compile_spirv_module(Filter_data, opt, spirv_data);
    pipe = new Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(16, 16, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    
    cmd->reset();
}

ColorBalance_vulkan::~ColorBalance_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void ColorBalance_vulkan::upload_param(const VkMat& src, VkMat& dst, std::vector<float>& shadows, std::vector<float>& midtones, std::vector<float>& highlights, bool preserve_lightness) const
{
    std::vector<VkMat> bindings(8);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (src.type == IM_DT_INT8)      bindings[4] = src;
    else if (src.type == IM_DT_INT16)     bindings[5] = src;
    else if (src.type == IM_DT_FLOAT16)   bindings[6] = src;
    else if (src.type == IM_DT_FLOAT32)   bindings[7] = src;
    std::vector<vk_constant_type> constants(20);
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
    constants[10].f = shadows[0];
    constants[11].f = shadows[1];
    constants[12].f = shadows[2];
    constants[13].f = midtones[0];
    constants[14].f = midtones[1];
    constants[15].f = midtones[2];
    constants[16].f = highlights[0];
    constants[17].f = highlights[1];
    constants[18].f = highlights[2];
    constants[19].i = preserve_lightness ? 1 : 0;
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void ColorBalance_vulkan::filter(const ImMat& src, ImMat& dst, std::vector<float>& shadows, std::vector<float>& midtones, std::vector<float>& highlights, bool preserve_lightness) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, shadows, midtones, highlights, preserve_lightness);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ColorBalance_vulkan::filter(const ImMat& src, VkMat& dst, std::vector<float>& shadows, std::vector<float>& midtones, std::vector<float>& highlights, bool preserve_lightness) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, shadows, midtones, highlights, preserve_lightness);

    cmd->submit_and_wait();
    cmd->reset();
}

void ColorBalance_vulkan::filter(const VkMat& src, ImMat& dst, std::vector<float>& shadows, std::vector<float>& midtones, std::vector<float>& highlights, bool preserve_lightness) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, shadows, midtones, highlights, preserve_lightness);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ColorBalance_vulkan::filter(const VkMat& src, VkMat& dst, std::vector<float>& shadows, std::vector<float>& midtones, std::vector<float>& highlights, bool preserve_lightness) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);
    
    upload_param(src, dst, shadows, midtones, highlights, preserve_lightness);

    cmd->submit_and_wait();
    cmd->reset();
}
} //namespace ImGui 
