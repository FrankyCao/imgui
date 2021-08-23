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
    float contrast; \n\
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
    sfpvec4 color = load_float_rgba(gx, gy, p.w, p.cstep, p.format); \n\
    sfpvec3 result = ((color.rgb - sfpvec3(0.5f)) * sfp(p.contrast) + sfpvec3(0.5f)); \n\
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
SHADER_MAIN
;
