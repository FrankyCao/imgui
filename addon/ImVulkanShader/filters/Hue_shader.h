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
    float hue; \n\
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

#define SHADER_HUE \
" \n\
const sfpvec3  kRGBToYPrime = sfpvec3(sfp(0.299f), sfp(0.587f), sfp(0.114f)); \n\
const sfpvec3  kRGBToI      = sfpvec3(sfp(0.595716f), sfp(-0.274453f), sfp(-0.321263f)); \n\
const sfpvec3  kRGBToQ      = sfpvec3(sfp(0.211456f), sfp(-0.522591f), sfp(0.31135f)); \n\
const sfpvec3  kYIQToR      = sfpvec3(sfp(1.0f), sfp(0.9563f), sfp(0.6210f)); \n\
const sfpvec3  kYIQToG      = sfpvec3(sfp(1.0f), sfp(-0.2721f), sfp(-0.6474f)); \n\
const sfpvec3  kYIQToB      = sfpvec3(sfp(1.0f), sfp(-1.1070f), sfp(1.7046f)); \n\
sfpvec3 hue(sfpvec3 color) \n\
{ \n\
    sfp   YPrime    = dot(color, kRGBToYPrime); \n\
    sfp   I         = dot(color, kRGBToI); \n\
    sfp   Q         = dot(color, kRGBToQ); \n\
    // Calculate the hue and chroma \n\
    sfp   hue       = atan (Q, I); \n\
    sfp   chroma    = sqrt (I * I + Q * Q); \n\
    // Make the user's adjustments \n\
    hue += (-p.hue); //why negative rotation? \n\
    // Convert back to YIQ \n\
    Q = chroma * sin (hue); \n\
    I = chroma * cos (hue); \n\
    // Convert back to RGB \n\
    sfpvec3    yIQ   = sfpvec3 (YPrime, I, Q); \n\
    color.r = dot(yIQ, kYIQToR); \n\
    color.g = dot(yIQ, kYIQToG); \n\
    color.b = dot(yIQ, kYIQToB); \n\
    return color; \n\
} \
"

#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    sfpvec3 color = load_src_rgb(gx, gy); \n\
    sfpvec3 result = hue(color); \n\
    store_dst_rgb(gx, gy, result); \n\
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
SHADER_HUE
SHADER_MAIN
;
