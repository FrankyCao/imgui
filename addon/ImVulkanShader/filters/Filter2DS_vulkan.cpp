#include "Filter2DS_vulkan.h"
#include "Filter2DS_shader.h"
#include "ImVulkanShader.h"

namespace ImGui 
{
Filter2DS_vulkan::Filter2DS_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);

    std::vector<vk_specialization_type> specializations(0);

    compile_spirv_module(FilterColumn_data, opt, spirv_column_data);
    pipe_column = new Pipeline(vkdev);
    pipe_column->set_optimal_local_size_xyz(16, 16, 1);
    pipe_column->create(spirv_column_data.data(), spirv_column_data.size() * 4, specializations);

    compile_spirv_module(FilterRow_data, opt, spirv_row_data);
    pipe_row = new Pipeline(vkdev);
    pipe_row->set_optimal_local_size_xyz(16, 16, 1);
    pipe_row->create(spirv_row_data.data(), spirv_row_data.size() * 4, specializations);
    
    cmd->reset();
}

Filter2DS_vulkan::~Filter2DS_vulkan()
{
    if (vkdev)
    {
        if (pipe_column) { delete pipe_column; pipe_column = nullptr; }
        if (pipe_row) { delete pipe_row; pipe_row = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Filter2DS_vulkan::upload_param(const VkMat& src, VkMat& dst) const
{
    std::vector<vk_constant_type> constants(8);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = xksize;
    constants[5].i = yksize;
    constants[6].i = xanchor;
    constants[7].i = yanchor;

    VkMat vk_column;
    vk_column.create_like(dst, opt.blob_vkallocator);

    std::vector<VkMat> column_bindings(3);
    column_bindings[0] = src;
    column_bindings[1] = vk_column;
    column_bindings[2] = vk_kernel;
    cmd->record_pipeline(pipe_column, column_bindings, constants, vk_column);

    std::vector<VkMat> row_bindings(3);
    row_bindings[0] = vk_column;
    row_bindings[1] = dst;
    row_bindings[2] = vk_kernel;
    cmd->record_pipeline(pipe_row, row_bindings, constants, dst);
}

void Filter2DS_vulkan::filter(const ImMat& src, ImMat& dst) const
{
    if (!vkdev || !pipe_column || !pipe_row || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32);
    dst.color_format = IM_CF_ABGR;

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Filter2DS_vulkan::filter(const ImMat& src, VkMat& dst) const
{
    if (!vkdev || !pipe_column || !pipe_row || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst);

    cmd->submit_and_wait();
    cmd->reset();
}

void Filter2DS_vulkan::filter(const VkMat& src, ImMat& dst) const
{
    if (!vkdev || !pipe_column || !pipe_row || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32);
    dst.color_format = IM_CF_ABGR;

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Filter2DS_vulkan::filter(const VkMat& src, VkMat& dst) const
{
    if (!vkdev || !pipe_column || !pipe_row || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;
    
    upload_param(src, dst);

    cmd->submit_and_wait();
    cmd->reset();
}
} //namespace ImGui 
