#pragma once
#include <imgui.h>
#include <imgui_mat.h>
#include <imvk_gpu.h>
#include <imvk_pipeline.h>
typedef struct _tag_rgbvec 
{
    float r, g, b, a;
} rgbvec;

enum default_lut : int {
    SDR709_HDRHLG = 0,
    SDR709_HDRPQ,
    HDRHLG_SDR709,
    HDRPQ_SDR709,
    HDRHLG_HDRPQ,
    HDRPQ_HDRHLG,
    NO_DEFAULT,
};

namespace ImGui 
{
class IMGUI_API LUT3D_vulkan
{
public:
    LUT3D_vulkan(int default_model = SDR709_HDRHLG, int interpolation = IM_INTERPOLATE_TRILINEAR, int gpu = 0);
    LUT3D_vulkan(std::string lut_path, int interpolation = IM_INTERPOLATE_TRILINEAR, int gpu = 0);
    ~LUT3D_vulkan();

    void filter(const ImGui::ImMat& src, ImGui::ImMat& dst);
    void filter(const ImGui::ImMat& src, ImGui::VkMat& dst);
    void filter(const ImGui::VkMat& src, ImGui::ImMat& dst);
    void filter(const ImGui::VkMat& src, ImGui::VkMat& dst);

    void write_header_file(std::string filename);
    
public:
    const ImGui::VulkanDevice* vkdev;
    ImGui::Pipeline * pipeline_lut3d = nullptr;
    ImGui::VkCompute * cmd = nullptr;
    ImGui::Option opt;
    ImGui::VkMat lut_gpu;

private:
    void *lut {nullptr};
    int lutsize {0};
    rgbvec scale {1.0, 1.0, 1.0, 0.0};
    int rgba_map[4];
    int interpolation_mode {IM_INTERPOLATE_TRILINEAR};
    bool from_file {false};

private:
    int init(int interpolation, int gpu);
    int allocate_3dlut(int size);
    int parse_cube(std::string lut_file);
    void upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst);
};
} // namespace ImGui 