#pragma once
#include <imgui.h>
#include <imvk_gpu.h>
#include <imvk_pipeline.h>

namespace ImGui
{
class IMGUI_API HQDN3D_vulkan
{
public:
    HQDN3D_vulkan(int width, int height, int channels, int gpu = 0);
    ~HQDN3D_vulkan();
    void SetParam(float lum_spac, float chrom_spac, float lum_tmp, float chrom_tmp);
    void filter(const ImGui::ImMat& src, ImGui::ImMat& dst);
    void filter(const ImGui::ImMat& src, ImGui::VkMat& dst);
    void filter(const ImGui::VkMat& src, ImGui::ImMat& dst);
    void filter(const ImGui::VkMat& src, ImGui::VkMat& dst);
private:
    ImGui::VulkanDevice* vkdev {nullptr};
    ImGui::Option opt;
    ImGui::Pipeline* pipe {nullptr};
    ImGui::VkCompute * cmd {nullptr};

public:
    int in_width {0};
    int in_height {0};
    int in_channels {0};

private:
    float strength[4] {0, 0, 0, 0};
    ImGui::ImMat coef_cpu[4];
    ImGui::ImMat frame_spatial_cpu;
    ImGui::ImMat frame_temporal_cpu;
    mutable ImGui::Mutex param_lock;
    ImGui::VkMat coefs[4];
    ImGui::VkMat frame_spatial;
    ImGui::VkMat frame_temporal;
private:
    void precalc_coefs(float dist25, int coef_index);
    void prealloc_frames(void);
    void upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst);
};
} // namespace ImGui