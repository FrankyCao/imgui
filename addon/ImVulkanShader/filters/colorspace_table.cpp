#include <immat.h>

namespace ImGui 
{
// color space matrix
const float r2y_601_full[3][3] = {
    { 0.299000f,  0.587000f,  0.114000f},
    {-0.168736f, -0.331264f,  0.500000f},
    { 0.500000f, -0.418688f, -0.081312f}
};

const float r2y_601_narrow[3][3] = {
    { 0.256788f,  0.515639f,  0.100141f},
    {-0.144914f, -0.290993f,  0.439216f},
    { 0.429412f, -0.367788f, -0.071427f}
};

const float r2y_709_full[3][3] = {
    { 0.212600f,  0.715200f,  0.072200f},
    {-0.114572f, -0.385428f,  0.500000f},
    { 0.500000f, -0.454153f, -0.045847f}
};

const float r2y_709_narrow[3][3] = {
    { 0.182586f,  0.628254f,  0.063423f},
    {-0.098397f, -0.338572f,  0.439216f},
    { 0.429412f, -0.398942f, -0.040274f}
};

const float r2y_2020_full[3][3] = {
    { 0.262700f,  0.678000f,  0.059300f},
    {-0.139630f, -0.360370f,  0.500000f},
    { 0.500000f, -0.459786f, -0.040214f}
};

const float r2y_2020_narrow[3][3] = {
    { 0.225613f,  0.595576f,  0.052091f},
    {-0.119918f, -0.316560f,  0.439216f},
    { 0.429412f, -0.403890f, -0.035325f}
};

const float y2r_601_full[3][3] = {
    {1.000000f,  0.000000f,  1.402000f},
    {1.000000f, -0.344136f, -0.714136f},
    {1.000000f,  1.772000f,  0.000000f}
};

const float y2r_601_narrow[3][3] = {
    {1.164384f,  0.000000f,  1.596027f},
    {1.164384f, -0.391762f, -0.812968f},
    {1.164384f,  2.017232f,  0.000000f}
};

const float y2r_709_full[3][3] = {
    {1.000000f,  0.000000f,  1.574800f},
    {1.000000f, -0.187324f, -0.468124f},
    {1.000000f,  1.855600f,  0.000000f}
};

const float y2r_709_narrow[3][3] = {
    {1.164384f,  0.000000f,  1.792741f},
    {1.164384f, -0.213249f, -0.532909f},
    {1.164384f,  2.112402f,  0.000000f}
};

const float y2r_2020_full[3][3] = {
    {1.000000f,  0.000000f,  1.474600f},
    {1.000000f, -0.164553f, -0.571353f},
    {1.000000f,  1.881400f,  0.000000f}
};

const float y2r_2020_narrow[3][3] = {
    {1.164384f,  0.000000f,  1.678674f},
    {1.164384f, -0.187326f, -0.650424f},
    {1.164384f,  2.141772f,  0.000000f}
};

const float srgb[3][3] = {
    {1.000000f, 1.000000f, 1.000000f},
    {1.000000f, 1.000000f, 1.000000f},
    {1.000000f, 1.000000f, 1.000000f}
};

const ImMat matrix_yr_601_full   (3, 3, (void *)y2r_601_full,   sizeof(float));
const ImMat matrix_yr_601_narrow (3, 3, (void *)y2r_601_narrow, sizeof(float));
const ImMat matrix_yr_709_full   (3, 3, (void *)y2r_709_full,   sizeof(float));
const ImMat matrix_yr_709_narrow (3, 3, (void *)y2r_709_narrow, sizeof(float));
const ImMat matrix_yr_2020_full  (3, 3, (void *)y2r_2020_full,  sizeof(float));
const ImMat matrix_yr_2020_narrow(3, 3, (void *)y2r_2020_full,  sizeof(float));

const ImMat matrix_ry_601_full   (3, 3, (void *)r2y_601_full,   sizeof(float));
const ImMat matrix_ry_601_narrow (3, 3, (void *)r2y_601_narrow, sizeof(float));
const ImMat matrix_ry_709_full   (3, 3, (void *)r2y_709_full,   sizeof(float));
const ImMat matrix_ry_709_narrow (3, 3, (void *)r2y_709_narrow, sizeof(float));
const ImMat matrix_ry_2020_full  (3, 3, (void *)r2y_2020_full,  sizeof(float));
const ImMat matrix_ry_2020_narrow(3, 3, (void *)r2y_2020_full,  sizeof(float));

const ImMat matrix_srgb(3, 3, (void *)srgb,  sizeof(float));

const ImMat * color_table[2][2][4] = {
    {
        { &matrix_srgb, &matrix_yr_601_full,   &matrix_yr_709_full,   &matrix_yr_2020_full},
        { &matrix_srgb, &matrix_yr_601_narrow, &matrix_yr_709_narrow, &matrix_yr_2020_narrow}
    },
    {
        { &matrix_srgb, &matrix_ry_601_full,   &matrix_ry_709_full,   &matrix_ry_2020_full},
        { &matrix_srgb, &matrix_ry_601_narrow, &matrix_ry_709_narrow, &matrix_ry_2020_narrow}
    }
};
} // namespace ImGui 

