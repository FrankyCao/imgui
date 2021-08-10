#include "CopyTo_vulkan.h"
#include "CopyTo_shader.h"
#include "ImVulkanShader.h"

namespace ImGui 
{
CopyTo_vulkan::CopyTo_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);

    std::vector<vk_specialization_type> specializations(0);

    compile_spirv_module(CopyTo_data, opt, spirv_data);
    pipe = new Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(8, 8, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    
    cmd->reset();
}

CopyTo_vulkan::~CopyTo_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void CopyTo_vulkan::upload_param(const VkMat& src, VkMat& dst, int x, int y, float alpha) const
{
    std::vector<VkMat> bindings(2);
    bindings[0] = src;
    bindings[1] = dst;
    std::vector<vk_constant_type> constants(11);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = dst.w;
    constants[5].i = dst.h;
    constants[6].i = dst.c;
    constants[7].i = dst.color_format;
    constants[8].i = x;
    constants[9].i = y;
    constants[10].f = alpha;
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void CopyTo_vulkan::copyTo(const ImMat& src, ImMat& dst, int x, int y, float alpha) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    if (src.dims != dst.dims || src.elemsize != dst.elemsize || src.elempack != dst.elempack ||
        src.c != dst.c || src.depth != dst.depth || dst.device != IM_DD_VULKAN ||
        src.type != dst.type || src.color_space != dst.color_space || src.color_format != dst.color_format || src.color_range != dst.color_range)
        return;
    
    if (x >= dst.w || y >= dst.h)
        return;

    VkMat out_gpu;
    cmd->record_clone(dst, out_gpu, opt);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, x, y, alpha);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void CopyTo_vulkan::copyTo(const ImMat& src, VkMat& dst, int x, int y, float alpha) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    if (src.dims != dst.dims || src.elemsize != dst.elemsize || src.elempack != dst.elempack ||
        src.c != dst.c || src.depth != dst.depth || dst.device != IM_DD_VULKAN ||
        src.type != dst.type || src.color_space != dst.color_space || src.color_format != dst.color_format || src.color_range != dst.color_range)
        return;
    
    if (x >= dst.w || y >= dst.h)
        return;

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, x, y, alpha);

    cmd->submit_and_wait();
    cmd->reset();
}

void CopyTo_vulkan::copyTo(const VkMat& src, ImMat& dst, int x, int y, float alpha) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    if (src.dims != dst.dims || src.elemsize != dst.elemsize || src.elempack != dst.elempack ||
        src.c != dst.c || src.depth != dst.depth || src.device != IM_DD_VULKAN ||
        src.type != dst.type || src.color_space != dst.color_space || src.color_format != dst.color_format || src.color_range != dst.color_range)
        return;
    
    if (x >= dst.w || y >= dst.h)
        return;

    VkMat out_gpu;
    cmd->record_clone(dst, out_gpu, opt);

    upload_param(src, out_gpu, x, y, alpha);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void CopyTo_vulkan::copyTo(const VkMat& src, VkMat& dst, int x, int y, float alpha) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    if (src.dims != dst.dims || src.elemsize != dst.elemsize || src.elempack != dst.elempack ||
        src.c != dst.c || src.device != dst.device || src.device_number != dst.device_number || src.depth != dst.depth || 
        src.type != dst.type || src.color_space != dst.color_space || src.color_format != dst.color_format || src.color_range != dst.color_range)
        return;
    
    if (x >= dst.w || y >= dst.h)
        return;
    
    upload_param(src, dst, x, y, alpha);

    cmd->submit_and_wait();
    cmd->reset();
}
} //namespace ImGui 
