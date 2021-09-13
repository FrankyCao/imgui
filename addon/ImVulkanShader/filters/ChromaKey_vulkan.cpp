#include "ChromaKey_vulkan.h"
#include "ChromaKey_shader.h"
#include "ImVulkanShader.h"
#include "Filter2DS_shader.h"

namespace ImGui 
{
ChromaKey_vulkan::ChromaKey_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    cmd = new VkCompute(vkdev);

    std::vector<vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;

    compile_spirv_module(Filter_data, opt, spirv_data);
    pipe = new Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(16, 16, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();

#if FILTER_2DS_BLUR
    compile_spirv_module(FilterColumnMono_data, opt, spirv_data);
    pipe_blur_column = new Pipeline(vkdev);
    pipe_blur_column->set_optimal_local_size_xyz(16, 16, 1);
    pipe_blur_column->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();

    compile_spirv_module(FilterRowMono_data, opt, spirv_data);
    pipe_blur_row = new Pipeline(vkdev);
    pipe_blur_row->set_optimal_local_size_xyz(16, 16, 1);
    pipe_blur_row->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();
#else
    compile_spirv_module(Blur_data, opt, spirv_data);
    pipe_blur = new Pipeline(vkdev);
    pipe_blur->set_optimal_local_size_xyz(16, 16, 1);
    pipe_blur->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();
#endif

    compile_spirv_module(Sharpen_data, opt, spirv_data);
    pipe_sharpen = new Pipeline(vkdev);
    pipe_sharpen->set_optimal_local_size_xyz(16, 16, 1);
    pipe_sharpen->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();

    compile_spirv_module(Despill_data, opt, spirv_data);
    pipe_despill = new Pipeline(vkdev);
    pipe_despill->set_optimal_local_size_xyz(16, 16, 1);
    pipe_despill->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    spirv_data.clear();

    prepare_kernel();
    cmd->reset();
}

ChromaKey_vulkan::~ChromaKey_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
#if FILTER_2DS_BLUR
        if (pipe_blur_column) { delete pipe_blur_column; pipe_blur_column = nullptr; }
        if (pipe_blur_row) { delete pipe_blur_row; pipe_blur_row = nullptr; }
#else
        if (pipe_blur) { delete pipe_blur; pipe_blur = nullptr; }
#endif
        if (pipe_sharpen) { delete pipe_sharpen; pipe_sharpen = nullptr; }
        if (pipe_despill) { delete pipe_despill; pipe_despill = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void ChromaKey_vulkan::prepare_kernel()
{
    ksize = blurRadius * 2 + 1;
    if (sigma <= 0.0f) 
        sigma = ((ksize - 1) * 0.5 - 1) * 0.3 + 0.8;
    double scale = 1.0f / (sigma * sigma * 2.0);
    double sum = 0.0;
#if FILTER_2DS_BLUR
    kernel.create(ksize, size_t(4u), 1);
    for (int i = 0; i < ksize; i++) 
    {
        int x = i - (ksize - 1) / 2;
        kernel.at<float>(i) = exp(-scale * (x * x));
        sum += kernel.at<float>(i);
    }
#else
    double cons = scale / M_PI;
    kernel.create(ksize, ksize, size_t(4u), 1);
    for (int i = 0; i < ksize; i++)
    {
        for (int j = 0; j < ksize; j++)
        {
            int x = i - (ksize - 1) / 2;
            int y = j - (ksize - 1) / 2;
            kernel.at<float>(i, j) = cons * exp(-scale * (x * x + y * y));
            sum += kernel.at<float>(i, j);
        }
    }
#endif
    sum = 1.0 / sum;
    kernel *= (float)(sum);

    VkTransfer tran(vkdev);
    tran.record_upload(kernel, vk_kernel, opt, false);
    tran.submit_and_wait();
}

void ChromaKey_vulkan::upload_param(const VkMat& src, VkMat& dst,
                                    float lumaMask, ImVec4 chromaColor,
                                    float alphaCutoffMin, float alphaScale, float alphaExponent,
                                    bool alpha_only)
{
    
    VkMat alpha_mat;
    alpha_mat.create_type(src.w, src.h, 1, IM_DT_FLOAT16, opt.blob_vkallocator);

    std::vector<VkMat> bindings(5);
    if      (src.type == IM_DT_INT8)     bindings[0] = src;
    else if (src.type == IM_DT_INT16)    bindings[1] = src;
    else if (src.type == IM_DT_FLOAT16)  bindings[2] = src;
    else if (src.type == IM_DT_FLOAT32)  bindings[3] = src;
    bindings[4] = alpha_mat;

    std::vector<vk_constant_type> constants(17);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = src.type;
    constants[5].i = alpha_mat.w;
    constants[6].i = alpha_mat.h;
    constants[7].i = alpha_mat.c;
    constants[8].i = alpha_mat.color_format;
    constants[9].i = alpha_mat.type;
    constants[10].f = lumaMask;
    constants[11].f = chromaColor.x;
    constants[12].f = chromaColor.y;
    constants[13].f = chromaColor.z;
    constants[14].f = alphaCutoffMin;
    constants[15].f = alphaScale;
    constants[16].f = alphaExponent;
    cmd->record_pipeline(pipe, bindings, constants, alpha_mat);

#if FILTER_2DS_BLUR
    VkMat column_blur_alpha_mat;
    column_blur_alpha_mat.create_like(alpha_mat, opt.blob_vkallocator);
    std::vector<VkMat> column_blur_bindings(9);
    if      (column_blur_alpha_mat.type == IM_DT_INT8)     column_blur_bindings[0] = column_blur_alpha_mat;
    else if (column_blur_alpha_mat.type == IM_DT_INT16)    column_blur_bindings[1] = column_blur_alpha_mat;
    else if (column_blur_alpha_mat.type == IM_DT_FLOAT16)  column_blur_bindings[2] = column_blur_alpha_mat;
    else if (column_blur_alpha_mat.type == IM_DT_FLOAT32)  column_blur_bindings[3] = column_blur_alpha_mat;

    if      (alpha_mat.type == IM_DT_INT8)      column_blur_bindings[4] = alpha_mat;
    else if (alpha_mat.type == IM_DT_INT16)     column_blur_bindings[5] = alpha_mat;
    else if (alpha_mat.type == IM_DT_FLOAT16)   column_blur_bindings[6] = alpha_mat;
    else if (alpha_mat.type == IM_DT_FLOAT32)   column_blur_bindings[7] = alpha_mat;
    column_blur_bindings[8] = vk_kernel;

    std::vector<vk_constant_type> column_blur_constants(14);
    column_blur_constants[0].i = alpha_mat.w;
    column_blur_constants[1].i = alpha_mat.h;
    column_blur_constants[2].i = alpha_mat.c;
    column_blur_constants[3].i = alpha_mat.color_format;
    column_blur_constants[4].i = alpha_mat.type;
    column_blur_constants[5].i = column_blur_alpha_mat.w;
    column_blur_constants[6].i = column_blur_alpha_mat.h;
    column_blur_constants[7].i = column_blur_alpha_mat.c;
    column_blur_constants[8].i = column_blur_alpha_mat.color_format;
    column_blur_constants[9].i = column_blur_alpha_mat.type;
    column_blur_constants[10].i = ksize;
    column_blur_constants[11].i = ksize;
    column_blur_constants[12].i = blurRadius;
    column_blur_constants[13].i = blurRadius;
    cmd->record_pipeline(pipe_blur_column, column_blur_bindings, column_blur_constants, column_blur_alpha_mat);

    VkMat blur_alpha_mat;
    blur_alpha_mat.create_like(alpha_mat, opt.blob_vkallocator);
    std::vector<VkMat> row_blur_bindings(9);
    if      (blur_alpha_mat.type == IM_DT_INT8)     row_blur_bindings[0] = blur_alpha_mat;
    else if (blur_alpha_mat.type == IM_DT_INT16)    row_blur_bindings[1] = blur_alpha_mat;
    else if (blur_alpha_mat.type == IM_DT_FLOAT16)  row_blur_bindings[2] = blur_alpha_mat;
    else if (blur_alpha_mat.type == IM_DT_FLOAT32)  row_blur_bindings[3] = blur_alpha_mat;

    if      (column_blur_alpha_mat.type == IM_DT_INT8)      row_blur_bindings[4] = column_blur_alpha_mat;
    else if (column_blur_alpha_mat.type == IM_DT_INT16)     row_blur_bindings[5] = column_blur_alpha_mat;
    else if (column_blur_alpha_mat.type == IM_DT_FLOAT16)   row_blur_bindings[6] = column_blur_alpha_mat;
    else if (column_blur_alpha_mat.type == IM_DT_FLOAT32)   row_blur_bindings[7] = column_blur_alpha_mat;
    row_blur_bindings[8] = vk_kernel;
    std::vector<vk_constant_type> row_blur_constants(14);
    row_blur_constants[0].i = column_blur_alpha_mat.w;
    row_blur_constants[1].i = column_blur_alpha_mat.h;
    row_blur_constants[2].i = column_blur_alpha_mat.c;
    row_blur_constants[3].i = column_blur_alpha_mat.color_format;
    row_blur_constants[4].i = column_blur_alpha_mat.type;
    row_blur_constants[5].i = blur_alpha_mat.w;
    row_blur_constants[6].i = blur_alpha_mat.h;
    row_blur_constants[7].i = blur_alpha_mat.c;
    row_blur_constants[8].i = blur_alpha_mat.color_format;
    row_blur_constants[9].i = blur_alpha_mat.type;
    row_blur_constants[10].i = ksize;
    row_blur_constants[11].i = ksize;
    row_blur_constants[12].i = blurRadius;
    row_blur_constants[13].i = blurRadius;
    cmd->record_pipeline(pipe_blur_row, row_blur_bindings, row_blur_constants, blur_alpha_mat);
#else
    VkMat blur_alpha_mat;
    blur_alpha_mat.create_like(alpha_mat, opt.blob_vkallocator);
    std::vector<VkMat> blur_bindings(2);
    blur_bindings[0] = alpha_mat;
    blur_bindings[1] = blur_alpha_mat;
    std::vector<vk_constant_type> blur_constants(10);
    blur_constants[0].i = alpha_mat.w;
    blur_constants[1].i = alpha_mat.h;
    blur_constants[2].i = alpha_mat.c;
    blur_constants[3].i = alpha_mat.color_format;
    blur_constants[4].i = alpha_mat.type;
    blur_constants[5].i = blur_alpha_mat.w;
    blur_constants[6].i = blur_alpha_mat.h;
    blur_constants[7].i = blur_alpha_mat.c;
    blur_constants[8].i = blur_alpha_mat.color_format;
    blur_constants[9].i = blur_alpha_mat.type;
    cmd->record_pipeline(pipe_blur, blur_bindings, blur_constants, blur_alpha_mat);
#endif
/*
    VkMat sharpen_alpha_mat;
    sharpen_alpha_mat.create_like(blur_alpha_mat, opt.blob_vkallocator);
    std::vector<VkMat> sharpen_bindings(2);
    sharpen_bindings[0] = blur_alpha_mat;
    sharpen_bindings[1] = sharpen_alpha_mat;
    std::vector<vk_constant_type> sharpen_constants(11);
    sharpen_constants[0].i = blur_alpha_mat.w;
    sharpen_constants[1].i = blur_alpha_mat.h;
    sharpen_constants[2].i = blur_alpha_mat.c;
    sharpen_constants[3].i = blur_alpha_mat.color_format;
    sharpen_constants[4].i = blur_alpha_mat.type;
    sharpen_constants[5].i = sharpen_alpha_mat.w;
    sharpen_constants[6].i = sharpen_alpha_mat.h;
    sharpen_constants[7].i = sharpen_alpha_mat.c;
    sharpen_constants[8].i = sharpen_alpha_mat.color_format;
    sharpen_constants[9].i = sharpen_alpha_mat.type;
    sharpen_constants[10].f = 2.0f;
    cmd->record_pipeline(pipe_sharpen, sharpen_bindings, sharpen_constants, sharpen_alpha_mat);
*/
    std::vector<VkMat> despill_bindings(9);
    if      (dst.type == IM_DT_INT8)     despill_bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    despill_bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  despill_bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  despill_bindings[3] = dst;
    if      (src.type == IM_DT_INT8)     despill_bindings[4] = src;
    else if (src.type == IM_DT_INT16)    despill_bindings[5] = src;
    else if (src.type == IM_DT_FLOAT16)  despill_bindings[6] = src;
    else if (src.type == IM_DT_FLOAT32)  despill_bindings[7] = src;
    despill_bindings[8] = blur_alpha_mat;
    std::vector<vk_constant_type> despill_constants(19);
    despill_constants[0].i = src.w;
    despill_constants[1].i = src.h;
    despill_constants[2].i = src.c;
    despill_constants[3].i = src.color_format;
    despill_constants[4].i = src.type;
    despill_constants[5].i = dst.w;
    despill_constants[6].i = dst.h;
    despill_constants[7].i = dst.c;
    despill_constants[8].i = dst.color_format;
    despill_constants[9].i = dst.type;
    despill_constants[10].i = blur_alpha_mat.w;
    despill_constants[11].i = blur_alpha_mat.h;
    despill_constants[12].i = blur_alpha_mat.c;
    despill_constants[13].i = blur_alpha_mat.color_format;
    despill_constants[14].i = blur_alpha_mat.type;
    despill_constants[15].f = chromaColor.x;
    despill_constants[16].f = chromaColor.y;
    despill_constants[17].f = chromaColor.z;
    despill_constants[18].i = alpha_only ? 1 : 0;

    cmd->record_pipeline(pipe_despill, despill_bindings, despill_constants, dst);
}

void ChromaKey_vulkan::filter(const ImMat& src, ImMat& dst,
                            float lumaMask, ImVec4 chromaColor,
                            float alphaCutoffMin, float alphaScale, float alphaExponent,
                            bool alpha_only)
{
#if FILTER_2DS_BLUR
    if (!vkdev || !pipe || !pipe_blur_column || !pipe_blur_row || !cmd)
#else
    if (!vkdev || !pipe || !pipe_blur || !cmd)
#endif
    {
        return;
    }
    dst.create_type(src.w, src.h, alpha_only ? 1 : 4, dst.type);

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, lumaMask, chromaColor, alphaCutoffMin, alphaScale, alphaExponent, alpha_only);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ChromaKey_vulkan::filter(const ImMat& src, VkMat& dst,
                            float lumaMask, ImVec4 chromaColor,
                            float alphaCutoffMin, float alphaScale, float alphaExponent,
                            bool alpha_only)
{
#if FILTER_2DS_BLUR
    if (!vkdev || !pipe || !pipe_blur_column || !pipe_blur_row || !cmd)
#else
    if (!vkdev || !pipe || !pipe_blur || !cmd)
#endif
    {
        return;
    }
    dst.create_type(src.w, src.h, alpha_only ? 1 : 4, dst.type, opt.blob_vkallocator);

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, lumaMask, chromaColor, alphaCutoffMin, alphaScale, alphaExponent, alpha_only);

    cmd->submit_and_wait();
    cmd->reset();
}

void ChromaKey_vulkan::filter(const VkMat& src, ImMat& dst,
                            float lumaMask, ImVec4 chromaColor,
                            float alphaCutoffMin, float alphaScale, float alphaExponent,
                            bool alpha_only)
{
#if FILTER_2DS_BLUR
    if (!vkdev || !pipe || !pipe_blur_column || !pipe_blur_row || !cmd)
#else
    if (!vkdev || !pipe || !pipe_blur || !cmd)
#endif
    {
        return;
    }
    dst.create_type(src.w, src.h, alpha_only ? 1 : 4, dst.type);

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, lumaMask, chromaColor, alphaCutoffMin, alphaScale, alphaExponent, alpha_only);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void ChromaKey_vulkan::filter(const VkMat& src, VkMat& dst,
                            float lumaMask, ImVec4 chromaColor,
                            float alphaCutoffMin, float alphaScale, float alphaExponent,
                            bool alpha_only)
{
#if FILTER_2DS_BLUR
    if (!vkdev || !pipe || !pipe_blur_column || !pipe_blur_row || !cmd)
#else
    if (!vkdev || !pipe || !pipe_blur || !cmd)
#endif
    {
        return;
    }

    dst.create_type(src.w, src.h, alpha_only ? 1 : 4, dst.type, opt.blob_vkallocator);
    
    upload_param(src, dst, lumaMask, chromaColor, alphaCutoffMin, alphaScale, alphaExponent, alpha_only);

    cmd->submit_and_wait();
    cmd->reset();
}
} //namespace ImGui 
