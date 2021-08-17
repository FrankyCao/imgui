#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <string>
#include <Filter2D_vulkan.h>

namespace ImGui
{
class IMGUI_API  Laplacian_vulkan : public Filter2D_vulkan
{
public:
    Laplacian_vulkan(int gpu = 0);
    ~Laplacian_vulkan();
    void SetParam(int _Strength);

private:
    int Strength {2};
    void prepare_kernel();
};
} // namespace ImGui
