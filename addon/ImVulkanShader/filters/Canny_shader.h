#pragma once
#include <vk_mat_shader.h>

#define DSOBEL_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
\n\
    int format; \n\
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
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
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
            x = max(0, min(x, p.w - 1)); \n\
            y = max(0, min(y, p.h - 1)); \n\
            int index = j + i * 3; \n\
            sfp value = load_float_rgba(x, y, p.w, p.cstep, p.format).r; \n\
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
    store_float_rgb(sum, gx, gy, p.w, 3, p.format); \n\
} \
"

static const char DSobelFilter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
DSOBEL_PARAM
SHADER_LOAD_FLOAT_RGBA
SHADER_STORE_FLOAT_RGB
SHADER_DSOBEL_MAIN
;

#define NMS_SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
\n\
    int format; \n\
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
    // normalizedDirection = (normalizedDirection + 1.0) * 0.5; \n\
    return normalizedDirection; \n\
} \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    sfpvec2 suv = sfpvec2(gx, gy) + sfpvec2(0.5f); \n\
    sfpvec3 gradinetAndDirection = load_float_rgb(int(suv.x), int(suv.y), p.w, 3, p.format); \n\
    sfpvec2 direction = normDirection(gradinetAndDirection.gb); \n\
    sfp firstGradientMagnitude = load_float_rgb(int(suv.x + direction.x), int(suv.y + direction.y), p.w, 3, p.format).r; \n\
    sfp secondGradientMagnitude = load_float_rgb(int(suv.x - direction.x), int(suv.y - direction.y), p.w, 3, p.format).r; \n\
    sfp multiplier = step(firstGradientMagnitude, gradinetAndDirection.r); \n\
    multiplier = multiplier * step(secondGradientMagnitude, gradinetAndDirection.r); \n\
    sfp thresholdCompliance = smoothstep(p.minThreshold, p.maxThreshold, gradinetAndDirection.r); \n\
    multiplier = multiplier * thresholdCompliance; \n\
    store_float(multiplier, gx, gy, p.w); \n\
} \
"

static const char NMSFilter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
NMS_SHADER_PARAM
SHADER_LOAD_FLOAT_RGB
SHADER_STORE_FLOAT
SHADER_NMS_MAIN
;

#define CANNY_SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
\n\
    int format; \n\
} p; \
"

#define SHADER_CANNY_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    sfp sum = sfp(0.0f); \n\
    sfp current = load_float(gx, gy, p.w); \n\
    for (int i = 0; i < 3; ++i) \n\
    { \n\
        for (int j = 0; j < 3; ++j) \n\
        { \n\
            int x = gx - 1 + j; \n\
            int y = gy - 1 + i; \n\
            // REPLICATE border \n\
            x = max(0, min(x, p.w - 1)); \n\
            y = max(0, min(y, p.h - 1)); \n\
            sfp value = load_float(x, y, p.w); \n\
            sum += value; \n\
        } \n\
    } \n\
    sfp sumTest = step(sfp(1.5f), sum); \n\
    sfp pixelTest = step(sfp(0.01f), current); \n\
    store_float_rgba(sfpvec4(sfpvec3(sumTest * pixelTest), 1.0f), gx, gy, p.w, p.cstep, p.format); \n\
} \
"

static const char CannyFilter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
CANNY_SHADER_PARAM
SHADER_LOAD_FLOAT
SHADER_STORE_FLOAT_RGBA
SHADER_CANNY_MAIN
;
