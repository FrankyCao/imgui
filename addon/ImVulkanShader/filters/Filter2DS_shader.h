#pragma once
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
const int PATCH_PER_BLOCK = 4;\n\
const int HALO_SIZE = 1; \n\
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
	int xksize;	\n\
    int yksize;	\n\
    int xanchor; \n\
    int yanchor; \n\
} p; \
"


#define SHADER_LOAD_SRC_RGB \
" \n\
sfpvec4 load_src_rgb(int x, int y) \n\
{ \n\
    sfpvec4 rgb_in = sfpvec4(0); \n\
    ivec4 i_offset = (y * p.w + x) * p.cstep + (p.format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    rgb_in.r = sfp(uint(src_int8_data[i_offset.r])) / sfp(255.f); \n\
    rgb_in.g = sfp(uint(src_int8_data[i_offset.g])) / sfp(255.f); \n\
    rgb_in.b = sfp(uint(src_int8_data[i_offset.b])) / sfp(255.f); \n\
    rgb_in.a = sfp(1.f); \n\
    return rgb_in; \n\
} \
"

#define SHADER_STORE_DST_RGB \
" \n\
void store_dst_rgb(int x, int y, sfpvec4 rgb) \n\
{ \n\
    ivec4 o_offset = (y * p.w + x) * p.cstep + (p.format == ABGR ? ivec4(0, 1, 2, 3) : ivec4(0, 3, 2, 1)); \n\
    dst_int8_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.f))), 0, 255)); \n\
    dst_int8_data[o_offset.a] = uint8_t(255); \n\
} \
"

#define SHADER_FILTER_COLUMN_MAIN \
" \n\
shared sfpvec4 column_shared[16 * (PATCH_PER_BLOCK + HALO_SIZE * 2)][16]; \n\
void main() \n\
{ \n\
    ivec2 size = ivec2(p.w, p.h); \n\
    int x = int(gl_GlobalInvocationID.x); \n\
    if (x >= size.x) \n\
        return; \n\
    int wy = int(gl_WorkGroupID.y); \n\
    int wsy = int(gl_WorkGroupSize.y); \n\
    int lx = int(gl_LocalInvocationID.x); \n\
    int ly = int(gl_LocalInvocationID.y); \n\
    // 纹理范围的全局起点 \n\
    int yStart = wy * (wsy * PATCH_PER_BLOCK) + ly; \n\
    // 填充每个块的上HALO_SIZE,注意每列最前面的那个块取不到纹理数据 \n\
    if (wy > 0) \n\
    { \n\
        // 填充非第一列的上边 \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            sfpvec4 rgba = load_src_rgb(x, yStart - (HALO_SIZE - j) * wsy); \n\
            column_shared[ly + j * wsy][lx] = rgba; \n\
        } \n\
    } \n\
    else \n\
    { \n\
        // 每列最上边 \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            int maxIdy = max(0, yStart - (HALO_SIZE - j) * wsy); \n\
            sfpvec4 rgba = load_src_rgb(x, maxIdy); \n\
            column_shared[ly + j * wsy][lx] = rgba; \n\
        } \n\
    } \n\
    // 填充正常加下边扩展的块,注意每列最后面的那个块取不到下HALO_SIZE块纹理数据 \n\
    if (wy + 2 < gl_NumWorkGroups.y) \n\
    { \n\
        // 主要导入的数据,一个线程取行上四个位置数据 \n\
        for (int j = 0; j < PATCH_PER_BLOCK; ++j) \n\
        { \n\
            sfpvec4 rgba = load_src_rgb(x, yStart + j * wsy); \n\
            int y = ly + (HALO_SIZE + j) * wsy; \n\
            column_shared[y][lx] = rgba; \n\
        } \n\
        // 下边的扩展中,还在纹理中 \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            sfpvec4 rgba = load_src_rgb(x, yStart + (PATCH_PER_BLOCK + j) * wsy); \n\
            int y = ly + (PATCH_PER_BLOCK + HALO_SIZE + j) * wsy; \n\
            column_shared[y][lx] = rgba; \n\
        } \n\
    } \n\
    else \n\
    { \n\
        // 每列最后边的一个块 \n\
        for (int j = 0; j < PATCH_PER_BLOCK; ++j) \n\
        { \n\
            int minIdy = min(size.y - 1, yStart + j * wsy); \n\
            int y = ly + (HALO_SIZE + j) * wsy; \n\
            sfpvec4 rgba = load_src_rgb(x, minIdy); \n\
            column_shared[y][lx] = rgba; \n\
        } \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            int minIdy = min(size.y - 1, yStart + (PATCH_PER_BLOCK + j) * wsy); \n\
            int y = ly + (PATCH_PER_BLOCK+HALO_SIZE + j) * wsy; \n\
            sfpvec4 rgba = load_src_rgb(x, minIdy); \n\
            column_shared[y][lx] = rgba; \n\
        } \n\
    } \n\
	memoryBarrierShared(); \n\
	barrier(); \n\
    for (int j = 0; j < PATCH_PER_BLOCK; ++j) \n\
    { \n\
        int y = yStart + j * wsy; \n\
        if (y < size.y) \n\
        { \n\
            sfpvec4 sum = sfpvec4(0); \n\
            for (int k = 0; k < p.yksize; ++k) \n\
            { \n\
                int yy = ly + (HALO_SIZE + j) * wsy - p.yanchor + k; \n\
                sum = sum + column_shared[yy][lx] * sfp(kernel_data[k]); \n\
            } \n\
            store_dst_rgb(x, y ,sum); \n\
        } \n\
    } \n\
} \
"

#define SHADER_FILTER_ROW_MAIN \
" \n\
shared sfpvec4 row_shared[16][16 * (PATCH_PER_BLOCK + HALO_SIZE * 2)]; \n\
void main() \n\
{ \n\
    ivec2 size = ivec2(p.w, p.h); \n\
    int y = int(min(gl_GlobalInvocationID.y, size.y - 1)); \n\
    // 纹理正常范围的全局起点 \n\
    int wx = int(gl_WorkGroupID.x); \n\
    int wsx = int(gl_WorkGroupSize.x); \n\
    int lx = int(gl_LocalInvocationID.x); \n\
    int ly = int(gl_LocalInvocationID.y); \n\
    int xStart = wx * (wsx * PATCH_PER_BLOCK) + lx; \n\
    // 每个线程组填充HALO_SIZE*gl_WorkGroupSize个数据 \n\
    // 填充每个左边HALO_SIZE,需要注意每行左边是没有纹理数据的 \n\
    if (wx > 0) \n\
    { \n\
        //填充非最左边块的左边 \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            sfpvec4 rgba = load_src_rgb(xStart - (HALO_SIZE - j) * wsx, y); \n\
            row_shared[ly][lx + j * wsx] = rgba; \n\
        } \n\
    } \n\
    else \n\
    { \n\
        // 每行最左边 \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            int maxIdx = max(0, xStart - (HALO_SIZE - j) * wsx); \n\
            sfpvec4 rgba = load_src_rgb(maxIdx, y); \n\
            row_shared[ly][lx + j * wsx] = rgba; \n\
        } \n\
    } \n\
    // 填充中间与右边HALO_SIZE块,注意每行右边的HALO_SIZE块是没有纹理数据的 \n\
    if (wx + 2 < gl_NumWorkGroups.x) \n\
    { \n\
        // 填充中间块 \n\
        for (int j = 0; j < PATCH_PER_BLOCK; ++j) \n\
        { \n\
            sfpvec4 rgba = load_src_rgb(xStart + j * wsx, y); \n\
            int x = lx + (HALO_SIZE + j) * wsx; \n\
            row_shared[ly][x] = rgba; \n\
        } \n\
        // 右边的扩展中,还在纹理中 \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            sfpvec4 rgba = load_src_rgb(xStart + (PATCH_PER_BLOCK + j) * wsx, y); \n\
            int x = lx + (PATCH_PER_BLOCK+HALO_SIZE + j) * wsx; \n\
            row_shared[ly][x] = rgba; \n\
        } \n\
    } \n\
    else \n\
    { \n\
        // 每行右边的一个块 \n\
        for (int j = 0; j < PATCH_PER_BLOCK; ++j) \n\
        { \n\
            int minIdx = min(size.x - 1, xStart + j * wsx); \n\
            int x = lx + (HALO_SIZE + j) * wsx; \n\
            sfpvec4 rgba = load_src_rgb(minIdx, y); \n\
            row_shared[ly][x] = rgba; \n\
        } \n\
        for (int j = 0; j < HALO_SIZE; ++j) \n\
        { \n\
            int minIdx = min(size.x - 1, xStart + (PATCH_PER_BLOCK + j) * wsx); \n\
            int x = lx + (PATCH_PER_BLOCK + HALO_SIZE + j) * wsx; \n\
            sfpvec4 rgba = load_src_rgb(minIdx, y); \n\
            row_shared[ly][x] = rgba; \n\
        } \n\
    } \n\
	memoryBarrierShared(); \n\
	barrier(); \n\
    for (int j = 0; j < PATCH_PER_BLOCK; ++j) \n\
    { \n\
        int x = xStart + j * wsx; \n\
        if (x < size.x && gl_GlobalInvocationID.y < size.y) \n\
        { \n\
            sfpvec4 sum = sfpvec4(0); \n\
            for (int k = 0; k < p.xksize; ++k) \n\
            { \n\
                int xx = lx + (HALO_SIZE + j) * wsx - p.xanchor + k; \n\
                sum = sum + row_shared[ly][xx] * sfp(kernel_data[k]); \n\
            } \n\
            store_dst_rgb(x, y, sum); \n\
        } \n\
    } \n\
} \
"

static const char FilterColumn_data[] = 
SHADER_HEADER
R"(
layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0) readonly buffer src_int8 { uint8_t src_int8_data[]; };
layout (binding = 1) writeonly buffer dst_int8 { uint8_t dst_int8_data[]; };
layout (binding = 2) readonly buffer kernel_float { float kernel_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_SRC_RGB
SHADER_STORE_DST_RGB
SHADER_FILTER_COLUMN_MAIN
;

static const char FilterRow_data[] = 
SHADER_HEADER
R"(
layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0) readonly buffer src_int8 { uint8_t src_int8_data[]; };
layout (binding = 1) writeonly buffer dst_int8 { uint8_t dst_int8_data[]; };
layout (binding = 2) readonly buffer kernel_float { float kernel_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_SRC_RGB
SHADER_STORE_DST_RGB
SHADER_FILTER_ROW_MAIN
;
