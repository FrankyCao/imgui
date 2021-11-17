#include "HQDN3D_vulkan.h"
#include "HQDN3D_shader.h"
#include "ImVulkanShader.h"
#define LUMA_SPATIAL   0
#define LUMA_TMP       1
#define CHROMA_SPATIAL 2
#define CHROMA_TMP     3

#define PARAM1_DEFAULT 4.0
#define PARAM2_DEFAULT 3.0
#define PARAM3_DEFAULT 6.0

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

namespace ImGui
{
HQDN3D_vulkan::HQDN3D_vulkan(int width, int height, int channels, int gpu)
{
    vkdev = get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    opt.use_fp16_storage = true;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;

    if (compile_spirv_module(HQDN3D_data, opt, spirv_data) == 0)
    {
        pipe = new Pipeline(vkdev);
        pipe->set_optimal_local_size_xyz(16, 16, 1);
        pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    }
    
    cmd->reset();

    in_width = width;
    in_height = height;
    in_channels = channels;
    strength[LUMA_SPATIAL] = 0;
	strength[CHROMA_SPATIAL] = 0;
	strength[LUMA_TMP] = 0;
	strength[CHROMA_TMP] = 0;
	if (!strength[LUMA_SPATIAL])
        strength[LUMA_SPATIAL] = PARAM1_DEFAULT;
    if (!strength[CHROMA_SPATIAL])
        strength[CHROMA_SPATIAL] = PARAM2_DEFAULT * strength[LUMA_SPATIAL] / PARAM1_DEFAULT;
    if (!strength[LUMA_TMP])
        strength[LUMA_TMP]   = PARAM3_DEFAULT * strength[LUMA_SPATIAL] / PARAM1_DEFAULT;
    if (!strength[CHROMA_TMP])
        strength[CHROMA_TMP] = strength[LUMA_TMP] * strength[CHROMA_SPATIAL] / strength[LUMA_SPATIAL];

    for (int i = 0; i < 4; i++)
	{
        coef_cpu[i].create_type(512*16, IM_DT_INT16);
        precalc_coefs(strength[i], i);
    }
    prealloc_frames();
}

HQDN3D_vulkan::~HQDN3D_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void HQDN3D_vulkan::prealloc_frames(void)
{
    MutexLockGuard lock(param_lock);
    frame_spatial_cpu.create_type(in_width, in_height, in_channels, IM_DT_INT32);
    frame_temporal_cpu.create_type(in_width, in_height, in_channels, IM_DT_INT32);
    VkTransfer tran_spatial(vkdev);
    tran_spatial.record_upload(frame_spatial_cpu, frame_spatial, opt, false);
    tran_spatial.submit_and_wait();
    VkTransfer tran_temporal(vkdev);
    tran_temporal.record_upload(frame_temporal_cpu, frame_temporal, opt, false);
    tran_temporal.submit_and_wait();
}

void HQDN3D_vulkan::precalc_coefs(float dist25, int coef_index)
{
    MutexLockGuard lock(param_lock);
    double gamma, simil, C;
    gamma = log(0.25) / log(1.0 - MIN(dist25,252.0)/255.0 - 0.00001);
    int16_t *ct = (int16_t *)coef_cpu[coef_index].data;
    for (int i = -256*16; i < 256*16; i++) 
    {
        double f = ((i<<5) + 16 - 1) / 512.0; // midpoint of the bin
        simil = MAX(0, 1.0 - fabs(f) / 255.0);
        C = pow(simil, gamma) * 256.0 * f;
        ct[256*16+i] = lrint(C);
    }
    VkTransfer tran(vkdev);
    tran.record_upload(coef_cpu[coef_index], coefs[coef_index], opt, false);
    tran.submit_and_wait();
}

void HQDN3D_vulkan::SetParam(float lum_spac, float chrom_spac, float lum_tmp, float chrom_tmp)
{
    if (lum_spac != strength[LUMA_SPATIAL])
    {
        strength[LUMA_SPATIAL] = lum_spac;
        precalc_coefs(strength[LUMA_SPATIAL], LUMA_SPATIAL);
    }
    if (chrom_spac != strength[CHROMA_SPATIAL])
    {
        strength[CHROMA_SPATIAL] = chrom_spac;
        precalc_coefs(strength[CHROMA_SPATIAL], CHROMA_SPATIAL);
    }
    if (lum_tmp != strength[LUMA_TMP])
    {
        strength[LUMA_TMP] = lum_tmp;
        precalc_coefs(strength[LUMA_TMP], LUMA_TMP);
    }
    if (chrom_tmp != strength[CHROMA_TMP])
    {
        strength[CHROMA_TMP] = chrom_tmp;   
        precalc_coefs(strength[CHROMA_TMP], CHROMA_TMP);
    }
}

void HQDN3D_vulkan::upload_param(const VkMat& src, VkMat& dst)
{
    std::vector<VkMat> bindings(14);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (src.type == IM_DT_INT8)     bindings[4] = src;
    else if (src.type == IM_DT_INT16)    bindings[5] = src;
    else if (src.type == IM_DT_FLOAT16)  bindings[6] = src;
    else if (src.type == IM_DT_FLOAT32)  bindings[7] = src;
    MutexLockGuard lock(param_lock);
    bindings[8] = coefs[0];
    bindings[9] = coefs[1];
    bindings[10] = coefs[2];
    bindings[11] = coefs[3];
    bindings[12] = frame_spatial;
    bindings[13] = frame_temporal;
    std::vector<vk_constant_type> constants(10);
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
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void HQDN3D_vulkan::filter(const ImMat& src, ImMat& dst)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

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

void HQDN3D_vulkan::filter(const ImMat& src, VkMat& dst)
{
    if (!vkdev || !pipe  || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst);

    cmd->submit_and_wait();
    cmd->reset();
}

void HQDN3D_vulkan::filter(const VkMat& src, ImMat& dst)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void HQDN3D_vulkan::filter(const VkMat& src, VkMat& dst)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);

    upload_param(src, dst);

    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui