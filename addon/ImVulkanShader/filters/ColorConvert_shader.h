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

#define SHADER_PARAM_Y2R \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int in_format; \n\
    int in_space; \n\
    int in_range; \n\
    float in_scale; \n\
    int out_format; \n\
} p;\
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

#define SHADER_GRAY2RGB \
" \n\
sfpvec3 gray_to_rgb(sfp gray) \n\
{ \n\
    sfpvec3 rgb = {gray, gray, gray};\n\
    return clamp(rgb, sfp(0.f), sfp(1.f)); \n\
}\n\
"

#define SHADER_LOAD_SRC_YUV \
" \n\
sfpvec3 load_src_yuv(int x, int y, int z) \n\
{ \n\
    sfpvec3 yuv_in = {sfp(0.f), sfp(0.5f), sfp(0.5f)}; \n\
    int uv_scale_w = p.in_format == CF_YUV420 || p.in_format == CF_YUV422 ? 2 : 1; \n\
    int uv_scale_h = p.in_format == CF_YUV420 || p.in_format == CF_NV12 ? 2 : 1; \n\
    int y_offset = y * p.w + x; \n\
    int u_offset = (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
    int v_offset = (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w; \n\
    ivec2 uv_offset = ((y / 2) * p.w / 2 + x / 2) * 2 + ivec2(0, 1); \n\
    yuv_in.x = sfp(uint(Y_data[y_offset])) / sfp(p.in_scale); \n\
    if (p.in_format == CF_NV12) \n\
    { \n\
        yuv_in.y = sfp(uint(U_data[uv_offset.x])) / sfp(p.in_scale); \n\
        yuv_in.z = sfp(uint(U_data[uv_offset.y])) / sfp(p.in_scale); \n\
    } \n\
    else \n\
    { \n\
        yuv_in.y = sfp(uint(U_data[u_offset])) / sfp(p.in_scale); \n\
        yuv_in.z = sfp(uint(V_data[v_offset])) / sfp(p.in_scale); \n\
    } \n\
    return yuv_in; \n\
} \
"

#define SHADER_LOAD_SRC_GRAY \
" \n\
sfp load_src_gray(int x, int y, int z) \n\
{ \n\
    sfp gray_in = sfp(0.f); \n\
    int offset = y * p.w + x; \n\
    gray_in = sfp(uint(G_data[offset])) / sfp(p.in_scale); \n\
    return gray_in; \n\
} \
"

#define SHADER_STORE_RGB \
" \n\
void store_dst_rgb(int x, int y, int z, sfpvec3 rgb) \n\
{ \n\
    ivec4 o_offset = (y * p.w + x) * p.cstep + (p.out_format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    RGBA_data[o_offset.r] = float(rgb.r); \n\
    RGBA_data[o_offset.g] = float(rgb.g); \n\
    RGBA_data[o_offset.b] = float(rgb.b); \n\
    RGBA_data[o_offset.a] = 1.0f; \n\
} \
"

#define SHADER_STORE_RGB_SIDE_BY_SIDE \
" \n\
void store_dst_rgb_side_by_side(int x, int y, int z, sfpvec3 rgb) \n\
{ \n\
    int planner = x % 2 == 0 ? x / 2 : x / 2 + p.w / 2; \n\
    ivec4 o_offset = (y * p.w + planner) * p.cstep + (p.out_format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    RGBA_data[o_offset.r] = float(rgb.r); \n\
    RGBA_data[o_offset.g] = float(rgb.g); \n\
    RGBA_data[o_offset.b] = float(rgb.b); \n\
    RGBA_data[o_offset.a] = 1.0f; \n\
} \
"

#define SHADER_YUV2RGB_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    sfpvec3 rgb = yuv_to_rgb(load_src_yuv(gx, gy, gz)); \n\
    store_dst_rgb(gx, gy, gz, rgb); \n\
} \
"

#define SHADER_GRAY2RGB_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    sfpvec3 rgb = gray_to_rgb(load_src_gray(gx, gy, gz)); \n\
    if (p.in_format == CF_NV12) \n\
        store_dst_rgb_side_by_side(gx, gy, gz, rgb); \n\
    else \n\
        store_dst_rgb(gx, gy, gz, rgb); \n\
} \
"

static const char YUV2RGB8_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer Y { uint8_t Y_data[]; };
layout (binding = 1) readonly buffer U { uint8_t U_data[]; };
layout (binding = 2) readonly buffer V { uint8_t V_data[]; };
layout (binding = 3) writeonly buffer RGBA { float RGBA_data[]; };
layout (binding = 4) readonly buffer matrix_y2r { float convert_matrix_y2r[]; };
)"
SHADER_MAT_Y2R
SHADER_PARAM_Y2R
SHADER_YUV2RGB
SHADER_LOAD_SRC_YUV
SHADER_STORE_RGB
SHADER_YUV2RGB_MAIN
;

static const char YUV2RGB16_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer Y { uint16_t Y_data[]; };
layout (binding = 1) readonly buffer U { uint16_t U_data[]; };
layout (binding = 2) readonly buffer V { uint16_t V_data[]; };
layout (binding = 3) writeonly buffer RGBA { float RGBA_data[]; };
layout (binding = 4) readonly buffer matrix_y2r { float convert_matrix_y2r[]; };
)"
SHADER_MAT_Y2R
SHADER_PARAM_Y2R
SHADER_YUV2RGB
SHADER_LOAD_SRC_YUV
SHADER_STORE_RGB
SHADER_YUV2RGB_MAIN
;

static const char GRAY2RGB8_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer G { uint8_t G_data[]; };
layout (binding = 1) writeonly buffer RGBA { float RGBA_data[]; };
)"
SHADER_PARAM_Y2R
SHADER_GRAY2RGB
SHADER_LOAD_SRC_GRAY
SHADER_STORE_RGB
SHADER_STORE_RGB_SIDE_BY_SIDE
SHADER_GRAY2RGB_MAIN
;

static const char GRAY2RGB16_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer G { uint16_t G_data[]; };
layout (binding = 1) writeonly buffer RGBA { float RGBA_data[]; };
)"
SHADER_PARAM_Y2R
SHADER_GRAY2RGB
SHADER_LOAD_SRC_GRAY
SHADER_STORE_RGB
SHADER_STORE_RGB_SIDE_BY_SIDE
SHADER_GRAY2RGB_MAIN
;

#define SHADER_PARAM_FLOAT \
" \n\
layout (push_constant) uniform parameter \n\
{ \n\
    int w; \n\
    int h; \n\
    int cstep; \n\
    int format; \n\
} p;\
"

#define SHADER_FLOAT_TO_INT8_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    sfpvec4 rgb; \n\
    if (p.cstep == 1) \n\
    { \n\
        sfp val = load_float(gx, gy, p.w); \n\
        rgb = sfpvec4(val, val, val, sfp(1.0f)); \n\
    } \n\
    else \n\
        rgb = load_float_rgba(gx, gy, p.w, p.cstep, p.format); \n\
    store_dst_rgba(rgb, gx, gy, p.w, p.cstep, p.format); \n\
} \
"

#define SHADER_FLOAT_TO_INT16_MAIN \
" \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    //sfpvec4 rgb = load_float_rgba(gx, gy, p.w, p.cstep, p.format); \n\
    // TODO::Dicky \n\
} \
"

static const char FloatInt8_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer Float_In { float src_float_data[]; };
layout (binding = 1) writeonly buffer Int8_Out { uint8_t dst_int8_data[]; };
)"
SHADER_PARAM_FLOAT
SHADER_LOAD_FLOAT
SHADER_LOAD_FLOAT_RGBA
SHADER_STORE_DST_RGBA
SHADER_FLOAT_TO_INT8_MAIN
;

static const char FloatInt16_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer Float_In { float src_float_data[]; };
layout (binding = 1) writeonly buffer Int16_Out { uint16_t dst_int16_data[]; };
)"
SHADER_PARAM_FLOAT
SHADER_LOAD_FLOAT_RGBA
//SHADER_STORE_DST_RGBA
SHADER_FLOAT_TO_INT16_MAIN
;