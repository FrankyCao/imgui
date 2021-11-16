#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "imvk_command.h"
#include "immat.h"
#include "imvk_substract_mean_normalize.h"
#include "filters/ColorConvert_vulkan.h"
#include "filters/Resize_vulkan.h"
#include <vulkan/vulkan.h>

namespace ImGui
{
    VKSHADER_API void ImMatToImVulkanMat(const ImMat &src, VkMat &dst);
    VKSHADER_API void ImVulkanVkMatToImMat(const VkMat &src, ImMat &dst);
    VKSHADER_API void ImVulkanVkMatToVkImageMat(const VkMat &src, VkImageMat &dst);
    VKSHADER_API void ImVulkanShaderInit();
    VKSHADER_API void ImVulkanShaderClear();
    VKSHADER_API float ImVulkanPeak(VulkanDevice* vkdev, int loop, int count_mb, int cmd_loop, int storage_type, int arithmetic_type, int packing_type);
} //namespace ImGui