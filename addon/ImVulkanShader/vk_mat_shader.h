#pragma once
#define SHADER_HEADER \
"\
#version 450 \n\
#extension GL_EXT_shader_8bit_storage: require \n\
#extension GL_EXT_shader_16bit_storage: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types_int8: require \n\
#extension GL_EXT_shader_atomic_float: require \n\
#define CF_GRAY     0 \n\
#define CF_BGR      1 \n\
#define CF_ABGR     2 \n\
#define CF_RGB      3 \n\
#define CF_ARGB     4 \n\
#define CF_YUV420   5 \n\
#define CF_YUV422   6 \n\
#define CF_YUV444   7 \n\
#define CF_YUVA     8 \n\
#define CF_NV12     9 \n\
#define INTERPOLATE_NEAREST     0 \n\
#define INTERPOLATE_BILINEAR    1 \n\
#define INTERPOLATE_BICUBIC     2 \n\
#define INTERPOLATE_AREA        3 \n\
#define INTERPOLATE_TRILINEAR   4 \n\
#define INTERPOLATE_TETRAHEDRAL 5 \n\
#define CS_SRGB     0 \n\
#define CS_BT601    1 \n\
#define CS_BT709    2 \n\
#define CS_BT2020   3 \n\
#define CS_HSV      4 \n\
#define CS_HLS      5 \n\
#define CS_CMY      6 \n\
#define CS_LAB      7 \n\
#define CR_FULL_RANGE   0 \n\
#define CR_NARROW_RANGE 1 \n\
#define DT_INT8         0 \n\
#define DT_INT16        1 \n\
#define DT_INT32        2 \n\
#define DT_INT64        3 \n\
#define DT_FLOAT16      4 \n\
#define DT_FLOAT32      5 \n\
#define DT_FLOAT64      6 \n\
\
"

#define SHADER_LOAD_SRC_RGB \
" \n\
sfpvec3 load_src_rgb(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = {0.f, 0.f, 0.f}; \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(src_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(src_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(src_int8_data[i_offset.b])) / sfp(255.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_LOAD_DST_RGB \
" \n\
sfpvec3 load_dst_rgb(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = {0.f, 0.f, 0.f}; \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(dst_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(dst_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(dst_int8_data[i_offset.b])) / sfp(255.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_STORE_SRC_RGB \
" \n\
void store_src_rgb(sfpvec3 rgb, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    src_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.f))), 0, 255)); \n\
    src_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.f))), 0, 255)); \n\
    src_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.f))), 0, 255)); \n\
    src_int8_data[o_offset.a] = uint8_t(255); \n\
} \
"

#define SHADER_STORE_DST_RGB \
" \n\
void store_dst_rgb(sfpvec3 rgb, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    dst_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.a] = uint8_t(255); \n\
} \
"

#define SHADER_LOAD_SRC_RGBA \
" \n\
sfpvec4 load_src_rgba(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(src_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(src_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(src_int8_data[i_offset.b])) / sfp(255.f); \n\
    rgb_in.a = sfp(uint(src_int8_data[i_offset.a])) / sfp(255.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_LOAD_DST_RGBA \
" \n\
sfpvec4 load_dst_rgba(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(dst_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(dst_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(dst_int8_data[i_offset.b])) / sfp(255.f); \n\
    rgb_in.a = sfp(uint(dst_int8_data[i_offset.a])) / sfp(255.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_STORE_SRC_RGBA \
" \n\
void store_src_rgba(sfpvec4 rgb, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    src_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.f))), 0, 255)); \n\
    src_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.f))), 0, 255)); \n\
    src_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.f))), 0, 255)); \n\
    src_int8_data[o_offset.a] = uint8_t(clamp(uint(floor(rgb.a * sfp(255.f))), 0, 255)); \n\
} \
"

#define SHADER_STORE_DST_RGBA \
" \n\
void store_dst_rgba(sfpvec4 rgb, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    dst_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.a] = uint8_t(clamp(uint(floor(rgb.a * sfp(255.f))), 0, 255)); \n\
} \
"