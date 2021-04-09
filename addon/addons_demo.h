#ifndef __ADDONS_DEMO_H
#define __ADDONS_DEMO_H
#include "imgui.h"
namespace ImGui
{
    IMGUI_API void          ShowAddonsDemoWindowWidgets();
    IMGUI_API void          ShowAddonsDuckWindow();
    IMGUI_API void          ShowAddonsTabWindow();
    IMGUI_API void          ShowAddonsZMOWindow();
    IMGUI_API void          CleanupZMODemo();
    IMGUI_API void          CleanupDemo();
#if IMGUI_VULKAN_SHADER
    IMGUI_API void          PrepareVulkanDemo();
    IMGUI_API void          ShowAddonsVulkanShaderWindow();
    IMGUI_API void          CleanVulkanDemo();
#endif
}
#endif /* __ADDONS_DEMO_H */