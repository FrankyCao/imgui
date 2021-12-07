#pragma once
#include <imvk_mat_shader.h>

#define SHADER_MAT_Y2R \
" \n\
sfpmat3 matrix_mat_y2r = { \n\
    {sfp(convert_matrix_y2r[0]), sfp(convert_matrix_y2r[3]), sfp(convert_matrix_y2r[6])}, \n\
    {sfp(convert_matrix_y2r[1]), sfp(convert_matrix_y2r[4]), sfp(convert_matrix_y2r[7])}, \n\
    {sfp(convert_matrix_y2r[2]), sfp(convert_matrix_y2r[5]), sfp(convert_matrix_y2r[8])}, \n\
}; \
"

#define SHADER_MAT_R2Y \
" \n\
sfpmat3 matrix_mat_r2y = { \n\
    {sfp(convert_matrix_r2y[0]), sfp(convert_matrix_r2y[3]), sfp(convert_matrix_r2y[6])}, \n\
    {sfp(convert_matrix_r2y[1]), sfp(convert_matrix_r2y[4]), sfp(convert_matrix_r2y[7])}, \n\
    {sfp(convert_matrix_r2y[2]), sfp(convert_matrix_r2y[5]), sfp(convert_matrix_r2y[8])}, \n\
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
    int uv_scale_h = p.in_format == CF_YUV420 || p.in_format == CF_NV12 || p.in_format == CF_P010LE ? 2 : 1; \n\
    int y_offset = y * p.w + x; \n\
    int u_offset = p.w * p.h + (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
    int v_offset = (p.in_format == CF_YUV444 ? p.w * p.h * 2 : p.w * p.h + (p.w / uv_scale_w) * (p.h / uv_scale_h)) \n\
                 + (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
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
        if (p.in_format == CF_P010LE) \n\
            yuv_in.x = sfp(uint(YUV_data_int16[y_offset])) / sfp(65535.0); \n\
        else \n\
            yuv_in.x = sfp(uint(YUV_data_int16[y_offset])) / sfp(p.in_scale); \n\
        if (p.in_format == CF_NV12) \n\
        { \n\
            yuv_in.y = sfp(uint(YUV_data_int16[uv_offset.x])) / sfp(p.in_scale); \n\
            yuv_in.z = sfp(uint(YUV_data_int16[uv_offset.y])) / sfp(p.in_scale); \n\
        } \n\
        else if (p.in_format == CF_P010LE) \n\
        { \n\
            yuv_in.y = sfp(uint(YUV_data_int16[uv_offset.x])) / sfp(65535.0); \n\
            yuv_in.z = sfp(uint(YUV_data_int16[uv_offset.y])) / sfp(65535.0); \n\
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

#define SHADER_PARAM_R2Y \
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
    int out_cstep; \n\
    int out_format; \n\
    int out_type; \n\
    int out_space; \n\
    int out_range; \n\
    float in_scale; \n\
} p;\
"

#define SHADER_RGB2YUV \
" \n\
sfpvec3 rgb_to_yuv(sfpvec3 rgb) \n\
{\n\
    sfpvec3 yuv; \n\
    sfpvec3 yuv_offset = {sfp(0.f), sfp(0.5f), sfp(0.5)}; \n\
    if (p.out_range == CR_NARROW_RANGE) \n\
        yuv_offset.x = sfp(16.0 / 255.0); \n\
    yuv = yuv_offset + matrix_mat_r2y * rgb; \n\
    return clamp(yuv, sfp(0.f), sfp(1.f)); \n\
} \
"

#define SHADER_STORE_YUV_INT8 \
" \n\
void store_yuv_int8(sfpvec3 yuv, int y_offset, int u_offset, int v_offset, ivec2 uv_offset) \n\
{ \n\
    dst_data_int8[y_offset] = uint8_t(clamp(uint(floor(yuv.x * sfp(255.0f))), 0, 255)); \n\
    if (p.out_format == CF_NV12) \n\
    { \n\
        dst_data_int8[uv_offset.x] = uint8_t(clamp(uint(floor(yuv.y * sfp(255.0f))), 0, 255)); \n\
        dst_data_int8[uv_offset.y] = uint8_t(clamp(uint(floor(yuv.z * sfp(255.0f))), 0, 255)); \n\
    } \n\
    else \n\
    { \n\
        dst_data_int8[u_offset] = uint8_t(clamp(uint(floor(yuv.y * sfp(255.0f))), 0, 255)); \n\
        dst_data_int8[v_offset] = uint8_t(clamp(uint(floor(yuv.z * sfp(255.0f))), 0, 255)); \n\
    } \n\
} \
"

#define SHADER_STORE_YUV_INT16 \
" \n\
void store_yuv_int16(sfpvec3 yuv, int y_offset, int u_offset, int v_offset, ivec2 uv_offset) \n\
{ \n\
    if (p.out_format == CF_P010LE) \n\
    { \n\
        uint16_t y = uint16_t(uint(floor(yuv.x * sfp(255.0)))); \n\
        uint16_t hy = uint16_t(uint(floor(yuv.x * sfp(65535.0)))); \n\
        dst_data_int16[y_offset] = y | hy; \n\
    } \n\
    else \n\
        dst_data_int16[y_offset] = uint16_t(clamp(uint(floor(yuv.x * sfp(p.in_scale))), 0, uint(p.in_scale))); \n\
    if (p.out_format == CF_NV12) \n\
    { \n\
        dst_data_int16[uv_offset.x] = uint16_t(clamp(uint(floor(yuv.y * sfp(p.in_scale))), 0, uint(p.in_scale))); \n\
        dst_data_int16[uv_offset.y] = uint16_t(clamp(uint(floor(yuv.z * sfp(p.in_scale))), 0, uint(p.in_scale))); \n\
    } \n\
    else if (p.out_format == CF_P010LE) \n\
    { \n\
        uint16_t u = uint16_t(uint(floor(yuv.y * sfp(255.0)))); \n\
        uint16_t hu = uint16_t(uint(floor(yuv.y * sfp(65535.0)))); \n\
        uint16_t v = uint16_t(uint(floor(yuv.z * sfp(255.0)))); \n\
        uint16_t hv = uint16_t(uint(floor(yuv.z * sfp(65535.0)))); \n\
        dst_data_int16[uv_offset.x] = u | hu; \n\
        dst_data_int16[uv_offset.y] = v | hv; \n\
    } \n\
    else \n\
    { \n\
        dst_data_int16[u_offset] = uint16_t(clamp(uint(floor(yuv.y * sfp(p.in_scale))), 0, uint(p.in_scale))); \n\
        dst_data_int16[v_offset] = uint16_t(clamp(uint(floor(yuv.z * sfp(p.in_scale))), 0, uint(p.in_scale))); \n\
    } \n\
} \
"

#define SHADER_STORE_YUV_FLOAT16 \
" \n\
void store_yuv_float16(sfpvec3 yuv, int y_offset, int u_offset, int v_offset, ivec2 uv_offset) \n\
{ \n\
    dst_data_float16[y_offset] = float16_t(clamp(yuv.x, sfp(0.0f), sfp(1.0f))); \n\
    if (p.out_format == CF_NV12) \n\
    { \n\
        dst_data_float16[uv_offset.x] = float16_t(clamp(yuv.y, sfp(0.0f), sfp(1.0f))); \n\
        dst_data_float16[uv_offset.y] = float16_t(clamp(yuv.z, sfp(0.0f), sfp(1.0f))); \n\
    } \n\
    else \n\
    { \n\
        dst_data_float16[u_offset] = float16_t(clamp(yuv.y, sfp(0.0f), sfp(1.0f))); \n\
        dst_data_float16[v_offset] = float16_t(clamp(yuv.z, sfp(0.0f), sfp(1.0f))); \n\
    } \n\
} \
"

#define SHADER_STORE_YUV_FLOAT32 \
" \n\
void store_yuv_float32(sfpvec3 yuv, int y_offset, int u_offset, int v_offset, ivec2 uv_offset) \n\
{ \n\
    dst_data_float32[y_offset] = clamp(yuv.x, sfp(0.0f), sfp(1.0f)); \n\
    if (p.out_format == CF_NV12) \n\
    { \n\
        dst_data_float32[uv_offset.x] = float(clamp(yuv.y, sfp(0.0f), sfp(1.0f))); \n\
        dst_data_float32[uv_offset.y] = float(clamp(yuv.z, sfp(0.0f), sfp(1.0f))); \n\
    } \n\
    else \n\
    { \n\
        dst_data_float32[u_offset] = float(clamp(yuv.y, sfp(0.0f), sfp(1.0f))); \n\
        dst_data_float32[v_offset] = float(clamp(yuv.z, sfp(0.0f), sfp(1.0f))); \n\
    } \n\
} \
"

#define SHADER_STORE_YUV \
SHADER_STORE_YUV_INT8 \
SHADER_STORE_YUV_INT16 \
SHADER_STORE_YUV_FLOAT16 \
SHADER_STORE_YUV_FLOAT32 \
" \n\
void store_dst_yuv(sfpvec3 v, int x, int y) \n\
{ \n\
    int uv_scale_w = p.out_format == CF_YUV420 || p.out_format == CF_YUV422 ? 2 : 1; \n\
    int uv_scale_h = p.out_format == CF_YUV420 || p.out_format == CF_NV12 ? 2 : 1; \n\
    int y_offset = y * p.w + x; \n\
    int u_offset = p.out_cstep + (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
    int v_offset = p.out_cstep * 2 + (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
    ivec2 uv_offset = p.out_cstep + ((y / 2) * p.w / 2 + x / 2) * 2 + ivec2(0, 1); \n\
    if (p.out_type == DT_INT8) \n\
        store_yuv_int8(v, y_offset, u_offset, v_offset, uv_offset); \n\
    else if (p.out_type == DT_INT16) \n\
        store_yuv_int16(v, y_offset, u_offset, v_offset, uv_offset); \n\
    else if (p.out_type == DT_FLOAT16) \n\
        store_yuv_float16(v, y_offset, u_offset, v_offset, uv_offset); \n\
    else if (p.out_type == DT_FLOAT32) \n\
        store_yuv_float32(v, y_offset, u_offset, v_offset, uv_offset); \n\
} \
"

#define SHADER_RGB2YUV_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    if (gx >= p.w || gy >= p.h) \n\
        return; \n\
    sfpvec3 yuv = rgb_to_yuv(load_rgba(gx, gy, p.w, p.cstep, p.in_format, p.in_type).rgb); \n\
    store_dst_yuv(yuv, gx, gy); \n\
} \
"

static const char RGB2YUV_data[] = 
SHADER_HEADER
SHADER_INPUT_OUTPUT_DATA
R"(
layout (binding = 8) readonly buffer mat_r2y        { float         convert_matrix_r2y[]; };
)"
SHADER_MAT_R2Y
SHADER_PARAM_R2Y
SHADER_LOAD_RGBA
SHADER_RGB2YUV
SHADER_STORE_YUV
SHADER_RGB2YUV_MAIN
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