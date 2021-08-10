#pragma once
#include <imgui.h>
#include "gpu.h"
#include "pipeline.h"
#include <imgui_mat.h>
#include <string>

class Sobel
{
public:
    Sobel(int gpu = 0);
    ~Sobel();
    void SetParam(float _edgeStrength);
    void filter(const ImGui::ImMat& src, ImGui::ImMat& dst, float edgeStrength);
    void filter(const ImGui::ImMat& src, ImGui::VkMat& dst, float edgeStrength);
    void filter(const ImGui::VkMat& src, ImGui::ImMat& dst, float edgeStrength);
    void filter(const ImGui::VkMat& src, ImGui::VkMat& dst, float edgeStrength);
private:
    ImGui::VulkanDevice* vkdev  {nullptr};
    ImGui::Option opt;
    ImGui::Pipeline* pipe       {nullptr};
    ImGui::VkCompute * cmd      {nullptr};
private:
    std::vector<uint32_t> spirv_data;
private:
    void upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst, float edgeStrength);
};
