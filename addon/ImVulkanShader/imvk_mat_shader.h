#pragma once
// for Shader benchmark
static const char glsl_p1_data[] = R"(
#version 450
#if ImVulkan_fp16_storage
#extension GL_EXT_shader_16bit_storage: require
#endif
#if ImVulkan_fp16_arithmetic
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#endif
layout (constant_id = 0) const int count = 0;
layout (constant_id = 1) const int loop = 1;
layout (binding = 0) readonly buffer a_blob { sfp a_blob_data[]; };
layout (binding = 1) readonly buffer b_blob { sfp b_blob_data[]; };
layout (binding = 2) writeonly buffer c_blob { sfp c_blob_data[]; };
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    if (gx >= count || gy >= 1 || gz >= 1)
        return;
    afp a = buffer_ld1(a_blob_data, gx);
    afp b = buffer_ld1(b_blob_data, gx);
    afp c = afp(1.f);
    for (int i = 0; i < loop; i++)
    {
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
    }
    buffer_st1(c_blob_data, gx, c);
}
)";

static const char glsl_p4_data[] = R"(
#version 450
#if ImVulkan_fp16_storage
#extension GL_EXT_shader_16bit_storage: require
#endif
#if ImVulkan_fp16_arithmetic
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#endif
layout (constant_id = 0) const int count = 0;
layout (constant_id = 1) const int loop = 1;
layout (binding = 0) readonly buffer a_blob { sfpvec4 a_blob_data[]; };
layout (binding = 1) readonly buffer b_blob { sfpvec4 b_blob_data[]; };
layout (binding = 2) writeonly buffer c_blob { sfpvec4 c_blob_data[]; };
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    if (gx >= count || gy >= 1 || gz >= 1)
        return;
    afpvec4 a = buffer_ld4(a_blob_data, gx);
    afpvec4 b = buffer_ld4(b_blob_data, gx);
    afpvec4 c = afpvec4(1.f);
    for (int i = 0; i < loop; i++)
    {
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
    }
    buffer_st4(c_blob_data, gx, c);
}
)";

static const char glsl_p8_data[] = R"(
#version 450
#if ImVulkan_fp16_storage
#extension GL_EXT_shader_16bit_storage: require
#endif
#if ImVulkan_fp16_arithmetic
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#endif
layout (constant_id = 0) const int count = 0;
layout (constant_id = 1) const int loop = 1;
layout (binding = 0) readonly buffer a_blob { sfpvec8 a_blob_data[]; };
layout (binding = 1) readonly buffer b_blob { sfpvec8 b_blob_data[]; };
layout (binding = 2) writeonly buffer c_blob { sfpvec8 c_blob_data[]; };
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    if (gx >= count || gy >= 1 || gz >= 1)
        return;
    afpvec8 a = buffer_ld8(a_blob_data, gx);
    afpvec8 b = buffer_ld8(b_blob_data, gx);
    afpvec8 c = afpvec8(afpvec4(1.f), afpvec4(1.f));
    for (int i = 0; i < loop; i++)
    {
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
    }
    buffer_st8(c_blob_data, gx, c);
}
)";

#define SHADER_HEADER \
"\
#version 450 \n\
#extension GL_EXT_shader_8bit_storage: require \n\
#extension GL_EXT_shader_16bit_storage: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types_int8: require \n\
#extension GL_EXT_shader_atomic_float: require \n\
#extension GL_EXT_shader_atomic_float2: require \n\
#define CF_GRAY     0 \n\
#define CF_BGR      1 \n\
#define CF_ABGR     2 \n\
#define CF_BGRA     3 \n\
#define CF_RGB      4 \n\
#define CF_ARGB     5 \n\
#define CF_RGBA     6 \n\
#define CF_YUV420   7 \n\
#define CF_YUV422   8 \n\
#define CF_YUV444   9 \n\
#define CF_YUVA     10 \n\
#define CF_NV12     11 \n\
#define CF_P010LE   12 \n\
#define INTERPOLATE_NEAREST     0 \n\
#define INTERPOLATE_BILINEAR    1 \n\
#define INTERPOLATE_BICUBIC     2 \n\
#define INTERPOLATE_AREA        3 \n\
#define INTERPOLATE_TRILINEAR   4 \n\
#define INTERPOLATE_TETRAHEDRAL 5 \n\
#define CS_SRGB     0 \n\
#define CS_BT601    1 \n\
#define CS_BT709    2 \n\
#define CS_BT2020   3 \n\
#define CS_HSV      4 \n\
#define CS_HLS      5 \n\
#define CS_CMY      6 \n\
#define CS_LAB      7 \n\
#define CR_FULL_RANGE   0 \n\
#define CR_NARROW_RANGE 1 \n\
#define DT_INT8         0 \n\
#define DT_INT16        1 \n\
#define DT_INT32        2 \n\
#define DT_INT64        3 \n\
#define DT_FLOAT16      4 \n\
#define DT_FLOAT32      5 \n\
#define DT_FLOAT64      6 \n\
\
" // 43 lines

// shader binding for command data
#define SHADER_SRC_DATA \
" \n\
layout (binding = 0) readonly buffer src_int8       { uint8_t   src_data_int8[]; };   \n\
layout (binding = 1) readonly buffer src_int16      { uint16_t  src_data_int16[]; };  \n\
layout (binding = 2) readonly buffer src_float16    { float16_t src_data_float16[]; };\n\
layout (binding = 3) readonly buffer src_float32    { float     src_data_float32[]; };\n\
"

#define SHADER_INPUT_DATA \
" \n\
layout (binding = 4) readonly buffer src_int8       { uint8_t   src_data_int8[]; };   \n\
layout (binding = 5) readonly buffer src_int16      { uint16_t  src_data_int16[]; };  \n\
layout (binding = 6) readonly buffer src_float16    { float16_t src_data_float16[]; };\n\
layout (binding = 7) readonly buffer src_float32    { float     src_data_float32[]; };\n\
"

#define SHADER_OUTPUT_DATA \
" \n\
layout (binding = 0) writeonly buffer dst_int8      { uint8_t   dst_data_int8[]; };   \n\
layout (binding = 1) writeonly buffer dst_int16     { uint16_t  dst_data_int16[]; };  \n\
layout (binding = 2) writeonly buffer dst_float16   { float16_t dst_data_float16[]; };\n\
layout (binding = 3) writeonly buffer dst_float32   { float     dst_data_float32[]; };\n\
"

#define SHADER_OUTPUT_RDWR_DATA \
" \n\
layout (binding = 0) buffer dst_int8      { uint8_t   dst_data_int8[]; };   \n\
layout (binding = 1) buffer dst_int16     { uint16_t  dst_data_int16[]; };  \n\
layout (binding = 2) buffer dst_float16   { float16_t dst_data_float16[]; };\n\
layout (binding = 3) buffer dst_float32   { float     dst_data_float32[]; };\n\
"

// 8 lines
#define SHADER_INPUT_OUTPUT_DATA \
SHADER_INPUT_DATA \
SHADER_OUTPUT_DATA

// Load data as gray
#define SHADER_LOAD_GRAY_INT8 \
" \n\
sfp load_gray_int8(int x, int y, int w, int cstep, int format, float scale) \n\
{ \n\
    sfp rgb_in = sfp(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    rgb_in = sfp(uint(src_data_int8[i_offset.x])) / sfp(scale); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_GRAY_INT16 \
" \n\
sfp load_gray_int16(int x, int y, int w, int cstep, int format, float scale) \n\
{ \n\
    sfp rgb_in = sfp(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    rgb_in = sfp(uint(src_data_int16[i_offset.x])) / sfp(scale); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_GRAY_FLOAT16 \
" \n\
sfp load_gray_float16(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfp rgb_in = sfp(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    rgb_in = sfp(src_data_float16[i_offset.x]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_GRAY_FLOAT32 \
" \n\
sfp load_gray_float32(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfp rgb_in = sfp(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    rgb_in = sfp(src_data_float32[i_offset.x]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_GRAY \
SHADER_LOAD_GRAY_INT8 \
SHADER_LOAD_GRAY_INT16 \
SHADER_LOAD_GRAY_FLOAT16 \
SHADER_LOAD_GRAY_FLOAT32 \
" \n\
sfp load_gray(int x, int y, int w, int cstep, int format, int type, float scale) \n\
{ \n\
    if (type == DT_INT8) \n\
        return load_gray_int8(x, y, w, cstep, format, scale); \n\
    else if (type == DT_INT16) \n\
        return load_gray_int16(x, y, w, cstep, format, scale); \n\
    else if (type == DT_FLOAT16) \n\
        return load_gray_float16(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        return load_gray_float32(x, y, w, cstep, format); \n\
    else \n\
        return sfp(0.f); \n\
} \n\
" // 46 lines

// Load data as rgb, if cstep is 4, means input is rgba
#define SHADER_LOAD_RGB_INT8 \
" \n\
sfpvec3 load_rgb_int8(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = sfpvec3(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    rgb_in.r = sfp(uint(src_data_int8[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(src_data_int8[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(src_data_int8[i_offset.b])) / sfp(255.f); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGB_INT16 \
" \n\
sfpvec3 load_rgb_int16(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = sfpvec3(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    rgb_in.r = sfp(uint(src_data_int16[i_offset.r])) / sfp(65535.f); \n\
    rgb_in.g = sfp(uint(src_data_int16[i_offset.g])) / sfp(65535.f); \n\
    rgb_in.b = sfp(uint(src_data_int16[i_offset.b])) / sfp(65535.f); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGB_FLOAT16 \
" \n\
sfpvec3 load_rgb_float16(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = sfpvec3(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    rgb_in.r = sfp(src_data_float16[i_offset.r]); \n\
    rgb_in.g = sfp(src_data_float16[i_offset.g]); \n\
    rgb_in.b = sfp(src_data_float16[i_offset.b]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGB_FLOAT32 \
" \n\
sfpvec3 load_rgb_float32(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec3 rgb_in = sfpvec3(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    rgb_in.r = sfp(src_data_float32[i_offset.r]); \n\
    rgb_in.g = sfp(src_data_float32[i_offset.g]); \n\
    rgb_in.b = sfp(src_data_float32[i_offset.b]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGB \
SHADER_LOAD_RGB_INT8 \
SHADER_LOAD_RGB_INT16 \
SHADER_LOAD_RGB_FLOAT16 \
SHADER_LOAD_RGB_FLOAT32 \
" \n\
sfpvec3 load_rgb(int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_INT8) \n\
        return load_rgb_int8(x, y, w, cstep, format); \n\
    else if (type == DT_INT16) \n\
        return load_rgb_int16(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT16) \n\
        return load_rgb_float16(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        return load_rgb_float32(x, y, w, cstep, format); \n\
    else \n\
        return sfpvec3(0.f); \n\
} \n\
" // 54 lines

// Load data as rgba
#define SHADER_LOAD_RGBA_INT8 \
" \n\
sfpvec4 load_rgba_int8(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(uint(src_data_int8[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(src_data_int8[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(src_data_int8[i_offset.b])) / sfp(255.f); \n\
    rgb_in.a = sfp(uint(src_data_int8[i_offset.a])) / sfp(255.f); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGBA_INT16 \
" \n\
sfpvec4 load_rgba_int16(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(uint(src_data_int16[i_offset.r])) / sfp(65535.f); \n\
    rgb_in.g = sfp(uint(src_data_int16[i_offset.g])) / sfp(65535.f); \n\
    rgb_in.b = sfp(uint(src_data_int16[i_offset.b])) / sfp(65535.f); \n\
    rgb_in.a = sfp(uint(src_data_int16[i_offset.a])) / sfp(65535.f); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGBA_FLOAT16 \
" \n\
sfpvec4 load_rgba_float16(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(src_data_float16[i_offset.r]); \n\
    rgb_in.g = sfp(src_data_float16[i_offset.g]); \n\
    rgb_in.b = sfp(src_data_float16[i_offset.b]); \n\
    rgb_in.a = sfp(src_data_float16[i_offset.a]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGBA_FLOAT32 \
" \n\
sfpvec4 load_rgba_float32(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(src_data_float32[i_offset.r]); \n\
    rgb_in.g = sfp(src_data_float32[i_offset.g]); \n\
    rgb_in.b = sfp(src_data_float32[i_offset.b]); \n\
    rgb_in.a = sfp(src_data_float32[i_offset.a]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_RGBA \
SHADER_LOAD_RGBA_INT8 \
SHADER_LOAD_RGBA_INT16 \
SHADER_LOAD_RGBA_FLOAT16 \
SHADER_LOAD_RGBA_FLOAT32 \
" \n\
sfpvec4 load_rgba(int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_INT8) \n\
        return load_rgba_int8(x, y, w, cstep, format); \n\
    else if (type == DT_INT16) \n\
        return load_rgba_int16(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT16) \n\
        return load_rgba_float16(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        return load_rgba_float32(x, y, w, cstep, format); \n\
    else \n\
        return sfpvec4(0.f); \n\
} \n\
" // 58 lines

// Load data from dst as rgba
#define SHADER_LOAD_DST_RGBA_INT8 \
" \n\
sfpvec4 load_dst_rgba_int8(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(uint(dst_data_int8[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(dst_data_int8[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(dst_data_int8[i_offset.b])) / sfp(255.f); \n\
    rgb_in.a = sfp(uint(dst_data_int8[i_offset.a])) / sfp(255.f); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_DST_RGBA_INT16 \
" \n\
sfpvec4 load_dst_rgba_int16(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(uint(dst_data_int16[i_offset.r])) / sfp(65535.f); \n\
    rgb_in.g = sfp(uint(dst_data_int16[i_offset.g])) / sfp(65535.f); \n\
    rgb_in.b = sfp(uint(dst_data_int16[i_offset.b])) / sfp(65535.f); \n\
    rgb_in.a = sfp(uint(dst_data_int16[i_offset.a])) / sfp(65535.f); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_DST_RGBA_FLOAT16 \
" \n\
sfpvec4 load_dst_rgba_float16(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(dst_data_float16[i_offset.r]); \n\
    rgb_in.g = sfp(dst_data_float16[i_offset.g]); \n\
    rgb_in.b = sfp(dst_data_float16[i_offset.b]); \n\
    rgb_in.a = sfp(dst_data_float16[i_offset.a]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_DST_RGBA_FLOAT32 \
" \n\
sfpvec4 load_dst_rgba_float32(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp(dst_data_float32[i_offset.r]); \n\
    rgb_in.g = sfp(dst_data_float32[i_offset.g]); \n\
    rgb_in.b = sfp(dst_data_float32[i_offset.b]); \n\
    rgb_in.a = sfp(dst_data_float32[i_offset.a]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_DST_RGBA \
SHADER_LOAD_DST_RGBA_INT8 \
SHADER_LOAD_DST_RGBA_INT16 \
SHADER_LOAD_DST_RGBA_FLOAT16 \
SHADER_LOAD_DST_RGBA_FLOAT32 \
" \n\
sfpvec4 load_dst_rgba(int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_INT8) \n\
        return load_dst_rgba_int8(x, y, w, cstep, format); \n\
    else if (type == DT_INT16) \n\
        return load_dst_rgba_int16(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT16) \n\
        return load_dst_rgba_float16(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        return load_dst_rgba_float32(x, y, w, cstep, format); \n\
    else \n\
        return sfpvec4(0.f); \n\
} \n\
" // 58 lines

// Store data as gray
#define SHADER_STORE_GRAY_INT8 \
" \n\
void store_gray_int8(sfp val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    dst_data_int8[o_offset.x] = uint8_t(clamp(uint(floor(val * sfp(255.0f))), 0, 255)); \n\
} \n\
"

#define SHADER_STORE_GRAY_INT16 \
" \n\
void store_gray_int16(sfp val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    dst_data_int16[o_offset.x] = uint16_t(clamp(uint(floor(val * sfp(65535.0f))), 0, 65535)); \n\
} \n\
"

#define SHADER_STORE_GRAY_FLOAT16 \
" \n\
void store_gray_float16(sfp val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    dst_data_float16[o_offset.x] = float16_t(clamp(val, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_GRAY_FLOAT32 \
" \n\
void store_gray_float32(sfp val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    dst_data_float32[o_offset.x] = float(clamp(val, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_GRAY \
SHADER_STORE_GRAY_INT8 \
SHADER_STORE_GRAY_INT16 \
SHADER_STORE_GRAY_FLOAT16 \
SHADER_STORE_GRAY_FLOAT32 \
" \n\
void store_gray(sfp val, int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_INT8) \n\
        store_gray_int8(val, x, y, w, cstep, format); \n\
    else if (type == DT_INT16) \n\
        store_gray_int16(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT16) \n\
        store_gray_float16(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        store_gray_float32(val, x, y, w, cstep, format); \n\
} \n\
" // 36 lines

// Store data as rgb
#define SHADER_STORE_RGB_INT8 \
" \n\
void store_rgb_int8(sfpvec3 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    dst_data_int8[o_offset.r] = uint8_t(clamp(uint(floor(val.r * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.g] = uint8_t(clamp(uint(floor(val.g * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.b] = uint8_t(clamp(uint(floor(val.b * sfp(255.0f))), 0, 255)); \n\
} \n\
"

#define SHADER_STORE_RGB_INT16 \
" \n\
void store_rgb_int16(sfpvec3 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    dst_data_int16[o_offset.r] = uint16_t(clamp(uint(floor(val.r * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.g] = uint16_t(clamp(uint(floor(val.g * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.b] = uint16_t(clamp(uint(floor(val.b * sfp(65535.0f))), 0, 65535)); \n\
} \n\
"

#define SHADER_STORE_RGB_FLOAT16 \
" \n\
void store_rgb_float16(sfpvec3 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    dst_data_float16[o_offset.r] = float16_t(clamp(val.r, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.g] = float16_t(clamp(val.g, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.b] = float16_t(clamp(val.b, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_RGB_FLOAT32 \
" \n\
void store_rgb_float32(sfpvec3 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    dst_data_float32[o_offset.r] = float(clamp(val.r, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.g] = float(clamp(val.g, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.b] = float(clamp(val.b, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_RGB \
SHADER_STORE_RGB_INT8 \
SHADER_STORE_RGB_INT16 \
SHADER_STORE_RGB_FLOAT16 \
SHADER_STORE_RGB_FLOAT32 \
" \n\
void store_rgb(sfpvec3 val, int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_INT8) \n\
        store_rgb_int8(val, x, y, w, cstep, format); \n\
    else if (type == DT_INT16) \n\
        store_rgb_int16(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT16) \n\
        store_rgb_float16(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        store_rgb_float32(val, x, y, w, cstep, format); \n\
} \n\
" // 44 lines

// Store data as rgba
#define SHADER_STORE_RGBA_INT8 \
" \n\
void store_rgba_int8(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_int8[o_offset.r] = uint8_t(clamp(uint(floor(val.r * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.g] = uint8_t(clamp(uint(floor(val.g * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.b] = uint8_t(clamp(uint(floor(val.b * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.a] = uint8_t(clamp(uint(floor(val.a * sfp(255.0f))), 0, 255)); \n\
} \n\
"

#define SHADER_STORE_RGBA_INT16 \
" \n\
void store_rgba_int16(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_int16[o_offset.r] = uint16_t(clamp(uint(floor(val.r * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.g] = uint16_t(clamp(uint(floor(val.g * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.b] = uint16_t(clamp(uint(floor(val.b * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.a] = uint16_t(clamp(uint(floor(val.a * sfp(65535.0f))), 0, 65535)); \n\
} \n\
"

#define SHADER_STORE_RGBA_FLOAT16 \
" \n\
void store_rgba_float16(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_float16[o_offset.r] = float16_t(clamp(val.r, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.g] = float16_t(clamp(val.g, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.b] = float16_t(clamp(val.b, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.a] = float16_t(clamp(val.a, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_RGBA_FLOAT32 \
" \n\
void store_rgba_float32(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_float32[o_offset.r] = float(clamp(val.r, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.g] = float(clamp(val.g, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.b] = float(clamp(val.b, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.a] = float(clamp(val.a, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_RGBA \
SHADER_STORE_RGBA_INT8 \
SHADER_STORE_RGBA_INT16 \
SHADER_STORE_RGBA_FLOAT16 \
SHADER_STORE_RGBA_FLOAT32 \
" \n\
void store_rgba(sfpvec4 val, int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_INT8) \n\
        store_rgba_int8(val, x, y, w, cstep, format); \n\
    else if (type == DT_INT16) \n\
        store_rgba_int16(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT16) \n\
        store_rgba_float16(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        store_rgba_float32(val, x, y, w, cstep, format); \n\
} \n\
" // 48 lines

// Store data to RGBA Side By Side
#define SHADER_STORE_RGBA_INT8_SIDE_BY_SIDE \
" \n\
void store_rgba_int8_side_by_side(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    int planner = x % 2 == 0 ? x / 2 : x / 2 + p.w / 2; \n\
    ivec4 o_offset = (y * w + planner) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_int8[o_offset.r] = uint8_t(clamp(uint(floor(val.r * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.g] = uint8_t(clamp(uint(floor(val.g * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.b] = uint8_t(clamp(uint(floor(val.b * sfp(255.0f))), 0, 255)); \n\
    dst_data_int8[o_offset.a] = uint8_t(clamp(uint(floor(val.a * sfp(255.0f))), 0, 255)); \n\
} \n\
"

#define SHADER_STORE_RGBA_INT16_SIDE_BY_SIDE \
" \n\
void store_rgba_int16_side_by_side(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    int planner = x % 2 == 0 ? x / 2 : x / 2 + p.w / 2; \n\
    ivec4 o_offset = (y * w + planner) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_int16[o_offset.r] = uint16_t(clamp(uint(floor(val.r * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.g] = uint16_t(clamp(uint(floor(val.g * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.b] = uint16_t(clamp(uint(floor(val.b * sfp(65535.0f))), 0, 65535)); \n\
    dst_data_int16[o_offset.a] = uint16_t(clamp(uint(floor(val.a * sfp(65535.0f))), 0, 65535)); \n\
} \n\
"

#define SHADER_STORE_RGBA_FLOAT16_SIDE_BY_SIDE \
" \n\
void store_rgba_float16_side_by_side(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    int planner = x % 2 == 0 ? x / 2 : x / 2 + p.w / 2; \n\
    ivec4 o_offset = (y * w + planner) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_float16[o_offset.r] = float16_t(clamp(val.r, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.g] = float16_t(clamp(val.g, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.b] = float16_t(clamp(val.b, sfp(0.f), sfp(1.f))); \n\
    dst_data_float16[o_offset.a] = float16_t(clamp(val.a, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_RGBA_FLOAT32_SIDE_BY_SIDE \
" \n\
void store_rgba_float32_side_by_side(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    int planner = x % 2 == 0 ? x / 2 : x / 2 + p.w / 2; \n\
    ivec4 o_offset = (y * w + planner) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    dst_data_float32[o_offset.r] = float(clamp(val.r, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.g] = float(clamp(val.g, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.b] = float(clamp(val.b, sfp(0.f), sfp(1.f))); \n\
    dst_data_float32[o_offset.a] = float(clamp(val.a, sfp(0.f), sfp(1.f))); \n\
} \n\
"

#define SHADER_STORE_RGBA_SIDE_BY_SIDE \
SHADER_STORE_RGBA_INT8_SIDE_BY_SIDE \
SHADER_STORE_RGBA_INT16_SIDE_BY_SIDE \
SHADER_STORE_RGBA_FLOAT16_SIDE_BY_SIDE \
SHADER_STORE_RGBA_FLOAT32_SIDE_BY_SIDE \
" \n\
void store_rgba_side_by_side(sfpvec4 val, int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_INT8) \n\
        store_rgba_int8_side_by_side(val, x, y, w, cstep, format); \n\
    else if (type == DT_INT16) \n\
        store_rgba_int16_side_by_side(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT16) \n\
        store_rgba_float16_side_by_side(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        store_rgba_float32_side_by_side(val, x, y, w, cstep, format); \n\
} \n\
" // 52 lines

// Store data as float rgb without clamp
#define SHADER_STORE_RGB_FLOAT16_NO_CLAMP \
" \n\
void store_rgb_float16_no_clamp(sfpvec3 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    dst_data_float16[o_offset.r] = float16_t(val.r); \n\
    dst_data_float16[o_offset.g] = float16_t(val.g); \n\
    dst_data_float16[o_offset.b] = float16_t(val.b); \n\
} \
"

#define SHADER_STORE_RGB_FLOAT32_NO_CLAMP \
" \n\
void store_rgb_float32_no_clamp(sfpvec3 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec3 o_offset = (y * w + x) * cstep + (format == CF_BGR ? ivec3(0, 1, 2) : ivec3(2, 1, 0)); \n\
    dst_data_float32[o_offset.r] = float(val.r); \n\
    dst_data_float32[o_offset.g] = float(val.g); \n\
    dst_data_float32[o_offset.b] = float(val.b); \n\
} \n\
"

#define SHADER_STORE_RGB_FLOAT_NO_CLAMP \
SHADER_STORE_RGB_FLOAT16_NO_CLAMP \
SHADER_STORE_RGB_FLOAT32_NO_CLAMP \
" \n\
void store_rgb_float_no_clamp(sfpvec3 val, int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_FLOAT16) \n\
        store_rgb_float16_no_clamp(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        store_rgb_float32_no_clamp(val, x, y, w, cstep, format); \n\
} \n\
" // 44 lines

// Store data as float rgba without clamp
#define SHADER_STORE_RGBA_FLOAT16_NO_CLAMP(dst) \
" \n\
void store_rgba_float16_no_clamp_"#dst"(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    "#dst"_data_float16[o_offset.r] = float16_t(val.r); \n\
    "#dst"_data_float16[o_offset.g] = float16_t(val.g); \n\
    "#dst"_data_float16[o_offset.b] = float16_t(val.b); \n\
    "#dst"_data_float16[o_offset.a] = float16_t(val.a); \n\
} \n\
"

#define SHADER_STORE_RGBA_FLOAT32_NO_CLAMP(dst) \
" \n\
void store_rgba_float32_no_clamp_"#dst"(sfpvec4 val, int x, int y, int w, int cstep, int format) \n\
{ \n\
    ivec4 o_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    "#dst"_data_float32[o_offset.r] = float(val.r); \n\
    "#dst"_data_float32[o_offset.g] = float(val.g); \n\
    "#dst"_data_float32[o_offset.b] = float(val.b); \n\
    "#dst"_data_float32[o_offset.a] = float(val.a); \n\
} \n\
"

#define SHADER_STORE_RGBA_FLOAT_NO_CLAMP(dst) \
SHADER_STORE_RGBA_FLOAT16_NO_CLAMP(dst) \
SHADER_STORE_RGBA_FLOAT32_NO_CLAMP(dst) \
" \n\
void store_rgba_float_no_clamp_"#dst"(sfpvec4 val, int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_FLOAT16) \n\
        store_rgba_float16_no_clamp_"#dst"(val, x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        store_rgba_float32_no_clamp_"#dst"(val, x, y, w, cstep, format); \n\
} \n\
" // 46 lines


#define SHADER_LOAD_FLOAT16_RGBA(src) \
" \n\
sfpvec4 load_float16_rgba_"#src"(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp("#src"_data[i_offset.r]); \n\
    rgb_in.g = sfp("#src"_data[i_offset.g]); \n\
    rgb_in.b = sfp("#src"_data[i_offset.b]); \n\
    rgb_in.a = sfp("#src"_data[i_offset.a]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_FLOAT32_RGBA(src) \
" \n\
sfpvec4 load_float32_rgba_"#src"(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0.f); \n\
    ivec4 i_offset = (y * w + x) * cstep + (format == CF_ABGR ? ivec4(0, 1, 2, 3) : ivec4(2, 1, 0, 3)); \n\
    rgb_in.r = sfp("#src"_data[i_offset.r]); \n\
    rgb_in.g = sfp("#src"_data[i_offset.g]); \n\
    rgb_in.b = sfp("#src"_data[i_offset.b]); \n\
    rgb_in.a = sfp("#src"_data[i_offset.a]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_FLOAT_RGBA(src) \
SHADER_LOAD_FLOAT16_RGBA(src) \
SHADER_LOAD_FLOAT32_RGBA(src) \
" \n\
sfpvec4 load_float_rgba_"#src"(int x, int y, int w, int cstep, int format, int type) \n\
{ \n\
    if (type == DT_FLOAT16) \n\
        return load_float16_rgba_"#src"(x, y, w, cstep, format); \n\
    else if (type == DT_FLOAT32) \n\
        return load_float32_rgba_"#src"(x, y, w, cstep, format); \n\
    else \n\
        return sfpvec4(0.f); \n\
} \n\
" // 46 lines

#define SHADER_LOAD_FLOAT16_GRAY(src) \
" \n\
sfp load_float16_gray_"#src"(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfp rgb_in = sfp(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    rgb_in = sfp("#src"_data_float16[i_offset.x]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_LOAD_FLOAT32_GRAY(src) \
" \n\
sfp load_float32_gray_"#src"(int x, int y, int w, int cstep, int format) \n\
{ \n\
    sfp rgb_in = sfp(0.f); \n\
    ivec3 i_offset = (y * w + x) * cstep + ivec3(0, 0, 0); \n\
    rgb_in = sfp("#src"_data_float32[i_offset.x]); \n\
    return rgb_in; \n\
} \n\
"

#define SHADER_DEFAULT_PARAM \
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
    \n\
    float param_x; \n\
    float param_y; \n\
    float param_z; \n\
    float param_w; \n\
} p; \n\
"
