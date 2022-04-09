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
    \n\
    int separate; \n\
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
    int dx = p.separate == 1 ? gx / 3 : gx; \n\
    int ox = p.separate == 1 ? p.out_w / 3 : 0; \n\
    sfpvec4 rgba = load_rgba(gx, gy, p.w, p.cstep, p.in_format, p.in_type); \n\
    int dyr = int(rgba.r * (p.out_h - 1)); \n\
    ivec4 offset_r = (dyr * p.out_w + dx) * p.out_cstep + ivec4(0, 1, 2, 3); \n\
    int dyg = int(rgba.g * (p.out_h - 1)); \n\
    ivec4 offset_g = (dyg * p.out_w + dx + ox) * p.out_cstep + ivec4(0, 1, 2, 3); \n\
    int dyb = int(rgba.b * (p.out_h - 1)); \n\
    ivec4 offset_b = (dyb * p.out_w + dx + ox + ox) * p.out_cstep + ivec4(0, 1, 2, 3); \n\
    //int dya = int(rgba.a * (p.out_h - 1)); \n\
    //ivec4 offset_a = (dya * p.out_w + dx + ox + ox + ox) * p.out_cstep + ivec4(0, 1, 2, 3); \n\
    memoryBarrierBuffer(); \n\
    atomicAdd(waveform_int32_data[offset_r.r], 1); \n\
    atomicAdd(waveform_int32_data[offset_g.g], 1); \n\
    atomicAdd(waveform_int32_data[offset_b.b], 1); \n\
    //atomicAdd(waveform_int32_data[offset_a.a], 1); \n\
    memoryBarrierBuffer(); \n\
} \
"

static const char Waveform_data[] = 
SHADER_HEADER
SHADER_PARAM
SHADER_SRC_DATA
R"(
layout (binding = 4) restrict buffer waveform_int32  { int waveform_int32_data[]; };
)"
SHADER_LOAD_RGBA
SHADER_MAIN
;

#define PARAM_ZERO \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
} p;\
"

#define ZERO_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    ivec4 in_offset = (gy * p.w + gx) * p.cstep + ivec4(0, 1, 2, 3); \n\
    waveform_int32_data[in_offset.r] = 0; \n\
    waveform_int32_data[in_offset.g] = 0; \n\
    waveform_int32_data[in_offset.b] = 0; \n\
    waveform_int32_data[in_offset.a] = 0; \n\
} \
"

static const char Zero_data[] =
SHADER_HEADER
PARAM_ZERO
R"(
layout (binding = 0) restrict buffer waveform_int32  { int waveform_int32_data[]; };
)"
ZERO_MAIN
;

#define PARAM_CONV \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int out_cstep; \n\
    int out_format; \n\
    float intensity; \n\
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
    ivec4 in_offset_nestest = ((gy == 0 ? gy : gy - 1) * p.w + gx) * p.cstep + ivec4(0, 1, 2, 3); \n\
    ivec4 in_offset = (gy * p.w + gx) * p.cstep + ivec4(0, 1, 2, 3); \n\
    ivec4 out_offset_nestest = ((gy == 0 ? gy : gy - 1) * p.w + gx) * p.out_cstep + (p.out_format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    ivec4 out_offset = (gy * p.w + gx) * p.out_cstep + (p.out_format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    // R Conv \n\
    if (waveform_int32_data[in_offset.r] == 0) \n\
    { \n\
        int v_r = waveform_int32_data[in_offset_nestest.r] / 2; \n\
        waveform_int8_data[out_offset.r] = uint8_t(clamp(v_r * p.intensity, 0, 255)); \n\
        waveform_int8_data[out_offset_nestest.r] = uint8_t(clamp(v_r * p.intensity, 0, 255)); \n\
    } \n\
    else \n\
    { \n\
        int v_r = waveform_int32_data[in_offset.r]; \n\
        waveform_int8_data[out_offset.r] = uint8_t(clamp(v_r * p.intensity, 0, 255)); \n\
    } \n\
    // G Conv \n\
    if (waveform_int32_data[in_offset.g] == 0) \n\
    { \n\
        int v_g = waveform_int32_data[in_offset_nestest.g] / 2; \n\
        waveform_int8_data[out_offset.g] = uint8_t(clamp(v_g * p.intensity, 0, 255)); \n\
        waveform_int8_data[out_offset_nestest.g] = uint8_t(clamp(v_g * p.intensity, 0, 255)); \n\
    } \n\
    else \n\
    { \n\
        int v_g = waveform_int32_data[in_offset.g]; \n\
        waveform_int8_data[out_offset.g] = uint8_t(clamp(v_g * p.intensity, 0, 255)); \n\
    } \n\
    // B Conv \n\
    if (waveform_int32_data[in_offset.b] == 0) \n\
    { \n\
        int v_b = waveform_int32_data[in_offset_nestest.b] / 2; \n\
        waveform_int8_data[out_offset.b] = uint8_t(clamp(v_b * p.intensity, 0, 255)); \n\
        waveform_int8_data[out_offset_nestest.b] = uint8_t(clamp(v_b * p.intensity, 0, 255)); \n\
    } \n\
    else \n\
    { \n\
        int v_b = waveform_int32_data[in_offset.b]; \n\
        waveform_int8_data[out_offset.b] = uint8_t(clamp(v_b * p.intensity, 0, 255)); \n\
    } \n\
    waveform_int8_data[out_offset.a] = uint8_t(255); \n\
} \
"

static const char ConvInt2Mat_data[] = 
SHADER_HEADER
PARAM_CONV
R"(
layout (binding = 0) readonly buffer waveform_int32  { int waveform_int32_data[]; };
layout (binding = 1) writeonly buffer waveform_int8  { uint8_t waveform_int8_data[]; };
)"
CONV_MAIN
;
