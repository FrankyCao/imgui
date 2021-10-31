#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <imvk_gpu.h>
#include <imvk_pipeline.h>

namespace ImGui 
{
class IMGUI_API DeBand_vulkan
{
public:
    DeBand_vulkan(int width, int height, int channels, int gpu = 0);
    ~DeBand_vulkan();
    void SetParam(int _range, float _direction);
    void filter(const ImGui::ImMat& src, ImGui::ImMat& dst, float threshold, bool blur);
    void filter(const ImGui::ImMat& src, ImGui::VkMat& dst, float threshold, bool blur);
    void filter(const ImGui::VkMat& src, ImGui::ImMat& dst, float threshold, bool blur);
    void filter(const ImGui::VkMat& src, ImGui::VkMat& dst, float threshold, bool blur);
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
    int range {16};
    float direction {2*M_PI};
    ImGui::ImMat xpos_cpu;
    ImGui::ImMat ypos_cpu;
    mutable ImGui::Mutex param_lock;
    ImGui::VkMat xpos;
    ImGui::VkMat ypos;
private:
    void precalc_pos(void);
    void upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst, float threshold, bool blur);
};
} // namespace ImGui 