#include "Waveform_vulkan.h"
#include "Waveform_shader.h"
#include "ImVulkanShader.h"

namespace ImGui
{
Waveform_vulkan::Waveform_vulkan(int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    opt.use_fp16_storage = false;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;
    if (compile_spirv_module(Waveform_data, opt, spirv_data) == 0)
    {
        pipe = new Pipeline(vkdev);
        pipe->set_optimal_local_size_xyz(16, 16, 4);
        pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }
    if (compile_spirv_module(ConvInt2Mat_data, opt, spirv_data) == 0)
    {
        pipe_conv = new Pipeline(vkdev);
        pipe_conv->set_optimal_local_size_xyz(16, 16, 1);
        pipe_conv->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }
    cmd->reset();
}

Waveform_vulkan::~Waveform_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (pipe_conv) { delete pipe_conv; pipe_conv = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Waveform_vulkan::upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst, float fintensity, bool separate)
{
    ImGui::ImMat dst_cpu;
    dst_cpu.create_type(dst.w, dst.h, dst.c, IM_DT_INT32);
    ImGui::VkMat dst_gpu_int32;
    cmd->record_clone(dst_cpu, dst_gpu_int32, opt);
    std::vector<VkMat> bindings(5);
    if      (src.type == IM_DT_INT8)     bindings[0] = src;
    else if (src.type == IM_DT_INT16)    bindings[1] = src;
    else if (src.type == IM_DT_FLOAT16)  bindings[2] = src;
    else if (src.type == IM_DT_FLOAT32)  bindings[3] = src;
    bindings[4] = dst_gpu_int32;
    std::vector<vk_constant_type> constants(11);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = src.type;
    constants[5].i = dst_gpu_int32.w;
    constants[6].i = dst_gpu_int32.h;
    constants[7].i = dst_gpu_int32.c;
    constants[8].i = dst_gpu_int32.color_format;
    constants[9].i = dst_gpu_int32.type;
    constants[10].i = separate ? 1 : 0;
    cmd->record_pipeline(pipe, bindings, constants, dst_gpu_int32);

    std::vector<VkMat> conv_bindings(2);
    conv_bindings[0] = dst_gpu_int32;
    conv_bindings[1] = dst;

    std::vector<vk_constant_type> conv_constants(6);
    conv_constants[0].i = dst_gpu_int32.w;
    conv_constants[1].i = dst_gpu_int32.h;
    conv_constants[2].i = dst_gpu_int32.c;
    conv_constants[3].i = dst.c;
    conv_constants[4].i = dst.color_format;
    conv_constants[5].f = fintensity;
    cmd->record_pipeline(pipe_conv, conv_bindings, conv_constants, dst);
}

void Waveform_vulkan::scope(const ImGui::ImMat& src, ImGui::ImMat& dst, int level, float fintensity, bool separate)
{
    if (!vkdev || !pipe || !pipe_conv || !cmd)
    {
        return;
    }
    VkMat dst_gpu;
    dst_gpu.create_type(src.w, level, 4, IM_DT_INT8, opt.blob_vkallocator);

    VkMat src_gpu;
    if (src.device == IM_DD_VULKAN)
    {
        src_gpu = src;
    }
    else if (src.device == IM_DD_CPU)
    {
        cmd->record_clone(src, src_gpu, opt);
    }

    upload_param(src_gpu, dst_gpu, fintensity, separate);

    // download
    if (dst.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, dst, opt);
    else if (dst.device == IM_DD_VULKAN)
        dst = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui
