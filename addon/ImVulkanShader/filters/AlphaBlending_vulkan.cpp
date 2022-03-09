#include "AlphaBlending_vulkan.h"
#include "AlphaBlending_shader.h"
#include "ImVulkanShader.h"

namespace ImGui 
{
AlphaBlending_vulkan::AlphaBlending_vulkan(int gpu)
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

    if (compile_spirv_module(AlphaBlending_data, opt, spirv_data) == 0)
    {
        pipe = new Pipeline(vkdev);
        pipe->set_optimal_local_size_xyz(16, 16, 1);
        pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    if (compile_spirv_module(AlphaBlending_alpha_data, opt, spirv_data) == 0)
    {
        pipe_alpha = new Pipeline(vkdev);
        pipe_alpha->set_optimal_local_size_xyz(16, 16, 1);
        pipe_alpha->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }
    cmd->reset();
}

AlphaBlending_vulkan::~AlphaBlending_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void AlphaBlending_vulkan::upload_param(const VkMat& src1, const VkMat& src2, VkMat& dst, int x, int y) const
{
    std::vector<VkMat> bindings(12);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (src1.type == IM_DT_INT8)      bindings[4] = src1;
    else if (src1.type == IM_DT_INT16)     bindings[5] = src1;
    else if (src1.type == IM_DT_FLOAT16)   bindings[6] = src1;
    else if (src1.type == IM_DT_FLOAT32)   bindings[7] = src1;

    if      (src2.type == IM_DT_INT8)      bindings[8] = src2;
    else if (src2.type == IM_DT_INT16)     bindings[9] = src2;
    else if (src2.type == IM_DT_FLOAT16)   bindings[10] = src2;
    else if (src2.type == IM_DT_FLOAT32)   bindings[11] = src2;

    std::vector<vk_constant_type> constants(17);
    constants[0].i = src1.w;
    constants[1].i = src1.h;
    constants[2].i = src1.c;
    constants[3].i = src1.color_format;
    constants[4].i = src1.type;
    constants[5].i = src2.w;
    constants[6].i = src2.h;
    constants[7].i = src2.c;
    constants[8].i = src2.color_format;
    constants[9].i = src2.type;
    constants[10].i = dst.w;
    constants[11].i = dst.h;
    constants[12].i = dst.c;
    constants[13].i = dst.color_format;
    constants[14].i = dst.type;
    constants[15].i = x;
    constants[16].i = y;
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void AlphaBlending_vulkan::blend(const ImMat& src1, const ImMat& src2, ImMat& dst, int x, int y) const
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    // TODO::Dicky need check dims?
    //if (src1.dims != src2.dims || src1.color_space != src2.color_space || src1.color_range != src2.color_range)
    //    return;

    if (x >= src2.w || y >= src2.h || x <= -src1.w || y <= -src1.h)
        return;

    VkMat dst_gpu;
    dst_gpu.create_type(src2.w, src2.h, 4, dst.type, opt.blob_vkallocator);

    VkMat src1_gpu;
    if (src1.device == IM_DD_VULKAN)
    {
        src1_gpu = src1;
    }
    else if (src1.device == IM_DD_CPU)
    {
        cmd->record_clone(src1, src1_gpu, opt);
    }

    VkMat src2_gpu;
    if (src2.device == IM_DD_VULKAN)
    {
        src2_gpu = src2;
    }
    else if (src2.device == IM_DD_CPU)
    {
        cmd->record_clone(src2, src2_gpu, opt);
    }

    upload_param(src1_gpu, src2_gpu, dst_gpu, x, y);

    // download
    if (dst.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, dst, opt);
    else if (dst.device == IM_DD_VULKAN)
        dst = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}

void AlphaBlending_vulkan::upload_param(const VkMat& src1, const VkMat& src2, VkMat& dst, float alpha, int x, int y) const
{
    std::vector<VkMat> bindings(12);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (src1.type == IM_DT_INT8)      bindings[4] = src1;
    else if (src1.type == IM_DT_INT16)     bindings[5] = src1;
    else if (src1.type == IM_DT_FLOAT16)   bindings[6] = src1;
    else if (src1.type == IM_DT_FLOAT32)   bindings[7] = src1;

    if      (src2.type == IM_DT_INT8)      bindings[8] = src2;
    else if (src2.type == IM_DT_INT16)     bindings[9] = src2;
    else if (src2.type == IM_DT_FLOAT16)   bindings[10] = src2;
    else if (src2.type == IM_DT_FLOAT32)   bindings[11] = src2;

    std::vector<vk_constant_type> constants(18);
    constants[0].i = src1.w;
    constants[1].i = src1.h;
    constants[2].i = src1.c;
    constants[3].i = src1.color_format;
    constants[4].i = src1.type;
    constants[5].i = src2.w;
    constants[6].i = src2.h;
    constants[7].i = src2.c;
    constants[8].i = src2.color_format;
    constants[9].i = src2.type;
    constants[10].i = dst.w;
    constants[11].i = dst.h;
    constants[12].i = dst.c;
    constants[13].i = dst.color_format;
    constants[14].i = dst.type;
    constants[15].i = x;
    constants[16].i = y;
    constants[17].f = alpha;
    cmd->record_pipeline(pipe_alpha, bindings, constants, dst);
}

void AlphaBlending_vulkan::blend(const ImMat& src1, const ImMat& src2, ImMat& dst, float alpha, int x, int y) const
{
    if (!vkdev || !pipe_alpha || !cmd)
    {
        return;
    }

    // TODO::Dicky need check dims?
    //if (src1.dims != src2.dims || src1.color_space != src2.color_space || src1.color_range != src2.color_range)
    //    return;

    if (x >= src2.w || y >= src2.h || x <= -src1.w || y <= -src1.h)
        return;

    VkMat dst_gpu;
    dst_gpu.create_type(src2.w, src2.h, 4, dst.type, opt.blob_vkallocator);

    VkMat src1_gpu;
    if (src1.device == IM_DD_VULKAN)
    {
        src1_gpu = src1;
    }
    else if (src1.device == IM_DD_CPU)
    {
        cmd->record_clone(src1, src1_gpu, opt);
    }

    VkMat src2_gpu;
    if (src2.device == IM_DD_VULKAN)
    {
        src2_gpu = src2;
    }
    else if (src2.device == IM_DD_CPU)
    {
        cmd->record_clone(src2, src2_gpu, opt);
    }

    upload_param(src1_gpu, src2_gpu, dst_gpu, alpha, x, y);

    // download
    if (dst.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, dst, opt);
    else if (dst.device == IM_DD_VULKAN)
        dst = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}

} //namespace ImGui 
