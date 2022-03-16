#pragma once
#include <imvk_mat_shader.h>
#define SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int in_format; \n\
    int in_type; \n\
    \n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
    int out_type; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    //ivec4 rgba = ivec4(load_rgba(gx, gy, p.w, p.cstep, p.in_format, p.in_type) * sfp(255.0)); \n\
    sfpvec4 rgba = load_rgba(gx, gy, p.w, p.cstep, p.in_format, p.in_type); \n\
    uint rid = uint(floor(rgba.r * sfp(p.out_w - 1))); \n\
    uint gid = uint(floor(rgba.g * sfp(p.out_w - 1))); \n\
    uint bid = uint(floor(rgba.b * sfp(p.out_w - 1))); \n\
    uint aid = uint(floor(rgba.a * sfp(p.out_w - 1))); \n\
    memoryBarrierBuffer(); \n\
    atomicAdd(histogram_int32_data[rid + p.out_w * 0], 1); \n\
    atomicAdd(histogram_int32_data[gid + p.out_w * 1], 1); \n\
    atomicAdd(histogram_int32_data[bid + p.out_w * 2], 1); \n\
    atomicAdd(histogram_int32_data[aid + p.out_w * 3], 1); \n\
    memoryBarrierBuffer(); \n\
} \
"

static const char Histogram_data[] = 
SHADER_HEADER
SHADER_PARAM
SHADER_SRC_DATA
R"(
layout (binding = 4) restrict buffer histogram_int32  { int histogram_int32_data[]; };
)"
SHADER_LOAD_RGBA
SHADER_MAIN
;

#define PARAM_CONV \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    float scale; \n\
    int log_view; \n\
} p;\
"

#define CONV_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    ivec4 data_offset = (gy * p.w + gx) * p.cstep + ivec4(0, 1, 2, 3); \n\
    vec4 rgba = vec4(0.f); \n\
    if (p.log_view == 1) \n\
    { \n\
        rgba.r = log2(float(histogram_int32_data[data_offset.x]) + 1) * p.scale; \n\
        rgba.g = log2(float(histogram_int32_data[data_offset.y]) + 1) * p.scale; \n\
        rgba.b = log2(float(histogram_int32_data[data_offset.z]) + 1) * p.scale; \n\
        rgba.a = log2(float(histogram_int32_data[data_offset.w]) + 1) * p.scale; \n\
    } \n\
    else \n\
    { \n\
        rgba.r = float(histogram_int32_data[data_offset.x]) * p.scale; \n\
        rgba.g = float(histogram_int32_data[data_offset.y]) * p.scale; \n\
        rgba.b = float(histogram_int32_data[data_offset.z]) * p.scale; \n\
        rgba.a = float(histogram_int32_data[data_offset.w]) * p.scale; \n\
    } \n\
    histogram_float32_data[data_offset.x] = rgba.r; \n\
    histogram_float32_data[data_offset.y] = rgba.g; \n\
    histogram_float32_data[data_offset.z] = rgba.b; \n\
    histogram_float32_data[data_offset.w] = rgba.a; \n\
} \
"

static const char ConvInt2Float_data[] = 
SHADER_HEADER
PARAM_CONV
R"(
layout (binding = 0) readonly buffer histogram_int32  { int histogram_int32_data[]; };
layout (binding = 1) writeonly buffer histogram_float32  { float histogram_float32_data[]; };
)"
CONV_MAIN
;