#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <imvk_gpu.h>
#include <imvk_pipeline.h>

namespace ImGui 
{
class IMGUI_API CAS_vulkan
{
public:
    CAS_vulkan(int gpu = 0);
    ~CAS_vulkan();
    void filter(const ImGui::ImMat& src, ImGui::ImMat& dst, float strength);
    void filter(const ImGui::ImMat& src, ImGui::VkMat& dst, float strength);
    void filter(const ImGui::VkMat& src, ImGui::ImMat& dst, float strength);
    void filter(const ImGui::VkMat& src, ImGui::VkMat& dst, float strength);

private:
    ImGui::VulkanDevice* vkdev {nullptr};
    ImGui::Option opt;
    ImGui::Pipeline* pipe {nullptr};
    ImGui::VkCompute * cmd {nullptr};

private:
    void upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst, float strength);
};
} // namespace ImGui