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
    // 1 控制亮度的强度系数 \n\
    float lumaMask; \n\
    float chromaColorX; \n\
    float chromaColorY; \n\
    float chromaColorZ; \n\
    // 用环境光补受蓝绿幕影响的像素(简单理解扣像结果要放入的环境光的颜色) \n\
    float ambientScale; \n\
    float ambientColorX; \n\
    float ambientColorY; \n\
    float ambientColorZ; \n\
    float alphaCutoffMin; \n\
    float alphaScale; \n\
    float alphaExponent; \n\
    float despillScale; \n\
    float despillExponent; \n\
} p; \
"

#define SHADER_MAIN \
" \n\
const sfp PI = sfp(3.1415926f); \n\
sfpvec3 extractColor(sfpvec3 color, sfp lumaMask) \n\
{ \n\
    sfp luma = dot(color, sfpvec3(1.0f)); \n\
    // 亮度指数 \n\
    sfp colorMask = exp(-luma * 2 * PI / lumaMask); \n\
    // color*(1-colorMask)+color*luma \n\
    color = mix(color, sfpvec3(luma), colorMask); \n\
    // 生成基于亮度的饱和度图 \n\
    return color / dot(color, sfpvec3(2.0)); \n\
} \n\
void main() \n\
{ \n\
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n\
    if (uv.x >= p.w || uv.y >= p.h) \n\
        return; \n\
    sfpvec3 inputColor = load_float_rgba(uv.x, uv.y, p.w, p.cstep, p.format).rgb; \n\
    sfpvec3 chromaColor = sfpvec3(p.chromaColorX, p.chromaColorY, p.chromaColorZ); \n\
    sfpvec3 ambientColor = sfpvec3(p.ambientColorX, p.ambientColorY, p.ambientColorZ); \n\
    sfpvec3 color1 = extractColor(chromaColor, sfp(p.lumaMask)); \n\
    sfpvec3 color2 = extractColor(inputColor, sfp(p.lumaMask)); \n\
    sfpvec3 subColor = color1 - color2; \n\
    sfp diffSize = length(subColor); \n\
    sfp minClamp = max((diffSize - sfp(p.alphaCutoffMin)) * sfp(p.alphaScale), sfp(0.0f)); \n\
    // 扣像alpha \n\
    sfp alpha= clamp(pow(minClamp, sfp(p.alphaExponent)), sfp(0.0f), sfp(1.0f)); \n\
    // 受扣像背景影响的颜色alpha \n\
    sfp despillAlpha = sfp(1.0f) - clamp(pow(minClamp, sfp(p.despillExponent)), sfp(0.0f), sfp(1.0f)); \n\
    // 亮度系数 \n\
    sfpvec3 lumaFactor = sfpvec3(0.3f,0.59f,0.11f); \n\
    sfpvec3 dcolor = inputColor; \n\
    // 去除扣像背景影响的颜色 \n\
    dcolor -= inputColor * chromaColor * despillAlpha * sfp(p.despillScale); \n\
    // 添加环境光收益 \n\
    dcolor += inputColor * lumaFactor*ambientColor * sfp(p.ambientScale) * despillAlpha; \n\
    store_float_rgba(sfpvec4(dcolor, alpha), uv.x, uv.y, p.w, p.cstep, p.format); \n\
} \
"


static const char Filter_data[] = 
SHADER_HEADER
R"(
layout (binding = 0) readonly buffer src_float { float src_float_data[]; };
layout (binding = 1) writeonly buffer dst_float { float dst_float_data[]; };
)"
SHADER_PARAM
SHADER_LOAD_FLOAT_RGBA
SHADER_STORE_FLOAT_RGBA
SHADER_MAIN
;
