#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <string>
#include <Filter2D_vulkan.h>

class Laplacian : public ImGui::Filter2D_vulkan
{
public:
    Laplacian(int gpu = 0);
    ~Laplacian();
    void SetParam(int _Strength);

private:
    int Strength {2};
    void prepare_kernel();
};
