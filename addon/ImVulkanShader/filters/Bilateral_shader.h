#pragma once
#define SHADER_HEADER \
"\
#version 450 \n\
#extension GL_EXT_shader_8bit_storage: require \n\
#extension GL_EXT_shader_16bit_storage: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require \n\
#define GRAY    0 \n\
#define BGR     1 \n\
#define ABGR    2 \n\
#define RGB     3 \n\
#define ARGB    4 \n\
#define YUV420  5 \n\
#define YUV422  6 \n\
#define YUV444  7 \n\
#define YUVA    8 \n\
#define NV12    9 \n\
\
"

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

#define SHADER_LOAD_SRC_RGB \
" \n\
sfpvec3 load_src_rgb(int x, int y) \n\
{ \n\
    sfpvec3 rgb_in = {0.f, 0.f, 0.f}; \n\
    ivec4 i_offset = (y * p.w + x) * p.cstep + (p.format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(src_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(src_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(src_int8_data[i_offset.b])) / sfp(255.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_STORE_DST_RGB \
" \n\
void store_dst_rgb(int x, int y, sfpvec3 rgb) \n\
{ \n\
    ivec4 o_offset = (y * p.w + x) * p.cstep + (p.format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    dst_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.a] = uint8_t(255); \n\
} \
"

#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    sfpvec3 center = load_src_rgb(uv.x, uv.y); \n\
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
                sfpvec3 color = load_src_rgb(bx, by); \n\
                sfp norm = dot(abs(color - center), sfpvec3(1.0f)); \n\
                sfp weight = exp(space2 * p.sigma_spatial2_inv_half + norm * norm * p.sigma_color2_inv_half); \n\
                sum1 = sum1 + weight * color; \n\
                sum2 = sum2 + weight; \n\
            } \n\
        } \n\
    } \n\
    store_dst_rgb(uv.x, uv.y, sum1/sum2); \n\
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
