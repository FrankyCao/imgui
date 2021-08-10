#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <string>
#include <Filter2DS_vulkan.h>

class GaussianBlur : public ImGui::Filter2DS_vulkan
{
public:
    GaussianBlur(int gpu = 0);
    ~GaussianBlur();
    void SetParam(int _blurRadius, float _sigma);

private:
    int blurRadius {3};
    float sigma {0.0f};
    void prepare_kernel();
};
