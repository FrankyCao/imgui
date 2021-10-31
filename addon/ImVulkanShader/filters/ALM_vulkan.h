#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <imvk_gpu.h>
#include <imvk_pipeline.h>

namespace ImGui 
{
class IMGUI_API ALM_vulkan
{
public:
    ALM_vulkan(int gpu = 0);
    ~ALM_vulkan();
    void SetParam(float _strength, float _bias = 0.7, float _gamma = 2.2);
    void filter(const ImGui::ImMat& src, ImGui::ImMat& dst);
    void filter(const ImGui::ImMat& src, ImGui::VkMat& dst);
    void filter(const ImGui::VkMat& src, ImGui::ImMat& dst);
    void filter(const ImGui::VkMat& src, ImGui::VkMat& dst);

private:
    ImGui::VulkanDevice* vkdev {nullptr};
    ImGui::Option opt;
    ImGui::Pipeline* pipe {nullptr};
    ImGui::VkCompute * cmd {nullptr};

private:
    int rgba_map[4];
    float strength {0.5};
    float bias {0.7};
    float gamma {2.2};
    void upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst);
};
} // namespace ImGui 