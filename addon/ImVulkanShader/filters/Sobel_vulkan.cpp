#include "Sobel_vulkan.h"
#include "Sobel_shader.h"
#include "ImVulkanShader.h"

Sobel::Sobel(int gpu)
{
    vkdev = ImGui::get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = true;
    opt.use_fp16_arithmetic = true;
    cmd = new ImGui::VkCompute(vkdev);

    std::vector<ImGui::vk_specialization_type> specializations(0);
    ImGui::compile_spirv_module(Filter_data, opt, spirv_data);
    pipe = new ImGui::Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(8, 8, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    
    cmd->reset();
}

Sobel::~Sobel()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Sobel::upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst, float edgeStrength)
{
    std::vector<ImGui::VkMat> bindings(2);
    bindings[0] = src;
    bindings[1] = dst;
    std::vector<ImGui::vk_constant_type> constants(5);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].f = edgeStrength;
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void Sobel::filter(const ImGui::ImMat& src, ImGui::ImMat& dst, float edgeStrength)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;   // for render

    ImGui::VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    ImGui::VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, edgeStrength);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Sobel::filter(const ImGui::ImMat& src, ImGui::VkMat& dst, float edgeStrength)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render

    ImGui::VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, edgeStrength);

    cmd->submit_and_wait();
    cmd->reset();
}

void Sobel::filter(const ImGui::VkMat& src, ImGui::ImMat& dst, float edgeStrength)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, IM_DT_INT8);
    dst.color_format = IM_CF_ABGR;   // for render

    ImGui::VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, edgeStrength);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Sobel::filter(const ImGui::VkMat& src, ImGui::VkMat& dst, float edgeStrength)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, IM_DT_INT8, opt.blob_vkallocator);
    dst.color_format = IM_CF_ABGR;   // for render
    
    upload_param(src, dst, edgeStrength);

    cmd->submit_and_wait();
    cmd->reset();
}
