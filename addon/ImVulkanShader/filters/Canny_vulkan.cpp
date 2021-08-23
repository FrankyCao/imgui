#include "Canny_vulkan.h"
#include "Filter2DS_shader.h"
#include "Canny_shader.h"
#include "ImVulkanShader.h"

namespace ImGui
{
Canny_vulkan::Canny_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);

    compile_spirv_module(DSobelFilter_data, opt, spirv_dsobel_data);
    pipe_dsobel = new Pipeline(vkdev);
    pipe_dsobel->set_optimal_local_size_xyz(16, 16, 1);
    pipe_dsobel->create(spirv_dsobel_data.data(), spirv_dsobel_data.size() * 4, specializations);

    compile_spirv_module(NMSFilter_data, opt, spirv_nms_data);
    pipe_nms = new Pipeline(vkdev);
    pipe_nms->set_optimal_local_size_xyz(16, 16, 1);
    pipe_nms->create(spirv_nms_data.data(), spirv_nms_data.size() * 4, specializations);

    compile_spirv_module(CannyFilter_data, opt, spirv_data);
    pipe = new Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(16, 16, 1);
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

Canny_vulkan::~Canny_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (pipe_dsobel) { delete pipe_dsobel; pipe_dsobel = nullptr; }
        if (pipe_nms) { delete pipe_nms; pipe_nms = nullptr; }
        if (pipe_column) { delete pipe_column; pipe_column = nullptr; }
        if (pipe_row) { delete pipe_row; pipe_row = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Canny_vulkan::prepare_kernel()
{
    int ksize = blurRadius * 2 + 1;
    if (sigma <= 0.0f) 
    {
        sigma = ((ksize - 1) * 0.5 - 1) * 0.3 + 0.8;
    }
    double scale = 1.0f / (sigma * sigma * 2.0);
    double sum = 0.0;

    kernel.create(ksize, size_t(4u), 1);
    for (int i = 0; i < ksize; i++) 
    {
        int x = i - (ksize - 1) / 2;
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

void Canny_vulkan::upload_param(const VkMat& src, VkMat& dst, int _blurRadius, float minThreshold, float maxThreshold)
{
    if (blurRadius != _blurRadius)
    {
        blurRadius = _blurRadius;
        prepare_kernel();
    }

    VkMat vk_blur;
    vk_blur.create_like(dst, opt.blob_vkallocator);
    VkMat vk_bobel;
    vk_bobel.create_type(dst.w, dst.h, 3, IM_DT_FLOAT32, opt.blob_vkallocator);
    VkMat vk_nms;
    vk_nms.create_type(dst.w, dst.h, IM_DT_FLOAT32, opt.blob_vkallocator);
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

    std::vector<VkMat> sobel_bindings(2);
    sobel_bindings[0] = vk_blur;
    sobel_bindings[1] = vk_bobel;
    std::vector<vk_constant_type> sobel_constants(5);
    sobel_constants[0].i = src.w;
    sobel_constants[1].i = src.h;
    sobel_constants[2].i = src.c;
    sobel_constants[3].i = src.color_format;
    sobel_constants[4].f = 1.f;
    cmd->record_pipeline(pipe_dsobel, sobel_bindings, sobel_constants, vk_bobel);

    std::vector<VkMat> nms_bindings(2);
    nms_bindings[0] = vk_bobel;
    nms_bindings[1] = vk_nms;
    std::vector<vk_constant_type> nms_constants(6);
    nms_constants[0].i = src.w;
    nms_constants[1].i = src.h;
    nms_constants[2].i = src.c;
    nms_constants[3].i = src.color_format;
    nms_constants[4].f = minThreshold;
    nms_constants[5].f = maxThreshold;
    cmd->record_pipeline(pipe_nms, nms_bindings, nms_constants, vk_nms);

    std::vector<VkMat> canny_bindings(2);
    canny_bindings[0] = vk_nms;
    canny_bindings[1] = dst;
    std::vector<vk_constant_type> canny_constants(4);
    canny_constants[0].i = dst.w;
    canny_constants[1].i = dst.h;
    canny_constants[2].i = dst.c;
    canny_constants[3].i = dst.color_format;
    cmd->record_pipeline(pipe, canny_bindings, canny_constants, dst);
}

void Canny_vulkan::filter(const ImMat& src, ImMat& dst, int _blurRadius, float minThreshold, float maxThreshold)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, _blurRadius, minThreshold, maxThreshold);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Canny_vulkan::filter(const ImMat& src, VkMat& dst, int _blurRadius, float minThreshold, float maxThreshold)
{
    if (!vkdev || !pipe  || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, _blurRadius, minThreshold, maxThreshold);

    cmd->submit_and_wait();
    cmd->reset();
}

void Canny_vulkan::filter(const VkMat& src, ImMat& dst, int _blurRadius, float minThreshold, float maxThreshold)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, _blurRadius, minThreshold, maxThreshold);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Canny_vulkan::filter(const VkMat& src, VkMat& dst, int _blurRadius, float minThreshold, float maxThreshold)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    upload_param(src, dst, _blurRadius, minThreshold, maxThreshold);

    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui
