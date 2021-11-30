#include "imvk_copy_make_border.h"
#include "imvk_command.h"
#include "imvk_copy_make_border_shader.h"

namespace ImGui 
{
Copy_Make_Border_vulkan::Copy_Make_Border_vulkan(int gpu)
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

    if (compile_spirv_module(Filter_data, opt, spirv_data) == 0)
    {
        pipe = new Pipeline(vkdev);
        pipe->set_optimal_local_size_xyz(16, 16, 1);
        pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    }
    
    cmd->reset();
}

Copy_Make_Border_vulkan::~Copy_Make_Border_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void Copy_Make_Border_vulkan::upload_param(const VkMat& src, VkMat& dst, int top, int bottom, int left, int right, float value)
{
    std::vector<VkMat> bindings(8);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst; 
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (src.type == IM_DT_INT8)     bindings[4] = src;
    else if (src.type == IM_DT_INT16)    bindings[5] = src;
    else if (src.type == IM_DT_FLOAT16)  bindings[6] = src;
    else if (src.type == IM_DT_FLOAT32)  bindings[7] = src;

    std::vector<vk_constant_type> constants(15);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = src.type;
    constants[5].i = dst.w;
    constants[6].i = dst.h;
    constants[7].i = dst.c;
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    constants[10].i = top;
    constants[11].i = bottom;
    constants[12].i = left;
    constants[13].i = right;
    constants[14].f = value;
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void Copy_Make_Border_vulkan::forward(const ImMat& bottom_blob, ImMat& top_blob, int top, int bottom, int left, int right, float value)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    auto color_format = top_blob.color_format;
    int channels = IM_ISALPHA(color_format) ? 4 : IM_ISRGB(color_format) ? 3 : IM_ISMONO(color_format) ? 1 : 4;
    top_blob.create_type(bottom_blob.w + left + right, bottom_blob.h + top + bottom, channels, bottom_blob.type);
    top_blob.color_format = color_format;

    VkMat out_gpu;
    out_gpu.create_like(top_blob, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(bottom_blob, in_gpu, opt);

    upload_param(in_gpu, out_gpu, top, bottom, left, right, value);

    // download
    cmd->record_clone(out_gpu, top_blob, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void Copy_Make_Border_vulkan::forward(const VkMat& bottom_blob, VkMat& top_blob, int top, int bottom, int left, int right, float value)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    auto color_format = top_blob.color_format;
    int channels = IM_ISALPHA(color_format) ? 4 : IM_ISRGB(color_format) ? 3 : IM_ISMONO(color_format) ? 1 : 4;
    top_blob.create_type(bottom_blob.w + left + right, bottom_blob.h + top + bottom, channels, bottom_blob.type, opt.blob_vkallocator);
    top_blob.color_format = color_format;
    
    upload_param(bottom_blob, top_blob, top, bottom, left, right, value);

    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui
