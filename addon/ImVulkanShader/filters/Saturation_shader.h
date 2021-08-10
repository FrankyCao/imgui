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
    float saturation; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
const sfpvec3 W = sfpvec3(sfp(0.2125f), sfp(0.7154f), sfp(0.0721f)); \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    sfpvec3 color = load_src_rgb(gx, gy, p.w, p.cstep, p.format); \n\
    sfp luminance = dot(color.rgb, W); \n\
    sfpvec3 result = mix(sfpvec3(luminance), color.rgb, p.saturation); \n\
    store_dst_rgb(result, gx, gy, p.w, p.cstep, p.format); \n\
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
SHADER_MAIN
;
