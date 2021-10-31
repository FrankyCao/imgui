#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <imvk_gpu.h>
#include <imvk_pipeline.h>

namespace ImGui 
{
class IMGUI_API DeInterlace_vulkan
{
public:
    DeInterlace_vulkan(int gpu = 0);
    ~DeInterlace_vulkan();
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
    ImGui::ImMat fCropTbl;
    ImGui::VkMat vfCropTbl;
    void upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst);
};
} // namespace ImGui 