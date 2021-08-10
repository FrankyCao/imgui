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
    int format; \n\
\n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
\n\
    int x_offset; \n\
    int y_offset; \n\
    float alpha; \n\
} p; \
"

#define SHADER_LOAD_SRC_RGBA \
" \n\
sfpvec4 load_src_rgba(int x, int y) \n\
{ \n\
    sfpvec4 rgb_in = {0.f, 0.f, 0.f, 0.f}; \n\
    ivec4 i_offset = (y * p.w + x) * p.cstep + (p.format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(src_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(src_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(src_int8_data[i_offset.b])) / sfp(255.f); \n\
    rgb_in.a = sfp(uint(src_int8_data[i_offset.a])) / sfp(255.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_LOAD_DST_RGBA \
" \n\
sfpvec4 load_dst_rgba(int x, int y) \n\
{ \n\
    sfpvec4 rgb_in = {0.f, 0.f, 0.f, 0.f}; \n\
    ivec4 i_offset = (y * p.out_w + x) * p.out_cstep + (p.out_format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(dst_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(dst_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(dst_int8_data[i_offset.b])) / sfp(255.f); \n\
    rgb_in.a = sfp(uint(dst_int8_data[i_offset.a])) / sfp(255.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_STORE_DST_RGBA \
" \n\
void store_dst_rgba(int x, int y, sfpvec4 rgb) \n\
{ \n\
    ivec4 o_offset = (y * p.out_w + x) * p.out_cstep + (p.out_format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    dst_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.a] = uint8_t(clamp(uint(floor(rgb.a * sfp(255.f))), 0, 255)); \n\
} \
"

#define SHADER_ALPHA_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    if (uv.x + p.x_offset >= p.out_w || uv.y + p.y_offset >= p.out_h) \n\
        return; \n\
    sfpvec4 rgba = load_src_rgba(uv.x, uv.y); \n\
    sfpvec4 rgba_dst = load_dst_rgba(uv.x + p.x_offset, uv.y + p.y_offset); \n\
    sfpvec4 result = sfpvec4(mix(rgba_dst.rgb, rgba.rgb, rgba.a * sfp(p.alpha)), rgba_dst.a); \n\
    store_dst_rgba(uv.x + p.x_offset, uv.y + p.y_offset, result); \n\
} \
"

static const char CopyTo_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_int8 { uint8_t src_int8_data[]; };
layout (binding = 1) buffer dst_int8 { uint8_t dst_int8_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_SRC_RGBA
SHADER_LOAD_DST_RGBA
SHADER_STORE_DST_RGBA
SHADER_ALPHA_MAIN
;
