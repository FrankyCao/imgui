#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <string>
#include <Filter2D_vulkan.h>

namespace ImGui
{
class IMGUI_API  Laplacian : public Filter2D_vulkan
{
public:
    Laplacian(int gpu = 0);
    ~Laplacian();
    void SetParam(int _Strength);

private:
    int Strength {2};
    void prepare_kernel();
};
} // namespace ImGui
