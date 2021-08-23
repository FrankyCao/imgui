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
#define CF_BGRA     3 \n\
#define CF_RGB      4 \n\
#define CF_ARGB     5 \n\
#define CF_RGBA     6 \n\
#define CF_YUV420   7 \n\
#define CF_YUV422   8 \n\
#define CF_YUV444   9 \n\
#define CF_YUVA     10 \n\
#define CF_NV12     11 \n\
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
" // line 42

#define SHADER_LOAD_SRC_RGB \
" \n\
sfpvec3 load_src_rgb(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = sfpvec3(0.f); \n\
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

#define SHADER_LOAD_FLOAT \
" \n\
sfp load_float(int x, int y, int w) \n\
{ \n\
    int i_offset = y * w + x; \n\
    return sfp(src_float_data[i_offset]); \n\
} \
"

#define SHADER_LOAD_FLOAT_RGB \
" \n\
sfpvec3 load_float_rgb(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = sfpvec3(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + ivec3(0, 1, 2); \n\
    rgb_in.r = sfp(src_float_data[i_offset.r]); \n\
    rgb_in.g = sfp(src_float_data[i_offset.g]); \n\
    rgb_in.b = sfp(src_float_data[i_offset.b]); \n\
    return rgb_in; \n\
} \
"

#define SHADER_LOAD_FLOAT_RGBA \
" \n\
sfpvec4 load_float_rgba(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(src_float_data[i_offset.r]); \n\
    rgb_in.g = sfp(src_float_data[i_offset.g]); \n\
    rgb_in.b = sfp(src_float_data[i_offset.b]); \n\
    rgb_in.a = sfp(src_float_data[i_offset.a]); \n\
    return rgb_in; \n\
} \
"

#define SHADER_LOAD_FLOAT_DST_RGBA \
" \n\
sfpvec4 load_float_dst_rgba(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(dst_float_data[i_offset.r]); \n\
    rgb_in.g = sfp(dst_float_data[i_offset.g]); \n\
    rgb_in.b = sfp(dst_float_data[i_offset.b]); \n\
    rgb_in.a = sfp(dst_float_data[i_offset.a]); \n\
    return rgb_in; \n\
} \
"

#define SHADER_STORE_FLOAT \
" \n\
void store_float(sfp val, int x, int y, int w) \n\
{ \n\
    int o_offset = y * w + x; \n\
    dst_float_data[o_offset] = float(val); \n\
} \
"

#define SHADER_STORE_FLOAT_RGB \
" \n\
void store_float_rgb(sfpvec3 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + ivec3(0, 1, 2); \n\
    dst_float_data[o_offset.r] = float(val.r); \n\
    dst_float_data[o_offset.g] = float(val.g); \n\
    dst_float_data[o_offset.b] = float(val.b); \n\
} \
"

#define SHADER_STORE_FLOAT_RGBA \
" \n\
void store_float_rgba(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + ivec4(0, 1, 2, 3); \n\
    dst_float_data[o_offset.r] = float(val.r); \n\
    dst_float_data[o_offset.g] = float(val.g); \n\
    dst_float_data[o_offset.b] = float(val.b); \n\
    dst_float_data[o_offset.a] = float(val.a); \n\
} \
"
