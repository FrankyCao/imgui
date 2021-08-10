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
    int ksz; \n\
    float sigma_spatial2_inv_half; \n\
    float sigma_color2_inv_half; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    sfpvec3 center = load_src_rgb(uv.x, uv.y, p.w, p.cstep, p.format); \n\
    sfpvec3 sum1 = sfpvec3(0.0f); \n\
    sfp sum2 = sfp(0.0f); \n\
    int r = p.ksz / 2; \n\
    sfp r2 = sfp(r * r); \n\
    // 最长半径 \n\
    int tx = uv.x - r + p.ksz; \n\
    int ty = uv.y - r + p.ksz; \n\
    // 半径为r的圆里数据根据比重混合 \n\
    for (int cy = uv.y - r; cy < ty; ++cy) \n\
    { \n\
        for (int cx = uv.x - r; cx < tx; ++cx) \n\
        { \n\
            sfp space2 = (uv.x - cx) * (uv.x - cx) + (uv.y - cy) * (uv.y - cy); \n\
            if (space2 < r2) \n\
            { \n\
                int bx = max(0, min(cx, p.w - 1)); \n\
                int by = max(0, min(cy, p.h - 1)); \n\
                sfpvec3 color = load_src_rgb(bx, by, p.w, p.cstep, p.format); \n\
                sfp norm = dot(abs(color - center), sfpvec3(1.0f)); \n\
                sfp weight = exp(space2 * p.sigma_spatial2_inv_half + norm * norm * p.sigma_color2_inv_half); \n\
                sum1 = sum1 + weight * color; \n\
                sum2 = sum2 + weight; \n\
            } \n\
        } \n\
    } \n\
    store_dst_rgb(sum1/sum2, uv.x, uv.y, p.w, p.cstep, p.format); \n\
} \
"

static const char Filter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_int8 { uint8_t src_int8_data[]; };
layout (binding = 1) writeonly buffer dst_int8 { uint8_t dst_int8_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_SRC_RGB
SHADER_STORE_DST_RGB
SHADER_MAIN
;
