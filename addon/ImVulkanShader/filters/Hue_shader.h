#pragma once
#include <vk_mat_shader.h>

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
    sfpvec3 color = load_float_rgba(gx, gy, p.w, p.cstep, p.format).rgb; \n\
    sfpvec3 result = hue(color); \n\
    result = clamp(result, sfpvec3(0.f), sfpvec3(1.0f)); \n\
    store_float_rgba(sfpvec4(result, 1.0f), gx, gy, p.w, p.cstep, p.format); \n\
} \
"

static const char Filter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_FLOAT_RGBA
SHADER_STORE_FLOAT_RGBA
SHADER_HUE
SHADER_MAIN
;
