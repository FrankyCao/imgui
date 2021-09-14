#pragma once
#include <vk_mat_shader.h>

#define SHADER_MAT_Y2R \
" \n\
sfpmat3 matrix_mat_y2r = { \n\
    {sfp(convert_matrix_y2r[0]), sfp(convert_matrix_y2r[3]), sfp(convert_matrix_y2r[6])}, \n\
    {sfp(convert_matrix_y2r[1]), sfp(convert_matrix_y2r[4]), sfp(convert_matrix_y2r[7])}, \n\
    {sfp(convert_matrix_y2r[2]), sfp(convert_matrix_y2r[5]), sfp(convert_matrix_y2r[8])}, \n\
}; \
"

#define SHADER_YUV2RGB \
" \n\
sfpvec3 yuv_to_rgb(sfpvec3 yuv) \n\
{ \n\
    sfpvec3 rgb;\n\
    sfpvec3 yuv_offset = {sfp(0.f), sfp(0.5f), sfp(0.5f)}; \n\
    if (p.in_range == CR_NARROW_RANGE) \n\
        yuv_offset.x = sfp(16.0f / 255.0f); \n\
    rgb = matrix_mat_y2r * (yuv - yuv_offset); \n\
    return clamp(rgb, sfp(0.f), sfp(1.f)); \n\
}\n\
"

#define SHADER_PARAM_Y2R \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int in_format; \n\
    int in_type; \n\
    int in_space; \n\
    int in_range; \n\
    float in_scale; \n\
    int out_format; \n\
    int out_type; \n\
} p;\
"

#define SHADER_LOAD_SRC_YUV \
" \n\
sfpvec3 load_src_yuv(int x, int y) \n\
{ \n\
    sfpvec3 yuv_in = {sfp(0.f), sfp(0.5f), sfp(0.5f)}; \n\
    int uv_scale_w = p.in_format == CF_YUV420 || p.in_format == CF_YUV422 ? 2 : 1; \n\
    int uv_scale_h = p.in_format == CF_YUV420 || p.in_format == CF_NV12 ? 2 : 1; \n\
    int y_offset = y * p.w + x; \n\
    int u_offset = p.w * p.h + (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
    int v_offset = p.w * p.h * 2 + (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
    ivec2 uv_offset = p.w * p.h + ((y / 2) * p.w / 2 + x / 2) * 2 + ivec2(0, 1); \n\
    if (p.in_type == DT_INT8) \n\
    { \n\
        yuv_in.x = sfp(uint(YUV_data_int8[y_offset])) / sfp(p.in_scale); \n\
        if (p.in_format == CF_NV12) \n\
        { \n\
            yuv_in.y = sfp(uint(YUV_data_int8[uv_offset.x])) / sfp(p.in_scale); \n\
            yuv_in.z = sfp(uint(YUV_data_int8[uv_offset.y])) / sfp(p.in_scale); \n\
        } \n\
        else \n\
        { \n\
            yuv_in.y = sfp(uint(YUV_data_int8[u_offset])) / sfp(p.in_scale); \n\
            yuv_in.z = sfp(uint(YUV_data_int8[v_offset])) / sfp(p.in_scale); \n\
        } \n\
    } \n\
    else if (p.in_type == DT_INT16) \n\
    { \n\
        yuv_in.x = sfp(uint(YUV_data_int16[y_offset])) / sfp(p.in_scale); \n\
        if (p.in_format == CF_NV12) \n\
        { \n\
            yuv_in.y = sfp(uint(YUV_data_int16[uv_offset.x])) / sfp(p.in_scale); \n\
            yuv_in.z = sfp(uint(YUV_data_int16[uv_offset.y])) / sfp(p.in_scale); \n\
        } \n\
        else \n\
        { \n\
            yuv_in.y = sfp(uint(YUV_data_int16[u_offset])) / sfp(p.in_scale); \n\
            yuv_in.z = sfp(uint(YUV_data_int16[v_offset])) / sfp(p.in_scale); \n\
        } \n\
    } \n\
    else if (p.in_type == DT_FLOAT16) \n\
    { \n\
        yuv_in.x = sfp(YUV_data_float16[y_offset]); \n\
        if (p.in_format == CF_NV12) \n\
        { \n\
            yuv_in.y = sfp(YUV_data_float16[uv_offset.x]); \n\
            yuv_in.z = sfp(YUV_data_float16[uv_offset.y]); \n\
        } \n\
        else \n\
        { \n\
            yuv_in.y = sfp(YUV_data_float16[u_offset]); \n\
            yuv_in.z = sfp(YUV_data_float16[v_offset]); \n\
        } \n\
    } \n\
    else if (p.in_type == DT_FLOAT32) \n\
    { \n\
        yuv_in.x = sfp(YUV_data_float32[y_offset]); \n\
        if (p.in_format == CF_NV12) \n\
        { \n\
            yuv_in.y = sfp(YUV_data_float32[uv_offset.x]); \n\
            yuv_in.z = sfp(YUV_data_float32[uv_offset.y]); \n\
        } \n\
        else \n\
        { \n\
            yuv_in.y = sfp(YUV_data_float32[u_offset]); \n\
            yuv_in.z = sfp(YUV_data_float32[v_offset]); \n\
        } \n\
    } \n\
    return yuv_in; \n\
} \
"

#define SHADER_YUV2RGB_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    sfpvec3 rgb = yuv_to_rgb(load_src_yuv(gx, gy)); \n\
    store_rgba(sfpvec4(rgb, 1.0f), gx, gy, p.w, p.cstep, p.out_format, p.out_type); \n\
} \
"

static const char YUV2RGB_data[] = 
SHADER_HEADER
SHADER_OUTPUT_DATA      // binging 0-3
R"(
layout (binding = 4) readonly buffer YUV_int8       { uint8_t       YUV_data_int8[]; };
layout (binding = 5) readonly buffer YUV_int16      { uint16_t      YUV_data_int16[]; };
layout (binding = 6) readonly buffer YUV_float16    { float16_t     YUV_data_float16[]; };
layout (binding = 7) readonly buffer YUV_float32    { float         YUV_data_float32[]; };
layout (binding = 8) readonly buffer mat_y2r        { float         convert_matrix_y2r[]; };
)"
SHADER_MAT_Y2R
SHADER_PARAM_Y2R
SHADER_YUV2RGB
SHADER_LOAD_SRC_YUV
SHADER_STORE_RGBA
SHADER_YUV2RGB_MAIN
;


#define SHADER_PARAM_G2R \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int in_format; \n\
    int in_type; \n\
    \n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
    int out_type; \n\
    float in_scale; \n\
} p;\
"

#define SHADER_GRAY2RGB_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.out_w || gy >= p.out_h) \n\
        return; \n\
    sfp gray = load_gray(gx, gy, p.w, 1, p.in_format, p.in_type, p.in_scale); \n\
    if (p.in_format == CF_NV12) \n\
        store_rgba_side_by_side(sfpvec4(sfpvec3(gray), 1.0f), gx, gy, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
    else \n\
        store_rgba(sfpvec4(sfpvec3(gray), 1.0f), gx, gy, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char GRAY2RGB_data[] = 
SHADER_HEADER
SHADER_INPUT_OUTPUT_DATA
SHADER_PARAM_G2R
SHADER_LOAD_GRAY
SHADER_STORE_RGBA
SHADER_STORE_RGBA_SIDE_BY_SIDE
SHADER_GRAY2RGB_MAIN
;

#define SHADER_PARAM_CONV \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int in_format; \n\
    int in_type; \n\
    \n\
    int out_w; \n\
    int out_h; \n\
    int out_cstep; \n\
    int out_format; \n\
    int out_type; \n\
} p;\
"

#define SHADER_CONV_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.out_w || gy >= p.out_h) \n\
        return; \n\
    sfpvec4 rgba = load_rgba(gx, gy, p.w, p.cstep, p.in_format, p.in_type); \n\
    store_rgba(rgba, gx, gy, p.out_w, p.out_cstep, p.out_format, p.out_type); \n\
} \
"

static const char Conv_data[] = 
SHADER_HEADER
SHADER_INPUT_OUTPUT_DATA
SHADER_PARAM_CONV
SHADER_LOAD_RGBA
SHADER_STORE_RGBA
SHADER_CONV_MAIN
;