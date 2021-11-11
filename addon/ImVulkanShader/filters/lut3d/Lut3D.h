#pragma once
#include "imvk_gpu.h"
#include "imvk_pipeline.h"
#include "im_mat.h"

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
class VKSHADER_API LUT3D_vulkan
{
public:
    LUT3D_vulkan(int default_model = SDR709_HDRHLG, int interpolation = IM_INTERPOLATE_TRILINEAR, int gpu = 0);
    LUT3D_vulkan(std::string lut_path, int interpolation = IM_INTERPOLATE_TRILINEAR, int gpu = 0);
    ~LUT3D_vulkan();

    void filter(const ImMat& src, ImMat& dst);
    void filter(const ImMat& src, VkMat& dst);
    void filter(const VkMat& src, ImMat& dst);
    void filter(const VkMat& src, VkMat& dst);

    void write_header_file(std::string filename);
    
public:
    const VulkanDevice* vkdev;
    Pipeline * pipeline_lut3d = nullptr;
    VkCompute * cmd = nullptr;
    Option opt;
    VkMat lut_gpu;

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
    void upload_param(const VkMat& src, VkMat& dst);
};
} // namespace ImGui 