#include "Harris_vulkan.h"
#include "Filter2DS_shader.h"
#include "Harris_shader.h"
#include "ImVulkanShader.h"

namespace ImGui
{
Harris_vulkan::Harris_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);

    compile_spirv_module(PrewittFilter_data, opt, spirv_prewitt_data);
    pipe_prewitt = new Pipeline(vkdev);
    pipe_prewitt->set_optimal_local_size_xyz(16, 16, 1);
    pipe_prewitt->create(spirv_prewitt_data.data(), spirv_prewitt_data.size() * 4, specializations);

    compile_spirv_module(NMSFilter_data, opt, spirv_nms_data);
    pipe_nms = new Pipeline(vkdev);
    pipe_nms->set_optimal_local_size_xyz(16, 16, 1);
    pipe_nms->create(spirv_nms_data.data(), spirv_nms_data.size() * 4, specializations);

    compile_spirv_module(HarrisFilter_data, opt, spirv_harris_data);
    pipe_harris = new Pipeline(vkdev);
    pipe_harris->set_optimal_local_size_xyz(16, 16, 1);
    pipe_harris->create(spirv_harris_data.data(), spirv_harris_data.size() * 4, specializations);

    compile_spirv_module(FilterColumnF32_data, opt, spirv_column_data);
    pipe_column = new Pipeline(vkdev);
    pipe_column->set_optimal_local_size_xyz(16, 16, 1);
    pipe_column->create(spirv_column_data.data(), spirv_column_data.size() * 4, specializations);

    compile_spirv_module(FilterRowF32_data, opt, spirv_row_data);
    pipe_row = new Pipeline(vkdev);
    pipe_row->set_optimal_local_size_xyz(16, 16, 1);
    pipe_row->create(spirv_row_data.data(), spirv_row_data.size() * 4, specializations);

    cmd->reset();

    prepare_kernel();
}

Harris_vulkan::~Harris_vulkan()
{
    if (vkdev)
    {
        if (pipe_harris) { delete pipe_harris; pipe_harris = nullptr; }
        if (pipe_prewitt) { delete pipe_prewitt; pipe_prewitt = nullptr; }
        if (pipe_nms) { delete pipe_nms; pipe_nms = nullptr; }
        if (pipe_column) { delete pipe_column; pipe_column = nullptr; }
        if (pipe_row) { delete pipe_row; pipe_row = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Harris_vulkan::prepare_kernel()
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

void Harris_vulkan::upload_param(const VkMat& src, VkMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity)
{
    if (blurRadius != _blurRadius)
    {
        blurRadius = _blurRadius;
        prepare_kernel();
    }
    VkMat vk_prewitt;
    vk_prewitt.create_type(dst.w, dst.h, 4, IM_DT_FLOAT32, opt.blob_vkallocator);
    std::vector<VkMat> prewitt_bindings(2);
    prewitt_bindings[0] = src;
    prewitt_bindings[1] = vk_prewitt;
    std::vector<vk_constant_type> prewitt_constants(5);
    prewitt_constants[0].i = src.w;
    prewitt_constants[1].i = src.h;
    prewitt_constants[2].i = src.c;
    prewitt_constants[3].i = src.color_format;
    prewitt_constants[4].f = edgeStrength;
    cmd->record_pipeline(pipe_prewitt, prewitt_bindings, prewitt_constants, vk_prewitt);

    VkMat vk_column;
    vk_column.create_like(vk_prewitt, opt.blob_vkallocator);
    std::vector<vk_constant_type> blur_constants(8);
    blur_constants[0].i = src.w;
    blur_constants[1].i = src.h;
    blur_constants[2].i = src.c;
    blur_constants[3].i = src.color_format;
    blur_constants[4].i = xksize;
    blur_constants[5].i = yksize;
    blur_constants[6].i = xanchor;
    blur_constants[7].i = yanchor;

    std::vector<VkMat> column_bindings(3);
    column_bindings[0] = vk_prewitt;
    column_bindings[1] = vk_column;
    column_bindings[2] = vk_kernel;
    cmd->record_pipeline(pipe_column, column_bindings, blur_constants, vk_column);

    VkMat vk_blur;
    vk_blur.create_like(vk_prewitt, opt.blob_vkallocator);
    std::vector<VkMat> row_bindings(3);
    row_bindings[0] = vk_column;
    row_bindings[1] = vk_blur;
    row_bindings[2] = vk_kernel;
    cmd->record_pipeline(pipe_row, row_bindings, blur_constants, vk_blur);

    VkMat vk_harris;
    vk_harris.create_type(dst.w, dst.h, IM_DT_FLOAT32, opt.blob_vkallocator);
    std::vector<VkMat> harris_bindings(2);
    harris_bindings[0] = vk_blur;
    harris_bindings[1] = vk_harris;
    std::vector<vk_constant_type> harris_constants(6);
    harris_constants[0].i = vk_blur.w;
    harris_constants[1].i = vk_blur.h;
    harris_constants[2].i = vk_blur.c;
    harris_constants[3].i = vk_blur.color_format;
    harris_constants[4].f = harris;
    harris_constants[5].f = sensitivity;
    cmd->record_pipeline(pipe_harris, harris_bindings, harris_constants, vk_harris);

    VkMat vk_nms;
    vk_nms.create_type(dst.w, dst.h, IM_DT_FLOAT32, opt.blob_vkallocator);
    std::vector<VkMat> nms_bindings(3);
    nms_bindings[0] = vk_harris;
    nms_bindings[1] = src;
    nms_bindings[2] = dst;//vk_nms;
    std::vector<vk_constant_type> nms_constants(5);
    nms_constants[0].i = dst.w;
    nms_constants[1].i = dst.h;
    nms_constants[2].i = dst.c;
    nms_constants[3].i = dst.color_format;
    nms_constants[4].f = threshold;
    cmd->record_pipeline(pipe_nms, nms_bindings, nms_constants, dst);
}

void Harris_vulkan::filter(const ImMat& src, ImMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity)
{
    if (!vkdev || !pipe_harris || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, _blurRadius, edgeStrength, threshold, harris, sensitivity);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Harris_vulkan::filter(const ImMat& src, VkMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity)
{
    if (!vkdev || !pipe_harris  || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, _blurRadius, edgeStrength, threshold, harris, sensitivity);

    cmd->submit_and_wait();
    cmd->reset();
}

void Harris_vulkan::filter(const VkMat& src, ImMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity)
{
    if (!vkdev || !pipe_harris || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;   // for render

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, _blurRadius, edgeStrength, threshold, harris, sensitivity);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Harris_vulkan::filter(const VkMat& src, VkMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity)
{
    if (!vkdev || !pipe_harris || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    upload_param(src, dst, _blurRadius, edgeStrength, threshold, harris, sensitivity);

    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui
