#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <string>
#include <Filter2DS_vulkan.h>

class BoxBlur : public ImGui::Filter2DS_vulkan
{
public:
    BoxBlur(int gpu = 0);
    ~BoxBlur();
    void SetParam(int _xSize, int _ySize);

private:
    int xSize {3};
    int ySize {3};
    void prepare_kernel();
};
