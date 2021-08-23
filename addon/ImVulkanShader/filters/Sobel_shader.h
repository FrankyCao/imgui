#pragma once
#include <vk_mat_shader.h>

#define SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
\n\
    int format; \n\
\n\
    float strength; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
sfpmat3 matrix_mat_r2y = { \n\
    { sfp( 0.262700f), sfp(-0.139630f), sfp( 0.500000f)}, \n\
    { sfp( 0.678000f), sfp(-0.360370f), sfp(-0.459786f)}, \n\
    { sfp( 0.059300f), sfp( 0.500000f), sfp(-0.040214f)} \n\
}; \n\
sfpvec3 rgb_to_yuv(sfpvec3 rgb) \n\
{ \n\
    sfpvec3 yuv_offset = {sfp(0.f), sfp(0.5f), sfp(0.5f)}; \n\
    sfpvec3 yuv = yuv_offset + matrix_mat_r2y * rgb; \n\
    return yuv; \n\
} \n\
const float horizontKernel[9] = \n\
{ \n\
    -1.0f,-2.0f,-1.0f, \n\
     0.0f, 0.0f, 0.0f, \n\
     1.0f, 2.0f, 1.0f  \n\
}; \n\
const float verticalKernel[9] = \n\
{ \n\
    -1.0f, 0.0f, 1.0f, \n\
    -2.0f, 0.0f, 2.0f, \n\
    -1.0f, 0.0f, 1.0f  \n\
}; \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    sfp vertical = 0.0f; \n\
    sfp horizont = 0.0f; \n\
    for (int i = 0; i < 3; ++i) \n\
    { \n\
        for (int j = 0; j < 3; ++j) \n\
        { \n\
            int x = gx - 1 + j; \n\
            int y = gy - 1 + i; \n\
            // REPLICATE border \n\
            x = max(0, min(x, p.w - 1)); \n\
            y = max(0, min(y, p.h - 1)); \n\
            int index = j + i * 3; \n\
            sfpvec3 value = rgb_to_yuv(load_float_rgba(x, y, p.w, p.cstep, p.format).rgb); \n\
            vertical += value.x * sfp(verticalKernel[index]); \n\
            horizont += value.x * sfp(horizontKernel[index]); \n\
        } \n\
    } \n\
    sfp mag = length(sfpvec2(horizont, vertical)) * sfp(p.strength); \n\
    store_float_rgba(sfpvec4(mag, mag, mag, 1.0f), gx, gy, p.w, p.cstep, p.format); \n\
} \
"

static const char Filter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_FLOAT_RGBA
SHADER_STORE_FLOAT_RGBA
SHADER_MAIN
;
