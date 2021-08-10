#define SHADER_HEADER \
"\
#version 450 \n\
#extension GL_EXT_shader_8bit_storage: require \n\
#extension GL_EXT_shader_16bit_storage: require \n\
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require \n\
#define GRAY    0 \n\
#define BGR     1 \n\
#define ABGR    2 \n\
#define RGB     3 \n\
#define ARGB    4 \n\
#define YUV420  5 \n\
#define YUV422  6 \n\
#define YUV444  7 \n\
#define YUVA    8 \n\
#define NV12    9 \n\
\
"

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
    float strength; \n\
} p; \
"

#define SHADER_LOAD_SRC_RGB \
" \n\
sfpvec3 load_src_rgb(int x, int y, int z) \n\
{ \n\
    sfpvec3 rgb_in = {0.f, 0.f, 0.f}; \n\
    ivec4 i_offset = (y * p.w + x) * p.cstep + (p.format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(src_int8_data[i_offset.r])); \n\
    rgb_in.g = sfp(uint(src_int8_data[i_offset.g])); \n\
    rgb_in.b = sfp(uint(src_int8_data[i_offset.b])); \n\
    return rgb_in; \n\
} \
"

#define SHADER_STORE_DST_RGB \
" \n\
void store_dst_rgb(int x, int y, int z, sfpvec3 rgb) \n\
{ \n\
    ivec4 o_offset = (y * p.w + x) * p.cstep + (p.format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    dst_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r)), 0, 255)); \n\
    dst_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g)), 0, 255)); \n\
    dst_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b)), 0, 255)); \n\
    dst_int8_data[o_offset.a] = uint8_t(255); \n\
} \
"

#define SHADER_MAIN \
" \n\
sfpmat3 matrix_mat_r2y = { \n\
    { sfp( 0.262700f), sfp(-0.139630f), sfp( 0.500000f)}, \n\
    { sfp( 0.678000f), sfp(-0.360370f), sfp(-0.459786f)}, \n\
    { sfp( 0.059300f), sfp( 0.500000f), sfp(-0.040214f)} \n\
}; \n\
sfpvec3 rgb_to_yuv(sfpvec3 rgb) \n\
{ \n\
    sfpvec3 yuv_offset = {sfp(0.f), sfp(0.5f), sfp(0.5f)}; \n\
    sfpvec3 yuv = yuv_offset + matrix_mat_r2y * rgb; \n\
    return yuv; \n\
} \n\
const float horizontKernel[9] = \n\
{ \n\
    -1.0f,-2.0f,-1.0f, \n\
     0.0f, 0.0f, 0.0f, \n\
     1.0f, 2.0f, 1.0f  \n\
}; \n\
const float verticalKernel[9] = \n\
{ \n\
    -1.0f, 0.0f, 1.0f, \n\
    -2.0f, 0.0f, 2.0f, \n\
    -1.0f, 0.0f, 1.0f  \n\
}; \n\
void main() \n\
{ \n\
    int gx = int(gl_GlobalInvocationID.x); \n\
    int gy = int(gl_GlobalInvocationID.y); \n\
    int gz = int(gl_GlobalInvocationID.z); \n\
    if (gx >= p.w || gy >= p.h || gz >= 3) \n\
        return; \n\
    sfp vertical = 0.0f; \n\
    sfp horizont = 0.0f; \n\
    for (int i = 0; i < 3; ++i) \n\
    { \n\
        for (int j = 0; j < 3; ++j) \n\
        { \n\
            int x = gx - 1 + j; \n\
            int y = gy - 1 + i; \n\
            // REPLICATE border \n\
            x = max(0, min(x, p.w - 1)); \n\
            y = max(0, min(y, p.h - 1)); \n\
            int index = j + i * 3; \n\
            sfpvec3 value = rgb_to_yuv(load_src_rgb(x, y, gz)); \n\
            vertical += value.x * sfp(verticalKernel[index]); \n\
            horizont += value.x * sfp(horizontKernel[index]); \n\
        } \n\
    } \n\
    sfp mag = length(sfpvec2(horizont, vertical)) * sfp(p.strength); \n\
    store_dst_rgb(gx, gy, gz, sfpvec3(mag)); \n\
} \
"

static const char Filter_data[] = 
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
