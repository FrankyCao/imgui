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
	int _x;	\n\
    int _y;	\n\
    int _w;	\n\
    int _h;	\n\
} p; \
"

#define SHADER_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p._w || uv.y >= p._h) \n\
        return; \n\
    sfpvec3 result = load_src_rgb(uv.x + p._x, uv.y + p._y, p.w, p.cstep, p.format); \n\
    store_dst_rgb(result, uv.x, uv.y, p._w, p.cstep, p.format); \n\
} \
"

static const char Shader_data[] = 
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
