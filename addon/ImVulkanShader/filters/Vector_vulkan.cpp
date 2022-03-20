#include "Vector_vulkan.h"
#include "Vector_shader.h"
#include "ImVulkanShader.h"

namespace ImGui
{
Vector_vulkan::Vector_vulkan(int gpu)
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
    if (ImGui::compile_spirv_module(Vector_set_data, opt, spirv_data) == 0)
    {
        pipe_set = new ImGui::Pipeline(vkdev);
        pipe_set->set_optimal_local_size_xyz(16, 16, 1);
        pipe_set->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    if (compile_spirv_module(Vector_data, opt, spirv_data) == 0)
    {
        pipe = new Pipeline(vkdev);
        pipe->set_optimal_local_size_xyz(1, 512, 1);
        pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    if (ImGui::compile_spirv_module(Vector_merge_data, opt, spirv_data) == 0)
    {
        pipe_merge = new ImGui::Pipeline(vkdev);
        pipe_merge->set_optimal_local_size_xyz(16, 16, 1);
        pipe_merge->create(spirv_data.data(), spirv_data.size() * 4, specializations);
        spirv_data.clear();
    }

    buffer.create_type(size, size, IM_DT_INT32, opt.blob_vkallocator);
    buffer.color_format = IM_CF_GRAY;
    cmd->reset();
}

Vector_vulkan::~Vector_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Vector_vulkan::upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst, float intensity)
{
    std::vector<ImGui::VkMat> bindings_set(1);
    bindings_set[0] = buffer;
    std::vector<ImGui::vk_constant_type> constants_set(2);
    constants_set[0].i = buffer.w;
    constants_set[1].i = buffer.h;
    cmd->record_pipeline(pipe_set, bindings_set, constants_set, buffer);

    std::vector<ImGui::VkMat> bindings(5);
    if      (src.type == IM_DT_INT8)     bindings[0] = src;
    else if (src.type == IM_DT_INT16)    bindings[1] = src;
    else if (src.type == IM_DT_FLOAT16)  bindings[2] = src;
    else if (src.type == IM_DT_FLOAT32)  bindings[3] = src;
    bindings[4] = buffer;
    std::vector<ImGui::vk_constant_type> constants(8);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = src.type;
    constants[5].i = buffer.w;
    constants[6].i = buffer.h;
    constants[7].i = buffer.c;
    cmd->record_pipeline(pipe, bindings, constants, buffer);

    std::vector<ImGui::VkMat> bindings_merge(5);
    if      (dst.type == IM_DT_INT8)     bindings_merge[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings_merge[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings_merge[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings_merge[3] = dst;

    bindings_merge[4] = buffer;
    std::vector<ImGui::vk_constant_type> constants_merge(11);
    constants_merge[0].i = buffer.w;
    constants_merge[1].i = buffer.h;
    constants_merge[2].i = buffer.c;
    constants_merge[3].i = buffer.color_format;
    constants_merge[4].i = buffer.type;
    constants_merge[5].i = dst.w;
    constants_merge[6].i = dst.h;
    constants_merge[7].i = dst.c;
    constants_merge[8].i = dst.color_format;
    constants_merge[9].i = dst.type;
    constants_merge[10].f = intensity;
    cmd->record_pipeline(pipe_merge, bindings_merge, constants_merge, dst);
}

void Vector_vulkan::scope(const ImGui::ImMat& src, ImGui::ImMat& dst, float intensity)
{
    if (!vkdev || !pipe || !pipe_set || !pipe_merge || !cmd)
    {
        return;
    }

    VkMat dst_gpu;
    dst_gpu.create_type(size, size, 4, IM_DT_INT8, opt.blob_vkallocator);

    VkMat src_gpu;
    if (src.device == IM_DD_VULKAN)
    {
        src_gpu = src;
    }
    else if (src.device == IM_DD_CPU)
    {
        cmd->record_clone(src, src_gpu, opt);
    }

    upload_param(src_gpu, dst_gpu, intensity);

    // download
    if (dst.device == IM_DD_CPU)
        cmd->record_clone(dst_gpu, dst, opt);
    else if (dst.device == IM_DD_VULKAN)
        dst = dst_gpu;
    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui
