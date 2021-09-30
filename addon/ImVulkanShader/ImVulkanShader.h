#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "imvk_command.h"
#include "imvk_substract_mean_normalize.h"
#include "filters/ColorConvert_vulkan.h"
#include "filters/Resize_vulkan.h"
#include <vulkan/vulkan.h>

namespace ImGui
{
#if IMGUI_RENDERING_VULKAN
    IMGUI_API ImTextureID ImVulkanImageToImTexture(const VkImageMat & image_vk);
#endif
    IMGUI_API void ImMatToImVulkanMat(const ImMat &src, VkMat &dst);
    IMGUI_API void ImVulkanVkMatToImMat(const VkMat &src, ImMat &dst);
    IMGUI_API void ImVulkanVkMatToVkImageMat(const VkMat &src, VkImageMat &dst);
    IMGUI_API void ImVulkanShaderClear();
    IMGUI_API float ImVulkanPeak(VulkanDevice* vkdev, int loop, int count_mb, int cmd_loop, int storage_type, int arithmetic_type, int packing_type);
// Demo Window
#if IMGUI_BUILD_EXAMPLE
    IMGUI_API void ImVulkanTestWindow(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
#endif
} //namespace ImGui