#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <string>
#include <Filter2DS_vulkan.h>

namespace ImGui
{
class IMGUI_API BoxBlur_vulkan : public Filter2DS_vulkan
{
public:
    BoxBlur_vulkan(int gpu = 0);
    ~BoxBlur_vulkan();
    void SetParam(int _xSize, int _ySize);

private:
    int xSize {3};
    int ySize {3};
    void prepare_kernel();
};
} // namespace ImGui
