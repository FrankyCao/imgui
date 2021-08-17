#include "USM_vulkan.h"
#include "Filter2DS_shader.h"
#include "USM_shader.h"
#include "ImVulkanShader.h"

namespace ImGui
{
USM_vulkan::USM_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);

    compile_spirv_module(USMFilter_data, opt, spirv_data);
    pipe = new Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(8, 8, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);

    compile_spirv_module(FilterColumn_data, opt, spirv_column_data);
    pipe_column = new Pipeline(vkdev);
    pipe_column->set_optimal_local_size_xyz(16, 16, 1);
    pipe_column->create(spirv_column_data.data(), spirv_column_data.size() * 4, specializations);

    compile_spirv_module(FilterRow_data, opt, spirv_row_data);
    pipe_row = new Pipeline(vkdev);
    pipe_row->set_optimal_local_size_xyz(16, 16, 1);
    pipe_row->create(spirv_row_data.data(), spirv_row_data.size() * 4, specializations);

    cmd->reset();

    prepare_kernel();
}

USM_vulkan::~USM_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (pipe_column) { delete pipe_column; pipe_column = nullptr; }
        if (pipe_row) { delete pipe_row; pipe_row = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void USM_vulkan::prepare_kernel()
{
    int ksize = blurRadius * 2 + 1;
    if (sigma <= 0.0f) 
    {
        sigma = ((ksize - 1) * 0.5 - 1) * 0.3 + 0.8;
    }
    double scale = 1.0f / (sigma * sigma * 2.0);
    //double cons = scale / M_PI;
    double sum = 0.0;

    //kernel.create(ksize, ksize, size_t(4u), 1);
    kernel.create(ksize, size_t(4u), 1);
    for (int i = 0; i < ksize; i++) 
    {
        int x = i - (ksize - 1) / 2;
        //kernel.at<float>(i, j) = cons * exp(-scale * (x * x + y * y));
        kernel.at<float>(i) = exp(-scale * (x * x));
        sum += kernel.at<float>(i);
    }

    sum = 1.0 / sum;
    kernel *= (float)(sum);
    VkTransfer tran(vkdev);
    tran.record_upload(kernel, vk_kernel, opt, false);
    tran.submit_and_wait();

    xksize = yksize = ksize;
    xanchor = yanchor = blurRadius;
}

void USM_vulkan::upload_param(const VkMat& src, VkMat& dst, float _sigma, float amount, float threshold)
{
    if (sigma != _sigma)
    {
        sigma = _sigma;
        prepare_kernel();
    }

    VkMat vk_blur;
    vk_blur.create_like(dst, opt.blob_vkallocator);
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
    row_bindings[1] = vk_blur;
    row_bindings[2] = vk_kernel;
    cmd->record_pipeline(pipe_row, row_bindings, constants, vk_blur);

    std::vector<VkMat> usm_bindings(3);
    usm_bindings[0] = src;
    usm_bindings[1] = vk_blur;
    usm_bindings[2] = dst;
    std::vector<vk_constant_type> usm_constants(6);
    usm_constants[0].i = src.w;
    usm_constants[1].i = src.h;
    usm_constants[2].i = src.c;
    usm_constants[3].i = src.color_format;
    usm_constants[4].f = amount;
    usm_constants[5].f = threshold;
    cmd->record_pipeline(pipe, usm_bindings, usm_constants, dst);

}

void USM_vulkan::filter(const ImMat& src, ImMat& dst, float sigma, float amount, float threshold)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, sigma, amount, threshold);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void USM_vulkan::filter(const ImMat& src, VkMat& dst, float sigma, float amount, float threshold)
{
    if (!vkdev || !pipe  || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, sigma, amount, threshold);

    cmd->submit_and_wait();
    cmd->reset();
}

void USM_vulkan::filter(const VkMat& src, ImMat& dst, float sigma, float amount, float threshold)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, sigma, amount, threshold);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void USM_vulkan::filter(const VkMat& src, VkMat& dst, float sigma, float amount, float threshold)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    upload_param(src, dst, sigma, amount, threshold);

    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui
