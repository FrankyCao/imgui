#pragma once
#include <imvk_mat_shader.h>

#define DSOBEL_PARAM \
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
    float edgeStrength; \n\
} p; \
"

#define SHADER_DSOBEL_MAIN \
" \n\
const sfp horizontKernel[9] = { sfp(-1.0f), sfp(-2.0f), sfp(-1.0f), \n\
                                sfp( 0.0f), sfp( 0.0f), sfp( 0.0f), \n\
                                sfp( 1.0f), sfp( 2.0f), sfp( 1.0f)}; \n\
const sfp verticalKernel[9] = { sfp(-1.0f), sfp( 0.0f), sfp( 1.0f), \n\
                                sfp(-2.0f), sfp( 0.0f), sfp( 2.0f), \n\
                                sfp(-1.0f), sfp( 0.0f), sfp( 1.0f)}; \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.out_w || gy >= p.out_h) \n\
        return; \n\
    sfp vertical = sfp(0.0f); \n\
    sfp horizont = sfp(0.0f); \n\
    for (int i = 0; i < 3; ++i) \n\
    { \n\
        for (int j = 0; j < 3; ++j) \n\
        { \n\
            int x = gx - 1 + j; \n\
            int y = gy - 1 + i; \n\
            // REPLICATE border \n\
            x = max(0, min(x, p.out_w - 1)); \n\
            y = max(0, min(y, p.out_h - 1)); \n\
            int index = j + i * 3; \n\
            sfp value = load_rgba(x, y, p.w, p.cstep, p.in_format, p.in_type).r; \n\
            vertical += value * verticalKernel[index]; \n\
            horizont += value * horizontKernel[index]; \n\
        } \n\
    } \n\
    sfpvec3 sum = sfpvec3(0.0f); \n\
    sum.x = length(sfpvec2(horizont, vertical)) * sfp(p.edgeStrength); \n\
    // dx 奇怪了?y/z是否反了,但是GPUImage逻辑确实是这样 \n\
    sum.y = vertical; \n\
    // dy \n\
    sum.z = horizont; \n\
    store_rgb_float_no_clamp(sum, gx, gy, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char DSobelFilter_data[] = 
SHADER_HEADER
DSOBEL_PARAM
SHADER_INPUT_OUTPUT_DATA
SHADER_LOAD_RGBA
SHADER_STORE_RGB_FLOAT_NO_CLAMP // 保存的数值包括矢量, 不可做clamp
SHADER_DSOBEL_MAIN
;

#define NMS_SHADER_PARAM \
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
    float minThreshold; \n\
    float maxThreshold; \n\
} p; \
"

#define SHADER_NMS_MAIN \
" \n\
sfpvec2 normDirection(sfpvec2 dxy) \n\
{ \n\
    sfpvec2 normalizedDirection = normalize(dxy); \n\
    // Offset by 1-sin(pi/8) to set to 0 if near axis, 1 if away \n\
    normalizedDirection = sign(normalizedDirection) * floor(abs(normalizedDirection) + sfp(0.617316f)); \n\
    // Place -1.0 - 1.0 within 0 - 1.0 \n\
    //normalizedDirection = (normalizedDirection + 1.0) * 0.5; \n\
    return normalizedDirection; \n\
} \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.out_w || gy >= p.out_h) \n\
        return; \n\
    //sfpvec2 suv = sfpvec2(gx, gy) + sfpvec2(0.5f); \n\
    sfpvec3 gradinetAndDirection = load_rgb(gx, gy, p.w, p.cstep, p.in_format, p.in_type); \n\
    sfpvec2 direction = normDirection(gradinetAndDirection.yz); \n\
    sfp firstGradientMagnitude = load_rgb(gx + int(direction.x), gy + int(direction.y), p.w, p.cstep, p.in_format, p.in_type).x; \n\
    sfp secondGradientMagnitude = load_rgb(gx - int(direction.x), gy - int(direction.y), p.w, p.cstep, p.in_format, p.in_type).x; \n\
    sfp multiplier = step(firstGradientMagnitude, gradinetAndDirection.x); \n\
    multiplier = multiplier * step(secondGradientMagnitude, gradinetAndDirection.x); \n\
    sfp thresholdCompliance = smoothstep(sfp(p.minThreshold), sfp(p.maxThreshold), gradinetAndDirection.x); \n\
    multiplier = multiplier * thresholdCompliance; \n\
    store_gray(multiplier, gx, gy, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char NMSFilter_data[] = 
SHADER_HEADER
NMS_SHADER_PARAM
SHADER_INPUT_OUTPUT_DATA
SHADER_LOAD_RGB
SHADER_STORE_GRAY
SHADER_NMS_MAIN
;

#define CANNY_SHADER_PARAM \
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

#define SHADER_CANNY_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.out_w || gy >= p.out_h) \n\
        return; \n\
    sfp sum = sfp(0.0f); \n\
    sfp current = load_gray(gx, gy, p.w, p.cstep, p.in_format, p.in_type, 1.f); \n\
    for (int i = 0; i < 3; ++i) \n\
    { \n\
        for (int j = 0; j < 3; ++j) \n\
        { \n\
            int x = gx - 1 + j; \n\
            int y = gy - 1 + i; \n\
            // REPLICATE border \n\
            x = max(0, min(x, p.out_w - 1)); \n\
            y = max(0, min(y, p.out_h - 1)); \n\
            sfp value = load_gray(x, y, p.w, p.cstep, p.in_format, p.in_type, 1.f); \n\
            sum += value; \n\
        } \n\
    } \n\
    sfp sumTest = step(sfp(1.5f), sum); \n\
    sfp pixelTest = step(sfp(0.01f), current); \n\
    store_rgba(sfpvec4(sfpvec3(sumTest * pixelTest), 1.0f), gx, gy, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char CannyFilter_data[] = 
SHADER_HEADER
CANNY_SHADER_PARAM
SHADER_INPUT_OUTPUT_DATA
SHADER_LOAD_GRAY
SHADER_STORE_RGBA
SHADER_CANNY_MAIN
;
