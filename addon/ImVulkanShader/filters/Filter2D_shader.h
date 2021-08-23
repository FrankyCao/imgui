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
	int xksize;	\n\
    int yksize;	\n\
    int xanchor; \n\
    int yanchor; \n\
} p; \
"

#define SHADER_FILTER_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    sfpvec3 sum = sfpvec3(sfp(0.0f)); \n\
    int kInd = 0; \n\
    for (int i = 0; i < p.yksize; ++i) \n\
    { \n\
        for (int j= 0; j < p.xksize; ++j) \n\
        { \n\
            int x = uv.x - p.xanchor + j; \n\
            int y = uv.y - p.yanchor + i; \n\
            // REPLICATE border \n\
            x = max(0, min(x, p.w - 1)); \n\
            y = max(0, min(y, p.h - 1)); \n\
            sfpvec3 rgb = load_float_rgba(x, y, p.w, p.cstep, p.format).rgb * sfp(kernel_data[kInd++]); \n\
            sum = sum + rgb; \n\
        } \n\
    } \n\
    store_float_rgba(sfpvec4(sum, sfp(1.0f)), uv.x, uv.y, p.w, p.cstep, p.format); \n\
} \
"

static const char Filter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
layout (binding = 2) readonly buffer kernel_float { float kernel_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_FLOAT_RGBA
SHADER_STORE_FLOAT_RGBA
SHADER_FILTER_MAIN
;
