#pragma once
#include <vk_mat_shader.h>

#define SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int format; \n\
\n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
\n\
    int x_offset; \n\
    int y_offset; \n\
    float alpha; \n\
} p; \
"

#define SHADER_ALPHA_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    if (uv.x + p.x_offset >= p.out_w || uv.y + p.y_offset >= p.out_h) \n\
        return; \n\
    sfpvec4 rgba = load_src_rgba(uv.x, uv.y, p.w, p.cstep, p.format); \n\
    sfpvec4 rgba_dst = load_dst_rgba(uv.x + p.x_offset, uv.y + p.y_offset, p.out_w, p.out_cstep, p.out_format); \n\
    sfpvec4 result = sfpvec4(mix(rgba_dst.rgb, rgba.rgb, rgba.a * sfp(p.alpha)), rgba_dst.a); \n\
    store_dst_rgba(result, uv.x + p.x_offset, uv.y + p.y_offset, p.out_w, p.out_cstep, p.out_format); \n\
} \
"

static const char CopyTo_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_int8 { uint8_t src_int8_data[]; };
layout (binding = 1) buffer dst_int8 { uint8_t dst_int8_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_SRC_RGBA
SHADER_LOAD_DST_RGBA
SHADER_STORE_DST_RGBA
SHADER_ALPHA_MAIN
;
