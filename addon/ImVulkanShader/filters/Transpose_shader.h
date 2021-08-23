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
	int bFlipX;	\n\
    int bFlipY;	\n\
} p; \
"

#define SHADER_FILTER_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.h || uv.y >= p.w) \n\
        return; \n\
    int ix = uv.y; \n\
    int iy = uv.x; \n\
    if (p.bFlipX != 0) \n\
        ix = p.w - 1 - ix; \n\
    if (p.bFlipY != 0) \n\
        iy = p.h - 1 - iy; \n\
    sfpvec4 result = load_float_rgba(ix, iy, p.w, p.cstep, p.format); \n\
    store_float_rgba(result, uv.x, uv.y, p.h, p.cstep, p.format); \n\
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
SHADER_FILTER_MAIN
;
