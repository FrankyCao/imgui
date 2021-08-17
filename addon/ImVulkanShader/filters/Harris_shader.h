#pragma once
#include <vk_mat_shader.h>

#define PREWITT_PARAM \
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

#define SHADER_PREWITT_MAIN \
" \n\
// prewitt算子,水平与垂直是否反了?先按照GPUImage来,后面查证 对应GPUImageXYDerivativeFilter \n\
const sfp horizontKernel[9] = { sfp(-1.0f), sfp(0.0f), sfp(1.0f), \n\
                                sfp(-1.0f), sfp(0.0f), sfp(1.0f), \n\
                                sfp(-1.0f), sfp(0.0f), sfp(1.0f)}; \n\
const sfp verticalKernel[9] = { sfp(-1.0f), sfp(-1.0f), sfp(-1.0f), \n\
                                sfp( 0.0f), sfp( 0.0f), sfp( 0.0f), \n\
                                sfp( 1.0f), sfp( 1.0f), sfp( 1.0f)}; \n\
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
            sfp value = load_src_rgb(x, y, p.w, p.cstep, p.format).r; \n\
            vertical += value * verticalKernel[index]; \n\
            horizont += value * horizontKernel[index]; \n\
        } \n\
    } \n\
    vertical = vertical * sfp(p.edgeStrength); \n\
    horizont = horizont * sfp(p.edgeStrength); \n\
    sfpvec4 sum = sfpvec4(0.0f); \n\
    sum.x = horizont * horizont; \n\
    sum.y = vertical* vertical; \n\
    sum.z = ((horizont * vertical) + sfp(1.0f)) / sfp(2.0f); \n\
    sum.w = sfp(1.0f); \n\
    store_float_rgba(sum, gx, gy, p.w, 4, p.format); \n\
} \
"

static const char PrewittFilter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_int8 { uint8_t src_int8_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
PREWITT_PARAM
SHADER_LOAD_SRC_RGB
SHADER_STORE_FLOAT_RGBA
SHADER_PREWITT_MAIN
;

#define HARRIS_SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
\n\
    int format; \n\
\n\
    float harris; \n\
    float sensitivity; \n\
} p; \
"

#define SHADER_HARRIS_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    sfpvec4 derivativeElements = load_float_rgba(gx, gy, p.w, 4, p.format); \n\
    sfp derivativeSum = derivativeElements.x + derivativeElements.y; \n\
    sfp zElement = (derivativeElements.z * sfp(2.0f)) - sfp(1.0f); \n\
    // R = Ix^2 * Iy^2 - Ixy * Ixy - k * (Ix^2 + Iy^2)^2 \n\
    sfp cornerness = derivativeElements.x * derivativeElements.y - (zElement * zElement) - p.harris * derivativeSum * derivativeSum; \n\
    cornerness = cornerness * p.sensitivity; \n\
    store_float(cornerness, gx, gy, p.w); \n\
} \
"

static const char HarrisFilter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
HARRIS_SHADER_PARAM
SHADER_LOAD_FLOAT_RGBA
SHADER_STORE_FLOAT
SHADER_HARRIS_MAIN
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
    float threshold; \n\
} p; \
"

#define SHADER_NMS_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    sfp values[9]; \n\
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
            values[j + i * 3] = value; \n\
        } \n\
    } \n\
    // 找4是最大的点 \n\
    // 0 1 2 \n\
    // 3 4 5 \n\
    // 6 7 8 \n\
    // 如果左上角(0,1,3,6)大于当前点,multiplier=0 \n\
    sfp multiplier = sfp(1.0f) - step(values[4], values[1]); \n\
    multiplier = multiplier * (sfp(1.0f) - step(values[4], values[0])); \n\
    multiplier = multiplier * (sfp(1.0f) - step(values[4], values[3])); \n\
    multiplier = multiplier * (sfp(1.0f) - step(values[4], values[6])); \n\
    // 查找右下角(2,5,7,8)的最大值 \n\
    sfp maxValue = max(values[4], values[7]); \n\
    maxValue = max(values[4], values[8]); \n\
    maxValue = max(values[4], values[5]); \n\
    maxValue = max(values[4], values[2]); \n\
    // step(maxValue, values[4])需要当前值最大才为1 \n\
    sfp result = values[4]* step(maxValue, values[4]) * multiplier; \n\
    result = step(sfp(p.threshold), result); \n\
    sfpvec3 rgb_in = load_src_rgb(gx, gy, p.w, p.cstep, p.format); \n\
    if (result > 0) \n\
    { \n\
        rgb_in = sfpvec3(1.0, 0.0, 0.0); \n\
        for (int i = 0; i < 3; ++i) \n\
        { \n\
            for (int j = 0; j < 3; ++j) \n\
            { \n\
                int x = gx - 1 + j; \n\
                int y = gy - 1 + i; \n\
                // REPLICATE border \n\
                x = max(0, min(x, p.w - 1)); \n\
                y = max(0, min(y, p.h - 1)); \n\
                store_dst_rgb(rgb_in, x, y, p.w, p.cstep, p.format); \n\
            } \n\
        } \n\
    } \n\
    else \n\
        store_dst_rgb(rgb_in, gx, gy, p.w, p.cstep, p.format); \n\
} \
"

static const char NMSFilter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) readonly buffer src_int8 { uint8_t src_int8_data[]; };
layout (binding = 2) writeonly buffer dst_float { uint8_t dst_int8_data[]; };
)"
NMS_SHADER_PARAM
SHADER_LOAD_FLOAT
SHADER_LOAD_SRC_RGB
SHADER_STORE_DST_RGB
SHADER_NMS_MAIN
;

