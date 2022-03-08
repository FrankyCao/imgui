#pragma once
#include <imvk_mat_shader.h>

#define SHADER_PARAM \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int in_format; \n\
    int in_type; \n\
\n\
    int w2; \n\
    int h2; \n\
    int cstep2; \n\
    int in_format2; \n\
    int in_type2; \n\
\n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
    int out_type; \n\
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
    sfpvec4 result; \n\
    sfpvec4 rgba_src2 = load_rgba_src2(uv.x, uv.y, p.w2, p.cstep2, p.in_format2, p.in_type2); \n\
    if (uv.x - p.x_offset >= 0 && uv.y - p.y_offset >= 0 && \n\
        uv.x - p.x_offset < p.w && uv.y - p.y_offset < p.h) \n\
    { \n\
        sfpvec4 rgba_src1 = load_rgba(uv.x - p.x_offset, uv.y - p.y_offset, p.w, p.cstep, p.in_format, p.in_type); \n\
        result = sfpvec4(mix(rgba_src2.rgb, rgba_src1.rgb, rgba_src1.a), rgba_src2.a); \n\
    } \n\
    else \n\
    { \n\
        result = rgba_src2; \n\
    } \n\
    store_rgba(result, uv.x, uv.y, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char AlphaBlending_data[] = 
SHADER_HEADER
SHADER_PARAM
SHADER_INPUT_OUTPUT_DATA
R"(
layout (binding =  8) readonly buffer src2_int8       { uint8_t   src2_data_int8[]; };
layout (binding =  9) readonly buffer src2_int16      { uint16_t  src2_data_int16[]; };
layout (binding = 10) readonly buffer src2_float16    { float16_t src2_data_float16[]; };
layout (binding = 11) readonly buffer src2_float32    { float     src2_data_float32[]; };
)"
SHADER_LOAD_RGBA
SHADER_LOAD_RGBA_NAME(src2)
SHADER_STORE_RGBA
SHADER_ALPHA_MAIN
;


#define SHADER_ALPHA_WITH_ALPHA_MAIN \
" \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    sfpvec4 result; \n\
    sfpvec4 rgba_src2 = load_rgba_src2(uv.x, uv.y, p.w2, p.cstep2, p.in_format2, p.in_type2); \n\
    if (uv.x - p.x_offset >= 0 && uv.y - p.y_offset >= 0 && \n\
        uv.x - p.x_offset < p.w && uv.y - p.y_offset < p.h) \n\
    { \n\
        sfpvec4 rgba_src1 = load_rgba(uv.x - p.x_offset, uv.y - p.y_offset, p.w, p.cstep, p.in_format, p.in_type); \n\
        result = sfpvec4(mix(rgba_src2.rgb, rgba_src1.rgb * rgba_src1.a, sfp(p.alpha)), rgba_src2.a); \n\
    } \n\
    else \n\
    { \n\
        result = rgba_src2; \n\
    } \n\
    store_rgba(result, uv.x, uv.y, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char AlphaBlending_alpha_data[] = 
SHADER_HEADER
SHADER_PARAM
SHADER_INPUT_OUTPUT_DATA
R"(
layout (binding =  8) readonly buffer src2_int8       { uint8_t   src2_data_int8[]; };
layout (binding =  9) readonly buffer src2_int16      { uint16_t  src2_data_int16[]; };
layout (binding = 10) readonly buffer src2_float16    { float16_t src2_data_float16[]; };
layout (binding = 11) readonly buffer src2_float32    { float     src2_data_float32[]; };
)"
SHADER_LOAD_RGBA
SHADER_LOAD_RGBA_NAME(src2)
SHADER_STORE_RGBA
SHADER_ALPHA_WITH_ALPHA_MAIN
;