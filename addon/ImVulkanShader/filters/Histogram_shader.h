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

/*
#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    sfpvec4 rgba = load_rgba(gx, gy, p.w, p.cstep, p.in_format, p.in_type); \n\
    uint rid = uint(floor(rgba.r * sfp(p.out_w - 1))) + 0 * p.out_cstep; \n\
    uint gid = uint(floor(rgba.g * sfp(p.out_w - 1))) + 1 * p.out_cstep; \n\
    uint bid = uint(floor(rgba.b * sfp(p.out_w - 1))) + 2 * p.out_cstep; \n\
    //uint aid = uint(floor(rgba.a * sfp(p.out_w - 1))) + 3 * p.out_cstep; \n\
    memoryBarrierBuffer(); \n\
    atomicAdd(histogram_int32_data[rid], 1); \n\
    atomicAdd(histogram_int32_data[gid], 1); \n\
    atomicAdd(histogram_int32_data[bid], 1); \n\
    //atomicAdd(histogram_int32_data[aid], 1); \n\
    memoryBarrierBuffer(); \n\
} \
"
*/

#define SHADER_MAIN \
" \n\
shared int data_sharedR[256]; \n\
shared int data_sharedG[256]; \n\
shared int data_sharedB[256]; \n\
shared int data_sharedA[256]; \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    int tid = int(gl_LocalInvocationIndex); \n\
    data_sharedR[tid] = 0; \n\
    data_sharedG[tid] = 0; \n\
    data_sharedB[tid] = 0; \n\
    data_sharedA[tid] = 0; \n\
    if(uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    memoryBarrierShared(); \n\
    barrier(); \n\
    sfpvec4 rgba = load_rgba(uv.x, uv.y, p.w, p.cstep, p.in_format, p.in_type); \n\
    ivec4 irgba = ivec4(rgba * sfp(255.0f)); \n\
    atomicAdd(data_sharedR[irgba.r], 1); \n\
    atomicAdd(data_sharedG[irgba.g], 1); \n\
    atomicAdd(data_sharedB[irgba.b], 1); \n\
    atomicAdd(data_sharedA[irgba.a], 1); \n\
    memoryBarrierShared(); \n\
    barrier(); \n\
    uint rid = tid + 0 * p.out_cstep; \n\
    uint gid = tid + 1 * p.out_cstep; \n\
    uint bid = tid + 2 * p.out_cstep; \n\
    uint aid = tid + 3 * p.out_cstep; \n\
    atomicAdd(histogram_int32_data[rid], data_sharedR[tid]); \n\
    atomicAdd(histogram_int32_data[gid], data_sharedG[tid]); \n\
    atomicAdd(histogram_int32_data[bid], data_sharedB[tid]); \n\
    atomicAdd(histogram_int32_data[aid], data_sharedA[tid]); \n\
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
    ivec4 data_offset = (gy * p.w + gx) + ivec4(0, 1, 2, 3) * p.cstep; \n\
    ivec4 data_nestest = (gy * p.w + (gx == 0 ? gx : gx - 1)) + ivec4(0, 1, 2, 3) * p.cstep; \n\
    // R Conv \n\
    if (histogram_int32_data[data_offset.r] == 0) \n\
    { \n\
        int v_r = histogram_int32_data[data_nestest.r] / 2; \n\
        histogram_float32_data[data_nestest.r] = \n\
        histogram_float32_data[data_offset.r] = (p.log_view == 1 ? log2(float(v_r + 1)) : float(v_r)) * p.scale; \n\
    } \n\
    else \n\
    { \n\
        int v_r = histogram_int32_data[data_offset.r]; \n\
        histogram_float32_data[data_offset.r] = (p.log_view == 1 ? log2(float(v_r + 1)) : float(v_r)) * p.scale; \n\
    } \n\
    // G Conv \n\
    if (histogram_int32_data[data_offset.g] == 0) \n\
    { \n\
        int v_g = histogram_int32_data[data_nestest.g] / 2; \n\
        histogram_float32_data[data_nestest.g] = \n\
        histogram_float32_data[data_offset.g] = (p.log_view == 1 ? log2(float(v_g + 1)) : float(v_g)) * p.scale; \n\
    } \n\
    else \n\
    { \n\
        int v_g = histogram_int32_data[data_offset.g]; \n\
        histogram_float32_data[data_offset.g] = (p.log_view == 1 ? log2(float(v_g + 1)) : float(v_g)) * p.scale; \n\
    } \n\
    // B Conv \n\
    if (histogram_int32_data[data_offset.b] == 0) \n\
    { \n\
        int v_b = histogram_int32_data[data_nestest.b] / 2; \n\
        histogram_float32_data[data_nestest.b] = \n\
        histogram_float32_data[data_offset.b] = (p.log_view == 1 ? log2(float(v_b + 1)) : float(v_b)) * p.scale; \n\
    } \n\
    else \n\
    { \n\
        int v_b = histogram_int32_data[data_offset.b]; \n\
        histogram_float32_data[data_offset.b] = (p.log_view == 1 ? log2(float(v_b + 1)) : float(v_b)) * p.scale; \n\
    } \n\
    // A Conv \n\
    //if (histogram_int32_data[data_offset.a] == 0) \n\
    //{ \n\
    //    int v_a = histogram_int32_data[data_nestest.a] / 2; \n\
    //    histogram_float32_data[data_nestest.a] = \n\
    //    histogram_float32_data[data_offset.a] = (p.log_view == 1 ? log2(float(v_a + 1)) : float(v_a)) * p.scale; \n\
    //} \n\
    //else \n\
    //{ \n\
    //    int v_a = histogram_int32_data[data_offset.a]; \n\
    //    histogram_float32_data[data_offset.a] = (p.log_view == 1 ? log2(float(v_a + 1)) : float(v_a)) * p.scale; \n\
    //} \n\
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