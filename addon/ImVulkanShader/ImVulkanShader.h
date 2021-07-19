#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#include "gpu.h"
#include "pipeline.h"
#include "command.h"
#include "ColorConvert_vulkan.h"
#include "Resize_vulkan.h"
#include <vulkan/vulkan.h>

namespace ImGui
{
    IMGUI_API ImTextureID ImVulkanImageToImTexture(const VkImageMat & image_vk);
} //namespace ImGui