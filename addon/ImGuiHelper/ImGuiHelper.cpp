//- Common Code For All Addons needed just to ease inclusion as separate files in user code ----------------------
#include <imgui.h>
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
//-----------------------------------------------------------------------------------------------------------------

#include "ImGuiHelper.h"
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>	// ShellExecuteA(...) - Shell32.lib
#include <objbase.h>    // CoInitializeEx(...)  - ole32.lib
#if defined(IMGUI_RENDERING_DX11)
struct IUnknown;
#include <d3d11.h>
#elif defined(IMGUI_RENDERING_DX9)
#include <d3d9.h>
#endif
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#else //_WIN32
#include <unistd.h>
#include <stdlib.h> // system
#endif //_WIN32

#include <vector>
#include <algorithm>

#include <imgui_internal.h>
#ifdef IMGUI_VULKAN_SHADER
#include "ImVulkanShader.h"
#endif
#ifdef IMGUI_OPENGL
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#if defined(IMGUI_RENDERING_GL3)
#include <imgui_impl_opengl3.h>
#elif defined(IMGUI_RENDERING_GL2)
#include <imgui_impl_opengl2.h>
#endif
#endif

#ifndef NO_IMGUIHELPER_DRAW_METHODS
#if !defined(alloca)
#	if defined(__GLIBC__) || defined(__sun) || defined(__APPLE__) || defined(__NEWLIB__)
#		include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#	elif defined(_WIN32)
#       include <malloc.h>     // alloca
#       if !defined(alloca)
#           define alloca _alloca  // for clang with MS Codegen
#       endif //alloca
#   elif defined(__GLIBC__) || defined(__sun)
#       include <alloca.h>     // alloca
#   else
#       include <stdlib.h>     // alloca
#   endif
#endif //alloca
#endif //NO_IMGUIHELPER_DRAW_METHODS

#ifdef IMGUI_RENDERING_VULKAN
#include <imgui_impl_vulkan.h>
#endif

#if     defined(IMGUI_RENDERING_VULKAN)
struct ImTexture
{
    ImTextureVk TextureID = nullptr;
    int     Width     = 0;
    int     Height    = 0;
};
#elif   defined(IMGUI_RENDERING_DX11)
extern ID3D11Device* g_pd3dDevice;
struct ImTexture
{
    ID3D11ShaderResourceView * TextureID = nullptr;
    int    Width     = 0;
    int    Height    = 0;
};
#elif defined(IMGUI_RENDERING_DX9)
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
struct ImTexture
{
    LPDIRECT3DTEXTURE9 TextureID = nullptr;
    int    Width     = 0;
    int    Height    = 0;
};
#elif   defined(IMGUI_OPENGL)
struct ImTexture
{
    GLuint TextureID = 0;
    int    Width     = 0;
    int    Height    = 0;
};
#else
struct ImTexture
{
    int    TextureID = -1;
    int    Width     = 0;
    int    Height    = 0;
};
#endif

namespace ImGui {

// ImGui Info
void ShowImGuiInfo()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    ImGui::Text("define: __cplusplus = %d", (int)__cplusplus);
    ImGui::Separator();
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
    ImGui::Text("define: IMGUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
    ImGui::Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
    ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
    ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
    ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
    ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
    ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
    ImGui::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
    ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
    ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
    ImGui::Text("define: __GNUC__ = %d", (int)__GNUC__);
#endif
#ifdef __clang_version__
    ImGui::Text("define: __clang_version__ = %s", __clang_version__);
#endif
    ImGui::Separator();
    ImGui::Text("Backend Platform Name: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
    ImGui::Text("Backend Renderer Name: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
#ifdef IMGUI_RENDERING_VULKAN
    ImGui::Text("Backend GPU: %s", ImGui_ImplVulkan_GetDeviceName().c_str());
    ImGui::Text("Backend Vulkan API: %s", ImGui_ImplVulkan_GetApiVersion().c_str());
    ImGui::Text("Backend Vulkan Drv: %s", ImGui_ImplVulkan_GetDrvVersion().c_str());
    ImGui::Separator();
#elif defined(IMGUI_OPENGL)
#if defined(IMGUI_RENDERING_GL3)
    ImGui::Text("Gl Loader: %s", ImGui_ImplOpenGL3_GLLoaderName().c_str());
    ImGui::Text("GL Version: %s", ImGui_ImplOpenGL3_GetVerion().c_str());
#elif defined(IMGUI_RENDERING_GL2)
    ImGui::Text("Gl Loader: %s", ImGui_ImplOpenGL2_GLLoaderName().c_str());
    ImGui::Text("GL Version: %s", ImGui_ImplOpenGL2_GetVerion().c_str());
#endif
#endif
#ifdef IMGUI_VULKAN_SHADER
    ImGui::Text("Vulkan Shader:");
    int count = ImVulkan::get_gpu_count();
    for (int i = 0; i < count; i++)
    {
        const ImVulkan::GpuInfo& info = ImVulkan::get_gpu_info(i);
        ImGui::Text("  %s:", info.device_name());
        ImGui::Text("    bugs: sbn1=%d bilz=%d copc=%d ihfa=%d si=%d", 
                    info.bug_storage_buffer_no_l1(), info.bug_buffer_image_load_zero(), info.bug_corrupted_online_pipeline_cache(), info.bug_implicit_fp16_arithmetic(), info.bug_storage_image()); 
        ImGui::Text("    fp16: p=%d/s=%d/a=%d int8: p=%d/s=%d/a=%d", 
                    info.support_fp16_packed(), info.support_fp16_storage(), info.support_fp16_arithmetic(), 
                    info.support_int8_packed(), info.support_int8_storage(), info.support_int8_arithmetic());
        ImGui::Text("      sg: subgroup=%u basic=%d vote=%d ballot=%d shuffle=%d",
                    info.subgroup_size(), info.support_subgroup_basic(), info.support_subgroup_vote(),
                    info.support_subgroup_ballot(), info.support_subgroup_shuffle());
    }
    ImGui::Separator();
#endif
    ImGui::Text("Flash Timer: %.1f", io.ConfigMemoryCompactTimer >= 0.0f ? io.ConfigMemoryCompactTimer : 0);
    ImGui::Separator();
    ImGui::Text("Fonts: %d fonts", io.Fonts->Fonts.Size);
    ImGui::Text("Texure Size: %d x %d", io.Fonts->TexWidth, io.Fonts->TexHeight); 
    ImGui::Text("Display Size: %.2f x %.2f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::Text("Display Framebuffer Scale: %.2f %.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
}
// Image Load
static std::vector<ImTexture> g_Textures;

void ImGenerateOrUpdateTexture(ImTextureID& imtexid,int width,int height,int channels,const unsigned char* pixels,bool useMipmapsIfPossible,bool wraps,bool wrapt,bool minFilterNearest,bool magFilterNearest)
{
    IM_ASSERT(pixels);
    IM_ASSERT(channels>0 && channels<=4);
#if     defined(IMGUI_RENDERING_VULKAN)
    if (imtexid == 0)
    {
        g_Textures.resize(g_Textures.size() + 1);
        ImTexture& texture = g_Textures.back();
        texture.TextureID = (ImTextureVk)ImGui_ImplVulkan_CreateTexture(pixels, width, height);
        texture.Width  = width;
        texture.Height = height;
        imtexid = texture.TextureID;
        return;
    }
    ImGui_ImplVulkan_UpdateTexture(imtexid, pixels, width, height);
#elif   defined(IMGUI_RENDERING_DX11)
    auto textureID = (ID3D11ShaderResourceView *)imtexid;
    if (textureID)
    {
        textureID->Release();
        textureID = nullptr;
    }
    imtexid = ImCreateTexture(pixels, width, height);
#elif   defined(IMGUI_RENDERING_DX9)
    LPDIRECT3DTEXTURE9& texid = reinterpret_cast<LPDIRECT3DTEXTURE9&>(imtexid);
    if (texid==0 && g_pd3dDevice->CreateTexture(width, height, useMipmapsIfPossible ? 0 : 1, 0, channels==1 ? D3DFMT_A8 : channels==2 ? D3DFMT_A8L8 : channels==3 ? D3DFMT_R8G8B8 : D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texid, NULL) < 0) return;

    D3DLOCKED_RECT tex_locked_rect;
    if (texid->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK) {texid->Release();texid=0;return;}
    if (channels==3 || channels==4) {
        unsigned char* pw;
        const unsigned char* ppxl = pixels;
        for (int y = 0; y < height; y++)    {
            pw = (unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y;  // each row has Pitch bytes
            ppxl = &pixels[y*width*channels];
            for( int x = 0; x < width; x++ )
            {
                *pw++ = ppxl[2];
                *pw++ = ppxl[1];
                *pw++ = ppxl[0];
                if (channels==4) *pw++ = ppxl[3];
                ppxl+=channels;
            }
        }
    }
    else {
        for (int y = 0; y < height; y++)    {
            memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * channels) * y, (width * channels));
        }
    }
    texid->UnlockRect(0);
#elif   defined(IMGUI_OPENGL)
    glEnable(GL_TEXTURE_2D);
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLuint& texid = reinterpret_cast<GLuint&>(imtexid);
    if (texid==0) 
    {
        glGenTextures(1, &texid);
        g_Textures.resize(g_Textures.size() + 1);
        ImTexture& texture = g_Textures.back();
        texture.TextureID = texid;
        texture.Width  = width;
        texture.Height = height;
    }

    glBindTexture(GL_TEXTURE_2D, texid);

    GLenum clampEnum = 0x2900;    // 0x2900 -> GL_CLAMP; 0x812F -> GL_CLAMP_TO_EDGE
#   ifndef GL_CLAMP
#       ifdef GL_CLAMP_TO_EDGE
        clampEnum = GL_CLAMP_TO_EDGE;
#       else //GL_CLAMP_TO_EDGE
        clampEnum = 0x812F;
#       endif // GL_CLAMP_TO_EDGE
#   else //GL_CLAMP
    clampEnum = GL_CLAMP;
#   endif //GL_CLAMP

    unsigned char* potImageBuffer = NULL;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wraps ? GL_REPEAT : clampEnum);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrapt ? GL_REPEAT : clampEnum);
    //const GLfloat borderColor[]={0.f,0.f,0.f,1.f};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);
    if (magFilterNearest) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (useMipmapsIfPossible)   {
#       ifdef NO_IMGUI_OPENGL_GLGENERATEMIPMAP
#           ifndef GL_GENERATE_MIPMAP
#               define GL_GENERATE_MIPMAP 0x8191
#           endif //GL_GENERATE_MIPMAP
        // I guess this is compilable, even if it's not supported:
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);    // This call must be done before glTexImage2D(...) // GL_GENERATE_MIPMAP can't be used with NPOT if there are not supported by the hardware of GL_ARB_texture_non_power_of_two.
#       endif //NO_IMGUI_OPENGL_GLGENERATEMIPMAP
    }
    if (minFilterNearest) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmapsIfPossible ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);
    else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmapsIfPossible ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    GLenum luminanceAlphaEnum = 0x190A; // 0x190A -> GL_LUMINANCE_ALPHA [Note that we're FORCING this definition even if when it's not defined! What should we use for 2 channels?]
    GLenum compressedLuminanceAlphaEnum = 0x84EB; // 0x84EB -> GL_COMPRESSED_LUMINANCE_ALPHA [Note that we're FORCING this definition even if when it's not defined! What should we use for 2 channels?]
#   ifdef GL_LUMINANCE_ALPHA
    luminanceAlphaEnum = GL_LUMINANCE_ALPHA;
#   endif //GL_LUMINANCE_ALPHA
#   ifdef GL_COMPRESSED_LUMINANCE_ALPHA
    compressedLuminanceAlphaEnum = GL_COMPRESSED_LUMINANCE_ALPHA;
#   endif //GL_COMPRESSED_LUMINANCE_ALPHA

#   ifdef IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY
    if (&imtexid==&gImImplPrivateParams.fontTex && channels==1) {
        GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        //printf("IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY used.\n");
    }
#   endif //IMIMPL_USE_ARB_TEXTURE_SWIZZLE_TO_SAVE_FONT_TEXTURE_MEMORY

    GLenum ifmt = channels==1 ? GL_ALPHA : channels==2 ? luminanceAlphaEnum : channels==3 ? GL_RGB : GL_RGBA;  // channels == 1 could be GL_LUMINANCE, GL_ALPHA, GL_RED ...
    GLenum fmt = ifmt;
#   ifdef IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE
    if (&imtexid==&gImImplPrivateParams.fontTex)    {
        ifmt = channels==1 ? GL_COMPRESSED_ALPHA : channels==2 ? compressedLuminanceAlphaEnum : channels==3 ? GL_COMPRESSED_RGB : GL_COMPRESSED_RGBA;  // channels == 1 could be GL_COMPRESSED_LUMINANCE, GL_COMPRESSED_ALPHA, GL_COMPRESSED_RED ...
    }
#   endif //IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE

    glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, potImageBuffer ? potImageBuffer : pixels);

#   ifdef IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE
    if (&imtexid==&gImImplPrivateParams.fontTex)    {
        GLint compressed = GL_FALSE;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);
        if (compressed==GL_FALSE)
            printf("Font texture compressed = %s\n",compressed==GL_TRUE?"true":"false");
    }
#   endif //IMIMPL_USE_ARB_TEXTURE_COMPRESSION_TO_COMPRESS_FONT_TEXTURE

    if (potImageBuffer) {STBI_FREE(potImageBuffer);potImageBuffer=NULL;}

#   ifndef NO_IMGUI_OPENGL_GLGENERATEMIPMAP
    if (useMipmapsIfPossible) glGenerateMipmap(GL_TEXTURE_2D);
#   endif //NO_IMGUI_OPENGL_GLGENERATEMIPMAP
    glBindTexture(GL_TEXTURE_2D, last_texture);
#endif
}

ImTextureID ImCreateTexture(const void* data, int width, int height)
{
#if     defined(IMGUI_RENDERING_VULKAN)
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();
    texture.TextureID = (ImTextureVk)ImGui_ImplVulkan_CreateTexture(data, width, height);
    texture.Width  = width;
    texture.Height = height;
    return (ImTextureID)texture.TextureID;
#elif   defined(IMGUI_RENDERING_DX11)
    if (!g_pd3dDevice)
        return nullptr;
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D *pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    //ID3D11ShaderResourceView * texture = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &texture.TextureID);
    pTexture->Release();
    texture.Width  = width;
    texture.Height = height;
    return (ImTextureID)texture.TextureID;
#elif   defined(IMGUI_RENDERING_DX9)
    if (!g_pd3dDevice)
        return nullptr;
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();
    if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture.TextureID, NULL) < 0)
        return nullptr;
    D3DLOCKED_RECT tex_locked_rect;
    int bytes_per_pixel = 4;
    if (texture.TextureID->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
        return nullptr;
    for (int y = 0; y < height; y++)
        memcpy((unsigned char*)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, (unsigned char* )data + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
    texture.TextureID->UnlockRect(0);
    return (ImTextureID)texture.TextureID;
#elif   defined(IMGUI_OPENGL)
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();

    // Upload texture to graphics system
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &texture.TextureID);
    glBindTexture(GL_TEXTURE_2D, texture.TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, last_texture);

    texture.Width  = width;
    texture.Height = height;

    return reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture.TextureID));
#else
    return nullptr;
#endif
}

#ifdef IMGUI_VULKAN_SHADER
ImTextureID ImCreateTexture(ImVulkan::VkImageMat & image)
{
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();
    texture.TextureID = (ImTextureVk)ImVulkanImageToImTexture(image);
    texture.Width  = image.w;
    texture.Height = image.h;
    return (ImTextureID)texture.TextureID;
}
#endif

static std::vector<ImTexture>::iterator ImFindTexture(ImTextureID texture)
{
#if     defined(IMGUI_RENDERING_VULKAN)
    auto textureID = reinterpret_cast<ImTextureVk>(texture);
#elif   defined(IMGUI_RENDERING_DX11)
    auto textureID = (ID3D11ShaderResourceView *)texture;
#elif   defined(IMGUI_RENDERING_DX9)
    auto textureID = reinterpret_cast<LPDIRECT3DTEXTURE9>(texture);
#elif   defined(IMGUI_OPENGL)
    auto textureID = static_cast<GLuint>(reinterpret_cast<intptr_t>(texture));
#else
    int textureID = -1;
#endif
    return std::find_if(g_Textures.begin(), g_Textures.end(), [textureID](ImTexture& texture)
    {
        return texture.TextureID == textureID;
    });
}

void ImDestroyTexture(ImTextureID texture)
{
    auto textureIt = ImFindTexture(texture);
    if (textureIt == g_Textures.end())
        return;
#if     defined(IMGUI_RENDERING_VULKAN)
    if (textureIt->TextureID)
    {
        ImGui_ImplVulkan_DestroyTexture(&textureIt->TextureID);
        textureIt->TextureID = nullptr;
    }
#elif   defined(IMGUI_RENDERING_DX11)
    if (textureIt->TextureID)
    {
        textureIt->TextureID->Release();
        textureIt->TextureID = nullptr;
    }
#elif   defined(IMGUI_RENDERING_DX9)
    if (textureIt->TextureID)
    {
        textureIt->TextureID->Release();
        textureIt->TextureID = nullptr;
    }
#elif   defined(IMGUI_OPENGL)
    if (textureIt->TextureID)
    {
        glDeleteTextures(1, &textureIt->TextureID);
        textureIt->TextureID = 0;
    }
#endif
    g_Textures.erase(textureIt);
}

int ImGetTextureWidth(ImTextureID texture)
{
    auto textureIt = ImFindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Width;
    return 0;
}

int ImGetTextureHeight(ImTextureID texture)
{
    auto textureIt = ImFindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Height;
    return 0;
}

ImTextureID ImLoadTexture(const char* path)
{
    int width = 0, height = 0, component = 0;
    if (auto data = stbi_load(path, &width, &height, &component, 4))
    {
        auto texture = ImCreateTexture(data, width, height);
        stbi_image_free(data);
        return texture;
    }
    else
        return nullptr;
}

bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS)	
{
#       ifdef _WIN32
            //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);  // Needed ??? Well, let's suppose the user initializes it himself for now"
            return ((size_t)ShellExecuteA( NULL, exploreModeForWindowsOS ? "explore" : "open", url, "", ".", SW_SHOWNORMAL ))>32;
#       else //_WIN32
            if (exploreModeForWindowsOS) exploreModeForWindowsOS = false;   // No warnings
            char tmp[4096];
            const char* openPrograms[]={"xdg-open","gnome-open"};	// Not sure what can I append here for MacOS

            static int openProgramIndex=-2;
            if (openProgramIndex==-2)   {
                openProgramIndex=-1;
                for (size_t i=0,sz=sizeof(openPrograms)/sizeof(openPrograms[0]);i<sz;i++) {
                    strcpy(tmp,"/usr/bin/");	// Well, we should check all the folders inside $PATH... and we ASSUME that /usr/bin IS inside $PATH (see below)
                    strcat(tmp,openPrograms[i]);
                    FILE* fd = ImFileOpen(tmp,"r");
                    if (fd) {
                        fclose(fd);
                        openProgramIndex = (int)i;
                        //printf(stderr,"found %s\n",tmp);
                        break;
                    }
                }
            }

            // Note that here we strip the prefix "/usr/bin" and just use openPrograms[openProgramsIndex].
            // Also note that if nothing has been found we use "xdg-open" (that might still work if it exists in $PATH, but not in /usr/bin).
            strcpy(tmp,openPrograms[openProgramIndex<0?0:openProgramIndex]);

            strcat(tmp," \"");
            strcat(tmp,url);
            strcat(tmp,"\"");
            return system(tmp)==0;
#       endif //_WIN32
}

void CloseAllPopupMenus()   {
    ImGuiContext& g = *GImGui;
    while (g.OpenPopupStack.size() > 0) g.OpenPopupStack.pop_back();
}

// Posted by Omar in one post. It might turn useful...
bool IsItemActiveLastFrame()    {
    ImGuiContext& g = *GImGui;
    if (g.ActiveIdPreviousFrame)
        return g.ActiveIdPreviousFrame== GImGui->CurrentWindow->DC.LastItemId;
    return false;
}
bool IsItemJustReleased()   {
    return IsItemActiveLastFrame() && !ImGui::IsItemActive();
}

void Debug_DrawItemRect(const ImVec4& col)
{
    auto drawList = ImGui::GetWindowDrawList();
    auto itemMin = ImGui::GetItemRectMin();
    auto itemMax = ImGui::GetItemRectMax();
    drawList->AddRect(itemMin, itemMax, ImColor(col));
}

#ifndef NO_IMGUIHELPER_FONT_METHODS
const ImFont *GetFont(int fntIndex) {return (fntIndex>=0 && fntIndex<ImGui::GetIO().Fonts->Fonts.size()) ? ImGui::GetIO().Fonts->Fonts[fntIndex] : NULL;}
void PushFont(int fntIndex)    {
    IM_ASSERT(fntIndex>=0 && fntIndex<ImGui::GetIO().Fonts->Fonts.size());
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[fntIndex]);
}
void TextColoredV(int fntIndex, const ImVec4 &col, const char *fmt, va_list args) {
    ImGui::PushFont(fntIndex);
    ImGui::TextColoredV(col,fmt, args);
    ImGui::PopFont();
}
void TextColored(int fntIndex, const ImVec4 &col, const char *fmt,...)  {
    va_list args;
    va_start(args, fmt);
    TextColoredV(fntIndex,col, fmt, args);
    va_end(args);
}
void TextV(int fntIndex, const char *fmt, va_list args) {
    if (ImGui::GetCurrentWindow()->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    ImGui::PushFont(fntIndex);
    TextUnformatted(g.TempBuffer, text_end);
    ImGui::PopFont();
}
void Text(int fntIndex, const char *fmt,...)    {
    va_list args;
    va_start(args, fmt);
    TextV(fntIndex,fmt, args);
    va_end(args);
}

bool GetTexCoordsFromGlyph(unsigned short glyph, ImVec2 &uv0, ImVec2 &uv1) {
    if (!GImGui->Font) return false;
    const ImFontGlyph* g = GImGui->Font->FindGlyph(glyph);
    if (g)  {
        uv0.x = g->U0; uv0.y = g->V0;
        uv1.x = g->U1; uv1.y = g->V1;
        return true;
    }
    return false;
}
float CalcMainMenuHeight()  {
    // Warning: according to https://github.com/ocornut/imgui/issues/252 this approach can fail [Better call ImGui::GetWindowSize().y from inside the menu and store the result somewhere]
    if (GImGui->FontBaseSize>0) return GImGui->FontBaseSize + GImGui->Style.FramePadding.y * 2.0f;
    else {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        ImFont* font = ImGui::GetFont();
        if (!font) {
            if (io.Fonts->Fonts.size()>0) font = io.Fonts->Fonts[0];
            else return (14)+style.FramePadding.y * 2.0f;
        }
        return (io.FontGlobalScale * font->Scale * font->FontSize) + style.FramePadding.y * 2.0f;
    }
}
#endif //NO_IMGUIHELPER_FONT_METHODS

#ifndef NO_IMGUIHELPER_DRAW_METHODS
inline static void GetVerticalGradientTopAndBottomColors(ImU32 c,float fillColorGradientDeltaIn0_05,ImU32& tc,ImU32& bc)  {
    if (fillColorGradientDeltaIn0_05==0) {tc=bc=c;return;}

    static ImU32 cacheColorIn=0;static float cacheGradientIn=0.f;static ImU32 cacheTopColorOut=0;static ImU32 cacheBottomColorOut=0;
    if (cacheColorIn==c && cacheGradientIn==fillColorGradientDeltaIn0_05)   {tc=cacheTopColorOut;bc=cacheBottomColorOut;return;}
    cacheColorIn=c;cacheGradientIn=fillColorGradientDeltaIn0_05;

    const bool negative = (fillColorGradientDeltaIn0_05<0);
    if (negative) fillColorGradientDeltaIn0_05=-fillColorGradientDeltaIn0_05;
    if (fillColorGradientDeltaIn0_05>0.5f) fillColorGradientDeltaIn0_05=0.5f;


    // New code:
    //#define IM_COL32(R,G,B,A)    (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))
    const int fcgi = fillColorGradientDeltaIn0_05*255.0f;
    const int R = (unsigned char) (c>>IM_COL32_R_SHIFT);    // The cast should reset upper bits (as far as I hope)
    const int G = (unsigned char) (c>>IM_COL32_G_SHIFT);
    const int B = (unsigned char) (c>>IM_COL32_B_SHIFT);
    const int A = (unsigned char) (c>>IM_COL32_A_SHIFT);

    int r = R+fcgi, g = G+fcgi, b = B+fcgi;
    if (r>255) r=255;
    if (g>255) g=255;
    if (b>255) b=255;
    if (negative) bc = IM_COL32(r,g,b,A); else tc = IM_COL32(r,g,b,A);

    r = R-fcgi; g = G-fcgi; b = B-fcgi;
    if (r<0) r=0;
    if (g<0) g=0;
    if (b<0) b=0;
    if (negative) tc = IM_COL32(r,g,b,A); else bc = IM_COL32(r,g,b,A);

    // Old legacy code (to remove)... [However here we lerp alpha too...]
    /*// Can we do it without the double conversion ImU32 -> ImVec4 -> ImU32 ?
    const ImVec4 cf = ColorConvertU32ToFloat4(c);
    ImVec4 tmp(cf.x+fillColorGradientDeltaIn0_05,cf.y+fillColorGradientDeltaIn0_05,cf.z+fillColorGradientDeltaIn0_05,cf.w+fillColorGradientDeltaIn0_05);
    if (tmp.x>1.f) tmp.x=1.f;if (tmp.y>1.f) tmp.y=1.f;if (tmp.z>1.f) tmp.z=1.f;if (tmp.w>1.f) tmp.w=1.f;
    if (negative) bc = ColorConvertFloat4ToU32(tmp); else tc = ColorConvertFloat4ToU32(tmp);
    tmp=ImVec4(cf.x-fillColorGradientDeltaIn0_05,cf.y-fillColorGradientDeltaIn0_05,cf.z-fillColorGradientDeltaIn0_05,cf.w-fillColorGradientDeltaIn0_05);
    if (tmp.x<0.f) tmp.x=0.f;if (tmp.y<0.f) tmp.y=0.f;if (tmp.z<0.f) tmp.z=0.f;if (tmp.w<0.f) tmp.w=0.f;
    if (negative) tc = ColorConvertFloat4ToU32(tmp); else bc = ColorConvertFloat4ToU32(tmp);*/


    cacheTopColorOut=tc;cacheBottomColorOut=bc;
}
inline static ImU32 GetVerticalGradient(const ImVec4& ct,const ImVec4& cb,float DH,float H)    {
    IM_ASSERT(H!=0);
    const float fa = DH/H;
    const float fc = (1.f-fa);
    return ColorConvertFloat4ToU32(ImVec4(
        ct.x * fc + cb.x * fa,
        ct.y * fc + cb.y * fa,
        ct.z * fc + cb.z * fa,
        ct.w * fc + cb.w * fa)
    );
}
void ImDrawListAddConvexPolyFilledWithVerticalGradient(ImDrawList *dl, const ImVec2 *points, const int points_count, ImU32 colTop, ImU32 colBot,float miny,float maxy)
{
    if (!dl) return;
    if (colTop==colBot)  {
        dl->AddConvexPolyFilled(points,points_count,colTop);
        return;
    }
    const ImVec2 uv = GImGui->DrawListSharedData.TexUvWhitePixel;
    const bool anti_aliased = GImGui->Style.AntiAliasedFill;

    int height=0;
    if (miny<=0 || maxy<=0) {
        const float max_float = 999999999999999999.f;
        miny=max_float;maxy=-max_float;
        for (int i = 0; i < points_count; i++) {
            const float h = points[i].y;
            if (h < miny) miny = h;
            else if (h > maxy) maxy = h;
        }
    }
    height = maxy-miny;
    const ImVec4 colTopf = ColorConvertU32ToFloat4(colTop);
    const ImVec4 colBotf = ColorConvertU32ToFloat4(colBot);


    if (anti_aliased)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;

        const ImVec4 colTransTopf(colTopf.x,colTopf.y,colTopf.z,0.f);
        const ImVec4 colTransBotf(colBotf.x,colBotf.y,colBotf.z,0.f);
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        dl->PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = dl->_VtxCurrentIdx;
        unsigned int vtx_outer_idx = dl->_VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            dl->_IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            //_VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            //_VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            dl->_VtxWritePtr[0].pos = (points[i1] - dm); dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colTopf,colBotf,points[i1].y-miny,height);        // Inner
            dl->_VtxWritePtr[1].pos = (points[i1] + dm); dl->_VtxWritePtr[1].uv = uv; dl->_VtxWritePtr[1].col = GetVerticalGradient(colTransTopf,colTransBotf,points[i1].y-miny,height);  // Outer
            dl->_VtxWritePtr += 2;

            // Add indexes for fringes
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            dl->_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); dl->_IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); dl->_IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            dl->_IdxWritePtr += 6;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        dl->PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            //_VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            dl->_VtxWritePtr[0].pos = points[i]; dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colTopf,colBotf,points[i].y-miny,height);
            dl->_VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(dl->_VtxCurrentIdx); dl->_IdxWritePtr[1] = (ImDrawIdx)(dl->_VtxCurrentIdx+i-1); dl->_IdxWritePtr[2] = (ImDrawIdx)(dl->_VtxCurrentIdx+i);
            dl->_IdxWritePtr += 3;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}
void ImDrawListPathFillWithVerticalGradientAndStroke(ImDrawList *dl, const ImU32 &fillColorTop, const ImU32 &fillColorBottom, const ImU32 &strokeColor, bool strokeClosed, float strokeThickness, float miny, float maxy)    {
    if (!dl) return;
    if (fillColorTop==fillColorBottom) dl->AddConvexPolyFilled(dl->_Path.Data,dl->_Path.Size, fillColorTop);
    else if ((fillColorTop & IM_COL32_A_MASK) != 0 || (fillColorBottom & IM_COL32_A_MASK) != 0) ImDrawListAddConvexPolyFilledWithVerticalGradient(dl, dl->_Path.Data, dl->_Path.Size, fillColorTop, fillColorBottom,miny,maxy);
    if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0) dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, strokeClosed, strokeThickness);
    dl->PathClear();
}
void ImDrawListPathFillAndStroke(ImDrawList *dl, const ImU32 &fillColor, const ImU32 &strokeColor, bool strokeClosed, float strokeThickness)    {
    if (!dl) return;
    if ((fillColor & IM_COL32_A_MASK) != 0) dl->AddConvexPolyFilled(dl->_Path.Data, dl->_Path.Size, fillColor);
    if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0) dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, strokeClosed, strokeThickness);
    dl->PathClear();
}
void ImDrawListAddRect(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness) {
    if (!dl || (((fillColor & IM_COL32_A_MASK) == 0) && ((strokeColor & IM_COL32_A_MASK) == 0)))  return;
    dl->PathRect(a, b, rounding, rounding_corners);
    ImDrawListPathFillAndStroke(dl,fillColor,strokeColor,true,strokeThickness);
}
void ImDrawListAddRectWithVerticalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColorTop, const ImU32 &fillColorBottom, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness) {
    if (!dl || (((fillColorTop & IM_COL32_A_MASK) == 0) && ((fillColorBottom & IM_COL32_A_MASK) == 0) && ((strokeColor & IM_COL32_A_MASK) == 0)))  return;
    if (rounding==0.f || rounding_corners==ImDrawFlags_RoundCornersNone) {
        dl->AddRectFilledMultiColor(a,b,fillColorTop,fillColorTop,fillColorBottom,fillColorBottom); // Huge speedup!
        if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0.f) {
            dl->PathRect(a, b, rounding, rounding_corners);
            dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, true, strokeThickness);
            dl->PathClear();
        }
    }
    else    {
        dl->PathRect(a, b, rounding, rounding_corners);
        ImDrawListPathFillWithVerticalGradientAndStroke(dl,fillColorTop,fillColorBottom,strokeColor,true,strokeThickness,a.y,b.y);
    }
}
void ImDrawListPathArcTo(ImDrawList *dl, const ImVec2 &centre, const ImVec2 &radii, float amin, float amax, int num_segments)  {
    if (!dl) return;
    if (radii.x == 0.0f || radii.y==0) dl->_Path.push_back(centre);
    dl->_Path.reserve(dl->_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = amin + ((float)i / (float)num_segments) * (amax - amin);
        dl->_Path.push_back(ImVec2(centre.x + cosf(a) * radii.x, centre.y + sinf(a) * radii.y));
    }
}
void ImDrawListAddEllipse(ImDrawList *dl, const ImVec2 &centre, const ImVec2 &radii, const ImU32 &fillColor, const ImU32 &strokeColor, int num_segments, float strokeThickness)   {
    if (!dl) return;
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    ImDrawListPathArcTo(dl,centre, radii, 0.0f, a_max, num_segments);
    ImDrawListPathFillAndStroke(dl,fillColor,strokeColor,true,strokeThickness);
}
void ImDrawListAddEllipseWithVerticalGradient(ImDrawList *dl, const ImVec2 &centre, const ImVec2 &radii, const ImU32 &fillColorTop, const ImU32 &fillColorBottom, const ImU32 &strokeColor, int num_segments, float strokeThickness)   {
    if (!dl) return;
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    ImDrawListPathArcTo(dl,centre, radii, 0.0f, a_max, num_segments);
    ImDrawListPathFillWithVerticalGradientAndStroke(dl,fillColorTop,fillColorBottom,strokeColor,true,strokeThickness,centre.y-radii.y,centre.y+radii.y);
}
void ImDrawListAddCircle(ImDrawList *dl, const ImVec2 &centre, float radius, const ImU32 &fillColor, const ImU32 &strokeColor, int num_segments, float strokeThickness)   {
    if (!dl) return;
    const ImVec2 radii(radius,radius);
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    ImDrawListPathArcTo(dl,centre, radii, 0.0f, a_max, num_segments-1);
    ImDrawListPathFillAndStroke(dl,fillColor,strokeColor,true,strokeThickness);
}
void ImDrawListAddCircleWithVerticalGradient(ImDrawList *dl, const ImVec2 &centre, float radius, const ImU32 &fillColorTop, const ImU32 &fillColorBottom, const ImU32 &strokeColor, int num_segments, float strokeThickness)   {
    if (!dl) return;
    const ImVec2 radii(radius,radius);
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    ImDrawListPathArcTo(dl,centre, radii, 0.0f, a_max, num_segments-1);
    ImDrawListPathFillWithVerticalGradientAndStroke(dl,fillColorTop,fillColorBottom,strokeColor,true,strokeThickness,centre.y-radius,centre.y+radius);
}
void ImDrawListAddRectWithVerticalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, float fillColorGradientDeltaIn0_05, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness)   {
    ImU32 fillColorTop,fillColorBottom;GetVerticalGradientTopAndBottomColors(fillColor,fillColorGradientDeltaIn0_05,fillColorTop,fillColorBottom);
    ImDrawListAddRectWithVerticalGradient(dl,a,b,fillColorTop,fillColorBottom,strokeColor,rounding,rounding_corners,strokeThickness);
}
void ImDrawListAddPolyLine(ImDrawList *dl, const ImVec2* polyPoints, int numPolyPoints, ImU32 strokeColor, float strokeThickness, bool strokeClosed, const ImVec2 &offset, const ImVec2 &scale) {
    if (polyPoints && numPolyPoints>0 && (strokeColor & IM_COL32_A_MASK) != 0) {
	static ImVector<ImVec2> points;
	points.resize(numPolyPoints);
	for (int i=0;i<numPolyPoints;i++)   points[i] = offset + polyPoints[i]*scale;
    dl->AddPolyline(&points[0],points.size(),strokeColor,strokeClosed,strokeThickness);
    }
}

void ImDrawListAddConvexPolyFilledWithHorizontalGradient(ImDrawList *dl, const ImVec2 *points, const int points_count, ImU32 colLeft, ImU32 colRight, float minx, float maxx)
{
    if (!dl) return;
    if (colLeft==colRight)  {
        dl->AddConvexPolyFilled(points,points_count,colLeft);
        return;
    }
    const ImVec2 uv = GImGui->DrawListSharedData.TexUvWhitePixel;
    const bool anti_aliased = GImGui->Style.AntiAliasedFill;
    //if (ImGui::GetIO().KeyCtrl) anti_aliased = false; // Debug

    int width=0;
    if (minx<=0 || maxx<=0) {
        const float max_float = 999999999999999999.f;
        minx=max_float;maxx=-max_float;
        for (int i = 0; i < points_count; i++) {
            const float w = points[i].x;
            if (w < minx) minx = w;
            else if (w > maxx) maxx = w;
        }
    }
    width = maxx-minx;
    const ImVec4 colLeftf  = ColorConvertU32ToFloat4(colLeft);
    const ImVec4 colRightf = ColorConvertU32ToFloat4(colRight);


    if (anti_aliased)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;

        const ImVec4 colTransLeftf(colLeftf.x,colLeftf.y,colLeftf.z,0.f);
        const ImVec4 colTransRightf(colRightf.x,colRightf.y,colRightf.z,0.f);
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        dl->PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = dl->_VtxCurrentIdx;
        unsigned int vtx_outer_idx = dl->_VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            dl->_IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            //_VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            //_VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            dl->_VtxWritePtr[0].pos = (points[i1] - dm); dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colLeftf,colRightf,points[i1].x-minx,width);        // Inner
            dl->_VtxWritePtr[1].pos = (points[i1] + dm); dl->_VtxWritePtr[1].uv = uv; dl->_VtxWritePtr[1].col = GetVerticalGradient(colTransLeftf,colTransRightf,points[i1].x-minx,width);  // Outer
            dl->_VtxWritePtr += 2;

            // Add indexes for fringes
            dl->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); dl->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); dl->_IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            dl->_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); dl->_IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); dl->_IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            dl->_IdxWritePtr += 6;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        dl->PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            //_VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            dl->_VtxWritePtr[0].pos = points[i]; dl->_VtxWritePtr[0].uv = uv; dl->_VtxWritePtr[0].col = GetVerticalGradient(colLeftf,colRightf,points[i].x-minx,width);
            dl->_VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            dl->_IdxWritePtr[0] = (ImDrawIdx)(dl->_VtxCurrentIdx); dl->_IdxWritePtr[1] = (ImDrawIdx)(dl->_VtxCurrentIdx+i-1); dl->_IdxWritePtr[2] = (ImDrawIdx)(dl->_VtxCurrentIdx+i);
            dl->_IdxWritePtr += 3;
        }
        dl->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}
void ImDrawListPathFillWithHorizontalGradientAndStroke(ImDrawList *dl, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, bool strokeClosed, float strokeThickness, float minx, float maxx)    {
    if (!dl) return;
    if (fillColorLeft==fillColorRight) dl->AddConvexPolyFilled(dl->_Path.Data,dl->_Path.Size, fillColorLeft);
    else if ((fillColorLeft & IM_COL32_A_MASK) != 0 || (fillColorRight & IM_COL32_A_MASK) != 0) ImDrawListAddConvexPolyFilledWithHorizontalGradient(dl, dl->_Path.Data, dl->_Path.Size, fillColorLeft, fillColorRight,minx,maxx);
    if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0) dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, strokeClosed, strokeThickness);
    dl->PathClear();
}
void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColorLeft, const ImU32 &fillColoRight, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness) {
    if (!dl || (((fillColorLeft & IM_COL32_A_MASK) == 0) && ((fillColoRight & IM_COL32_A_MASK) == 0) && ((strokeColor & IM_COL32_A_MASK) == 0)))  return;
    if (rounding==0.f || rounding_corners==ImDrawFlags_RoundCornersNone) {
        dl->AddRectFilledMultiColor(a,b,fillColorLeft,fillColoRight,fillColoRight,fillColorLeft); // Huge speedup!
        if ((strokeColor& IM_COL32_A_MASK)!= 0 && strokeThickness>0.f) {
            dl->PathRect(a, b, rounding, rounding_corners);
            dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, true, strokeThickness);
            dl->PathClear();
        }
    }
    else    {
        dl->PathRect(a, b, rounding, rounding_corners);
        ImDrawListPathFillWithHorizontalGradientAndStroke(dl,fillColorLeft,fillColoRight,strokeColor,true,strokeThickness,a.x,b.x);
    }
}
void ImDrawListAddEllipseWithHorizontalGradient(ImDrawList *dl, const ImVec2 &centre, const ImVec2 &radii, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, int num_segments, float strokeThickness)   {
    if (!dl) return;
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    ImDrawListPathArcTo(dl,centre, radii, 0.0f, a_max, num_segments);
    ImDrawListPathFillWithHorizontalGradientAndStroke(dl,fillColorLeft,fillColorRight,strokeColor,true,strokeThickness,centre.y-radii.y,centre.y+radii.y);
}
void ImDrawListAddCircleWithHorizontalGradient(ImDrawList *dl, const ImVec2 &centre, float radius, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, int num_segments, float strokeThickness)   {
    if (!dl) return;
    const ImVec2 radii(radius,radius);
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    ImDrawListPathArcTo(dl,centre, radii, 0.0f, a_max, num_segments-1);
    ImDrawListPathFillWithHorizontalGradientAndStroke(dl,fillColorLeft,fillColorRight,strokeColor,true,strokeThickness,centre.y-radius,centre.y+radius);
}
void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, float fillColorGradientDeltaIn0_05, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness)   {
    ImU32 fillColorTop,fillColorBottom;GetVerticalGradientTopAndBottomColors(fillColor,fillColorGradientDeltaIn0_05,fillColorTop,fillColorBottom);
    ImDrawListAddRectWithHorizontalGradient(dl,a,b,fillColorTop,fillColorBottom,strokeColor,rounding,rounding_corners,strokeThickness);
}
#endif //NO_IMGUIHELPER_DRAW_METHODS

// These two methods are inspired by imguidock.cpp
void PutInBackground(const char* optionalRootWindowName)  {
    ImGuiWindow* w = optionalRootWindowName ? FindWindowByName(optionalRootWindowName) : GetCurrentWindow();
    if (!w) return;
    ImGuiContext& g = *GImGui;
    if (g.Windows[0] == w) return;
    const int isz = g.Windows.Size;
    for (int i = 0; i < isz; i++)  {
        if (g.Windows[i] == w)  {
            for (int j = i; j > 0; --j) g.Windows[j] = g.Windows[j-1];  // shifts [0,j-1] to [1,j]
            g.Windows[0] = w;
            break;
        }
    }
}
void PutInForeground(const char* optionalRootWindowName)  {
    ImGuiWindow* w = optionalRootWindowName ? FindWindowByName(optionalRootWindowName) : GetCurrentWindow();
    if (!w) return;
    ImGuiContext& g = *GImGui;
    const int iszMinusOne = g.Windows.Size - 1;
    if (iszMinusOne<0 || g.Windows[iszMinusOne] == w) return;
    for (int i = iszMinusOne; i >= 0; --i)  {
        if (g.Windows[i] == w)  {
            for (int j = i; j < iszMinusOne; j++) g.Windows[j] = g.Windows[j+1];  // shifts [i+1,iszMinusOne] to [i,iszMinusOne-1]
            g.Windows[iszMinusOne] = w;
            break;
        }
    }
}

ScopedItemWidth::ScopedItemWidth(float width)
{
    ImGui::PushItemWidth(width);
}

ScopedItemWidth::~ScopedItemWidth()
{
    Release();
}

void ScopedItemWidth::Release()
{
    if (m_IsDone)
        return;

    ImGui::PopItemWidth();

    m_IsDone = true;
}

ScopedDisableItem::ScopedDisableItem(bool disable, float disabledAlpha)
    : m_Disable(disable)
{
    if (!m_Disable)
        return;

    auto wasDisabled = (ImGui::GetCurrentWindow()->DC.ItemFlags & ImGuiItemFlags_Disabled) == ImGuiItemFlags_Disabled;

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

    auto& stale = ImGui::GetStyle();
    m_LastAlpha = stale.Alpha;

    // Don't override alpha if we're already in disabled context.
    if (!wasDisabled)
        stale.Alpha = disabledAlpha;
}

ScopedDisableItem::~ScopedDisableItem()
{
    Release();
}

void ScopedDisableItem::Release()
{
    if (!m_Disable)
        return;

    auto& stale = ImGui::GetStyle();
    stale.Alpha = m_LastAlpha;

    ImGui::PopItemFlag();

    m_Disable = false;
}

ScopedSuspendLayout::ScopedSuspendLayout()
{
    m_Window = ImGui::GetCurrentWindow();
    m_CursorPos = m_Window->DC.CursorPos;
    m_CursorPosPrevLine = m_Window->DC.CursorPosPrevLine;
    m_CursorMaxPos = m_Window->DC.CursorMaxPos;
    m_CurrLineSize = m_Window->DC.CurrLineSize;
    m_PrevLineSize = m_Window->DC.PrevLineSize;
    m_CurrLineTextBaseOffset = m_Window->DC.CurrLineTextBaseOffset;
    m_PrevLineTextBaseOffset = m_Window->DC.PrevLineTextBaseOffset;
}

ScopedSuspendLayout::~ScopedSuspendLayout()
{
    Release();
}

void ScopedSuspendLayout::Release()
{
    if (m_Window == nullptr)
        return;

    m_Window->DC.CursorPos = m_CursorPos;
    m_Window->DC.CursorPosPrevLine = m_CursorPosPrevLine;
    m_Window->DC.CursorMaxPos = m_CursorMaxPos;
    m_Window->DC.CurrLineSize = m_CurrLineSize;
    m_Window->DC.PrevLineSize = m_PrevLineSize;
    m_Window->DC.CurrLineTextBaseOffset = m_CurrLineTextBaseOffset;
    m_Window->DC.PrevLineTextBaseOffset = m_PrevLineTextBaseOffset;

    m_Window = nullptr;
}

ItemBackgroundRenderer::ItemBackgroundRenderer(OnDrawCallback onDrawBackground)
    : m_OnDrawBackground(std::move(onDrawBackground))
{
    m_DrawList = ImGui::GetWindowDrawList();
    m_Splitter.Split(m_DrawList, 2);
    m_Splitter.SetCurrentChannel(m_DrawList, 1);
}

ItemBackgroundRenderer::~ItemBackgroundRenderer()
{
    Commit();
}

void ItemBackgroundRenderer::Commit()
{
    if (m_Splitter._Current == 0)
        return;

    m_Splitter.SetCurrentChannel(m_DrawList, 0);

    if (m_OnDrawBackground)
        m_OnDrawBackground(m_DrawList);

    m_Splitter.Merge(m_DrawList);
}

void ItemBackgroundRenderer::Discard()
{
    if (m_Splitter._Current == 1)
        m_Splitter.Merge(m_DrawList);
}

StorageHandler<MostRecentlyUsedList::Settings> MostRecentlyUsedList::s_Storage;


void MostRecentlyUsedList::Install(ImGuiContext* context)
{
    context->SettingsHandlers.push_back(s_Storage.MakeHandler("MostRecentlyUsedList"));

    s_Storage.ReadLine = [](ImGuiContext*, Settings* entry, const char* line)
    {
        const char* lineEnd = line + strlen(line);

        auto parseListEntry = [lineEnd](const char* line, int& index) -> const char*
        {
            char* indexEnd = nullptr;
            errno = 0;
            index = strtol(line, &indexEnd, 10);
            if (errno == ERANGE)
                return nullptr;
            if (indexEnd >= lineEnd)
                return nullptr;
            if (*indexEnd != '=')
                return nullptr;
            return indexEnd + 1;
        };


        int index = 0;
        if (auto path = parseListEntry(line, index))
        {
            if (static_cast<int>(entry->m_List.size()) <= index)
                entry->m_List.resize(index + 1);
            entry->m_List[index] = path;
        }
    };

    s_Storage.WriteAll = [](ImGuiContext*, ImGuiTextBuffer* out_buf, const StorageHandler<Settings>::Storage& storage)
    {
        for (auto& entry : storage)
        {
            out_buf->appendf("[%s][%s]\n", "MostRecentlyUsedList", entry.first.c_str());
            int index = 0;
            for (auto& value : entry.second->m_List)
                out_buf->appendf("%d=%s\n", index++, value.c_str());
            out_buf->append("\n");
        }
    };
}

MostRecentlyUsedList::MostRecentlyUsedList(const char* id, int capacity /*= 10*/)
    : m_ID(id)
    , m_Capacity(capacity)
    , m_List(s_Storage.FindOrCreate(id)->m_List)
{
}

void MostRecentlyUsedList::Add(const std::string& item)
{
    Add(item.c_str());
}

void MostRecentlyUsedList::Add(const char* item)
{
    auto itemIt = std::find(m_List.begin(), m_List.end(), item);
    if (itemIt != m_List.end())
    {
        // Item is already on the list. Rotate list to move it to the
        // first place.
        std::rotate(m_List.begin(), itemIt, itemIt + 1);
    }
    else
    {
        // Push new item to the back, rotate list to move it to the front,
        // pop back last element if we're over capacity.
        m_List.push_back(item);
        std::rotate(m_List.begin(), m_List.end() - 1, m_List.end());
        if (static_cast<int>(m_List.size()) > m_Capacity)
            m_List.pop_back();
    }

    PushToStorage();

    ImGui::MarkIniSettingsDirty();
}

void MostRecentlyUsedList::Clear()
{
    if (m_List.empty())
        return;

    m_List.resize(0);

    PushToStorage();

    ImGui::MarkIniSettingsDirty();
}

const std::vector<std::string>& MostRecentlyUsedList::GetList() const
{
    return m_List;
}

int MostRecentlyUsedList::Size() const
{
    return static_cast<int>(m_List.size());
}

void MostRecentlyUsedList::PullFromStorage()
{
    if (auto settings = s_Storage.Find(m_ID.c_str()))
        m_List = settings->m_List;
}

void MostRecentlyUsedList::PushToStorage()
{
    auto settings = s_Storage.FindOrCreate(m_ID.c_str());
    settings->m_List = m_List;
}

void Grid::Begin(const char* id, int columns, float width)
{
    Begin(ImGui::GetID(id), columns, width);
}

void Grid::Begin(ImU32 id, int columns, float width)
{
    m_CursorPos = ImGui::GetCursorScreenPos();

    ImGui::PushID(id);
    m_Columns = ImMax(1, columns);
    m_Storage = ImGui::GetStateStorage();

    for (int i = 0; i < columns; ++i)
    {
        ImGui::PushID(ColumnSeed());
        m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidthAcc"), -1.0f);
        ImGui::PopID();
    }

    m_ColumnAlignment = 0.0f;
    m_MinimumWidth = width;

    ImGui::BeginGroup();

    EnterCell(0, 0);
}

void Grid::NextColumn()
{
    LeaveCell();

    int nextColumn = m_Column + 1;
    int nextRow    = 0;
    if (nextColumn > m_Columns)
    {
        nextColumn -= m_Columns;
        nextRow    += 1;
    }

    auto cursorPos = m_CursorPos;
    for (int i = 0; i < nextColumn; ++i)
    {
        ImGui::PushID(ColumnSeed(i));
        auto maximumColumnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
        ImGui::PopID();

        if (maximumColumnWidth > 0.0f)
            cursorPos.x += maximumColumnWidth + ImGui::GetStyle().ItemSpacing.x;
    }

    ImGui::SetCursorScreenPos(cursorPos);

    EnterCell(nextColumn, nextRow);
}

void Grid::NextRow()
{
    LeaveCell();

    auto cursorPos = ImGui::GetCursorScreenPos();
    cursorPos.x = m_CursorPos.x;
    for (int i = 0; i < m_Column; ++i)
    {
        ImGui::PushID(ColumnSeed(i));
        auto maximumColumnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
        ImGui::PopID();

        if (maximumColumnWidth > 0.0f)
            cursorPos.x += maximumColumnWidth + ImGui::GetStyle().ItemSpacing.x;
    }

    ImGui::SetCursorScreenPos(cursorPos);

    EnterCell(m_Column, m_Row + 1);
}

void Grid::EnterCell(int column, int row)
{
    m_Column = column;
    m_Row    = row;

    ImGui::PushID(ColumnSeed());
    m_MaximumColumnWidthAcc = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidthAcc"), -1.0f);
    auto maximumColumnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
    ImGui::PopID();

    ImGui::PushID(Seed());
    auto lastCellWidth = m_Storage->GetFloat(ImGui::GetID("LastCellWidth"), -1.0f);

    if (maximumColumnWidth >= 0.0f && lastCellWidth >= 0.0f)
    {
        auto freeSpace = maximumColumnWidth - lastCellWidth;

        auto offset = ImFloor(m_ColumnAlignment * freeSpace);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(offset, 0.0f));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::PopStyleVar();
    }

    ImGui::BeginGroup();
}

void Grid::LeaveCell()
{
    ImGui::EndGroup();

    auto itemSize = ImGui::GetItemRectSize();
    m_Storage->SetFloat(ImGui::GetID("LastCellWidth"), itemSize.x);
    ImGui::PopID();

    m_MaximumColumnWidthAcc = ImMax(m_MaximumColumnWidthAcc, itemSize.x);
    ImGui::PushID(ColumnSeed());
    m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidthAcc"), m_MaximumColumnWidthAcc);
    ImGui::PopID();
}

void Grid::SetColumnAlignment(float alignment)
{
    alignment = ImClamp(alignment, 0.0f, 1.0f);
    m_ColumnAlignment = alignment;
}

void Grid::End()
{
    LeaveCell();

    ImGui::EndGroup();

    float totalWidth = 0.0f;
    for (int i = 0; i < m_Columns; ++i)
    {
        ImGui::PushID(ColumnSeed(i));
        auto currentMaxCellWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidthAcc"), -1.0f);
        totalWidth += currentMaxCellWidth;
        m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidth"), currentMaxCellWidth);
        ImGui::PopID();
    }

    if (totalWidth < m_MinimumWidth)
    {
        auto spaceToDivide = m_MinimumWidth - totalWidth;
        auto spacePerColumn = ImCeil(spaceToDivide / m_Columns);

        for (int i = 0; i < m_Columns; ++i)
        {
            ImGui::PushID(ColumnSeed(i));
            auto columnWidth = m_Storage->GetFloat(ImGui::GetID("MaximumColumnWidth"), -1.0f);
            columnWidth += spacePerColumn;
            m_Storage->SetFloat(ImGui::GetID("MaximumColumnWidth"), columnWidth);
            ImGui::PopID();

            spaceToDivide -= spacePerColumn;
            if (spaceToDivide < 0)
                spacePerColumn += spaceToDivide;
        }
    }

    ImGui::PopID();
}

} // namespace Imgui

#ifndef NO_IMGUIHELPER_SERIALIZATION
#include <stdio.h>  // FILE
namespace ImGuiHelper   {

static const char* FieldTypeNames[ImGui::FT_COUNT+1] = {"INT","UNSIGNED","FLOAT","DOUBLE","STRING","ENUM","BOOL","COLOR","TEXTLINE","CUSTOM","COUNT"};
static const char* FieldTypeFormats[ImGui::FT_COUNT]={"%d","%u","%f","%f","%s","%d","%d","%f","%s","%s"};
static const char* FieldTypeFormatsWithCustomPrecision[ImGui::FT_COUNT]={"%.*d","%*u","%.*f","%.*f","%*s","%*d","%*d","%.*f","%*s","%*s"};

#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
void Deserializer::clear() {
    if (f_data) ImGui::MemFree(f_data);
    f_data = NULL;f_size=0;
}
bool Deserializer::loadFromFile(const char *filename) {
    clear();
    if (!filename) return false;
    FILE* f;
    if ((f = ImFileOpen(filename, "rt")) == NULL) return false;
    if (fseek(f, 0, SEEK_END))  {
        fclose(f);
        return false;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)    {
        fclose(f);
        return false;
    }
    f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))  {
        fclose(f);
        return false;
    }
    f_data = (char*)ImGui::MemAlloc(f_size+1);
    f_size = fread(f_data, 1, f_size, f); // Text conversion alter read size so let's not be fussy about return value
    fclose(f);
    if (f_size == 0)    {
        clear();
        return false;
    }
    f_data[f_size] = 0;
    ++f_size;
    return true;
}
bool Deserializer::allocate(size_t sizeToAllocate, const char *optionalTextToCopy, size_t optionalTextToCopySize)    {
    clear();
    if (sizeToAllocate==0) return false;
    f_size = sizeToAllocate;
    f_data = (char*)ImGui::MemAlloc(f_size);
    if (!f_data) {clear();return false;}
    if (optionalTextToCopy && optionalTextToCopySize>0) memcpy(f_data,optionalTextToCopy,optionalTextToCopySize>f_size ? f_size:optionalTextToCopySize);
    return true;
}
Deserializer::Deserializer(const char *filename) : f_data(NULL),f_size(0) {
    if (filename) loadFromFile(filename);
}
Deserializer::Deserializer(const char *text, size_t textSizeInBytes) : f_data(NULL),f_size(0) {
    allocate(textSizeInBytes,text,textSizeInBytes);
}

const char* Deserializer::parse(Deserializer::ParseCallback cb, void *userPtr, const char *optionalBufferStart) const {
    if (!cb || !f_data || f_size==0) return NULL;
    //------------------------------------------------
    // Parse file in memory
    char name[128];name[0]='\0';
    char typeName[32];char format[32]="";bool quitParsing = false;
    char charBuffer[sizeof(double)*10];void* voidBuffer = (void*) &charBuffer[0];
    static char textBuffer[2050];
    const char* varName = NULL;int numArrayElements = 0;FieldType ft = ImGui::FT_COUNT;
    const char* buf_end = f_data + f_size-1;
    for (const char* line_start = optionalBufferStart ? optionalBufferStart : f_data; line_start < buf_end; )
    {
        const char* line_end = line_start;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r') line_end++;

        if (name[0]=='\0' && line_start[0] == '[' && line_end > line_start && line_end[-1] == ']')
        {
            ImFormatString(name, IM_ARRAYSIZE(name), "%.*s", (int)(line_end-line_start-2), line_start+1);
            //fprintf(stderr,"name: %s\n",name);  // dbg

            // Here we have something like: FLOAT-4:VariableName
            // We have to split into FLOAT 4 VariableName
            varName = NULL;numArrayElements = 0;ft = ImGui::FT_COUNT;format[0]='\0';
            const char* colonCh = strchr(name,':');
            const char* minusCh = strchr(name,'-');
            if (!colonCh) {
                fprintf(stderr,"ImGuiHelper::Deserializer::parse(...) warning (skipping line with no semicolon). name: %s\n",name);  // dbg
                name[0]='\0';
            }
            else {
                ptrdiff_t diff = 0,diff2 = 0;
                if (!minusCh || (minusCh-colonCh)>0)  {diff = (colonCh-name);numArrayElements=1;}
                else {
                    diff = (minusCh-name);
                    diff2 = colonCh-minusCh;
                    if (diff2>1 && diff2<7)    {
                        static char buff[8];
                        strncpy(&buff[0],(const char*) &minusCh[1],diff2);buff[diff2-1]='\0';
                        sscanf(buff,"%d",&numArrayElements);
                        //fprintf(stderr,"WARN: %s\n",buff);
                    }
                    else if (diff>0) numArrayElements = ((char)name[diff+1]-(char)'0');  // TODO: FT_STRING needs multibytes -> rewrite!
                }
                if (diff>0) {
                    const size_t len = (size_t)(diff>31?31:diff);
                    strncpy(typeName,name,len);typeName[len]='\0';

                    for (int t=0;t<=ImGui::FT_COUNT;t++) {
                        if (strcmp(typeName,FieldTypeNames[t])==0)  {
                            ft = (FieldType) t;break;
                        }
                    }
                    varName = ++colonCh;

                    const bool isTextOrCustomType = ft==ImGui::FT_STRING || ft==ImGui::FT_TEXTLINE  || ft==ImGui::FT_CUSTOM;
                    if (ft==ImGui::FT_COUNT || numArrayElements<1 || (numArrayElements>4 && !isTextOrCustomType))   {
                        fprintf(stderr,"ImGuiHelper::Deserializer::parse(...) Error (wrong type detected): line:%s type:%d numArrayElements:%d varName:%s typeName:%s\n",name,(int)ft,numArrayElements,varName,typeName);
                        varName=NULL;
                    }
                    else {

                        if (ft==ImGui::FT_STRING && varName && varName[0]!='\0')  {
                            if (numArrayElements==1 && (!minusCh || (minusCh-colonCh)>0)) {
                                numArrayElements=0;   // NEW! To handle blank strings ""
                            }
                            //Process soon here, as the string can be multiline
                            line_start = ++line_end;
                            //--------------------------------------------------------
                            int cnt = 0;
                            while (line_end < buf_end && cnt++ < numArrayElements-1) ++line_end;
                            textBuffer[0]=textBuffer[2049]='\0';
                            const int maxLen = numArrayElements>0 ? (cnt>2049?2049:cnt) : 0;
                            strncpy(textBuffer,line_start,maxLen+1);
                            textBuffer[maxLen]='\0';
                            quitParsing = cb(ft,numArrayElements,(void*)textBuffer,varName,userPtr);
                            //fprintf(stderr,"Deserializer::parse(...) value:\"%s\" to type:%d numArrayElements:%d varName:%s maxLen:%d\n",textBuffer,(int)ft,numArrayElements,varName,maxLen);  // dbg


                            //else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
                            //--------------------------------------------------------
                            ft = ImGui::FT_COUNT;name[0]='\0';varName=NULL; // mandatory                            

                        }
                        else if (!isTextOrCustomType) {
                            format[0]='\0';
                            for (int t=0;t<numArrayElements;t++) {
                                if (t>0) strcat(format," ");
                                strcat(format,FieldTypeFormats[ft]);
                            }
                            // DBG:
                            //fprintf(stderr,"Deserializer::parse(...) DBG: line:%s type:%d numArrayElements:%d varName:%s format:%s\n",name,(int)ft,numArrayElements,varName,format);  // dbg
                        }
                    }
                }
            }
        }
        else if (varName && varName[0]!='\0')
        {
            switch (ft) {
            case ImGui::FT_FLOAT:
            case ImGui::FT_COLOR:
            {
                float* p = (float*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_DOUBLE:  {
                double* p = (double*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_INT:
            case ImGui::FT_ENUM:
            {
                int* p = (int*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_BOOL:
            {
                bool* p = (bool*) voidBuffer;
                int tmp[4];
                if ( (numArrayElements==1 && sscanf(line_start, format, &tmp[0])==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &tmp[0],&tmp[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &tmp[0],&tmp[1],&tmp[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &tmp[0],&tmp[1],&tmp[2],&tmp[3])==numArrayElements))    {
                     for (int i=0;i<numArrayElements;i++) p[i] = tmp[i];
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                }
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_UNSIGNED:  {
                unsigned* p = (unsigned*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"Deserializer::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case ImGui::FT_CUSTOM:
            case ImGui::FT_TEXTLINE:
            {
                // A similiar code can be used to parse "numArrayElements" line of text
                for (int i=0;i<numArrayElements;i++)    {
                    textBuffer[0]=textBuffer[2049]='\0';
                    const int maxLen = (line_end-line_start)>2049?2049:(line_end-line_start);
                    if (maxLen<=0) break;
                    strncpy(textBuffer,line_start,maxLen+1);textBuffer[maxLen]='\0';
                    quitParsing = cb(ft,i,(void*)textBuffer,varName,userPtr);

                    //fprintf(stderr,"%d) \"%s\"\n",i,textBuffer);  // Dbg

                    if (quitParsing) break;
                    line_start = line_end+1;
                    line_end = line_start;
                    if (line_end == buf_end) break;
                    while (line_end < buf_end && *line_end != '\n' && *line_end != '\r') line_end++;
                }
            }
            break;
            default:
            fprintf(stderr,"Deserializer::parse(...) Warning missing value type:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            break;
            }
            //---------------------------------------------------------------------------------
            name[0]='\0';varName=NULL; // mandatory
        }

        line_start = line_end+1;

        if (quitParsing) return line_start;
    }

    //------------------------------------------------
    return buf_end;
}

bool GetFileContent(const char *filePath, ImVector<char> &contentOut, bool clearContentOutBeforeUsage, const char *modes, bool appendTrailingZeroIfModesIsNotBinary)   {
    ImVector<char>& f_data = contentOut;
    if (clearContentOutBeforeUsage) f_data.clear();
//----------------------------------------------------
    if (!filePath) return false;
    const bool appendTrailingZero = appendTrailingZeroIfModesIsNotBinary && modes && strlen(modes)>0 && modes[strlen(modes)-1]!='b';
    FILE* f;
    if ((f = ImFileOpen(filePath, modes)) == NULL) return false;
    if (fseek(f, 0, SEEK_END))  {
        fclose(f);
        return false;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)    {
        fclose(f);
        return false;
    }
    size_t f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))  {
        fclose(f);
        return false;
    }
    f_data.resize(f_size+(appendTrailingZero?1:0));
    const size_t f_size_read = f_size>0 ? fread(&f_data[0], 1, f_size, f) : 0;
    fclose(f);
    if (f_size_read == 0 || f_size_read!=f_size)    return false;
    if (appendTrailingZero) f_data[f_size] = '\0';
//----------------------------------------------------
    return true;
}
bool FileExists(const char *filePath)   {
    if (!filePath || strlen(filePath)==0) return false;
    FILE* f = ImFileOpen(filePath, "rb");
    if (!f) return false;
    fclose(f);f=NULL;
    return true;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
bool SetFileContent(const char *filePath, const unsigned char* content, int contentSize,const char* modes)	{
    if (!filePath || !content) return false;
    FILE* f;
    if ((f = ImFileOpen(filePath, modes)) == NULL) return false;
    fwrite(content, contentSize, 1, f);
    fclose(f);f=NULL;
    return true;
}

class ISerializable {
public:
    ISerializable() {}
    virtual ~ISerializable() {}
    virtual void close()=0;
    virtual bool isValid() const=0;
    virtual int print(const char* fmt, ...)=0;
    virtual int getTypeID() const=0;
};
class SerializeToFile : public ISerializable {
public:
    SerializeToFile(const char* filename) : f(NULL) {
        saveToFile(filename);
    }
    SerializeToFile() : f(NULL) {}
    ~SerializeToFile() {close();}
    bool saveToFile(const char* filename) {
        close();
        f = ImFileOpen(filename,"w");
        return (f);
    }
    void close() {if (f) fclose(f);f=NULL;}
    bool isValid() const {return (f);}
    int print(const char* fmt, ...) {
        va_list args;va_start(args, fmt);
        const int rv = vfprintf(f,fmt,args);
        va_end(args);
        return rv;
    }
    int getTypeID() const {return 0;}
protected:
    FILE* f;
};
class SerializeToBuffer : public ISerializable {
public:
    SerializeToBuffer(int initialCapacity=2048) {b.reserve(initialCapacity);b.resize(1);b[0]='\0';}
    ~SerializeToBuffer() {close();}
    bool saveToFile(const char* filename) {
        if (!isValid()) return false;
        return SetFileContent(filename,(unsigned char*)&b[0],b.size(),"w");
    }
    void close() {b.clear();ImVector<char> o;b.swap(o);b.resize(1);b[0]='\0';}
    bool isValid() const {return b.size()>0;}
    int print(const char* fmt, ...) {
        va_list args,args2;
        va_start(args, fmt);
        va_copy(args2,args);                                    // since C99 (MANDATORY! otherwise we must reuse va_start(args2,fmt): slow)
        const int additionalSize = vsnprintf(NULL,0,fmt,args);  // since C99
        va_end(args);
        //IM_ASSERT(additionalSize>0);

        const int startSz = b.size();
        b.resize(startSz+additionalSize);
        const int rv = vsprintf(&b[startSz-1],fmt,args2);
        va_end(args2);
        //IM_ASSERT(additionalSize==rv);
        //IM_ASSERT(v[startSz+additionalSize-1]=='\0');

        return rv;
    }
    inline const char* getBuffer() const {return b.size()>0 ? &b[0] : NULL;}
    inline int getBufferSize() const {return b.size();}
    int getTypeID() const {return 1;}
protected:
    ImVector<char> b;
};
const char* Serializer::getBuffer() const   {
    return (f && f->getTypeID()==1 && f->isValid()) ? static_cast<SerializeToBuffer*>(f)->getBuffer() : NULL;
}
int Serializer::getBufferSize() const {
    return (f && f->getTypeID()==1 && f->isValid()) ? static_cast<SerializeToBuffer*>(f)->getBufferSize() : 0;
}
bool Serializer::WriteBufferToFile(const char* filename,const char* buffer,int bufferSize)   {
    if (!buffer) return false;
    FILE* f = ImFileOpen(filename,"w");
    if (!f) return false;
    fwrite((void*) buffer,bufferSize,1,f);
    fclose(f);
    return true;
}

void Serializer::clear() {if (f) {f->close();}}
Serializer::Serializer(const char *filename) {
    f=(SerializeToFile*) ImGui::MemAlloc(sizeof(SerializeToFile));
    IM_PLACEMENT_NEW((SerializeToFile*)f) SerializeToFile(filename);
}
Serializer::Serializer(int memoryBufferCapacity) {
    f=(SerializeToBuffer*) ImGui::MemAlloc(sizeof(SerializeToBuffer));
    IM_PLACEMENT_NEW((SerializeToBuffer*)f) SerializeToBuffer(memoryBufferCapacity);
}
Serializer::~Serializer() {
    if (f) {
        f->~ISerializable();
        ImGui::MemFree(f);
        f=NULL;
    }
}
template <typename T> inline static bool SaveTemplate(ISerializable* f,FieldType ft, const T* pValue, const char* name, int numArrayElements=1, int prec=-1)   {
    if (!f || ft==ImGui::FT_COUNT  || ft==ImGui::FT_CUSTOM || numArrayElements<0 || numArrayElements>4 || !pValue || !name || name[0]=='\0') return false;
    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);
    // value
    const char* precision = FieldTypeFormatsWithCustomPrecision[ft];
    for (int t=0;t<numArrayElements;t++) {
        if (t>0) f->print(" ");
        f->print(precision,prec,pValue[t]);
    }
    f->print("\n\n");
    return true;
}
bool Serializer::save(FieldType ft, const float* pValue, const char* name, int numArrayElements,  int prec)   {
    IM_ASSERT(ft==ImGui::FT_FLOAT || ft==ImGui::FT_COLOR);
    return SaveTemplate<float>(f,ft,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const double* pValue,const char* name,int numArrayElements, int prec)   {
    return SaveTemplate<double>(f,ImGui::FT_DOUBLE,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const bool* pValue,const char* name,int numArrayElements)   {
    if (!pValue || numArrayElements<0 || numArrayElements>4) return false;
    static int tmp[4];
    for (int i=0;i<numArrayElements;i++) tmp[i] = pValue[i] ? 1 : 0;
    return SaveTemplate<int>(f,ImGui::FT_BOOL,tmp,name,numArrayElements);
}
bool Serializer::save(FieldType ft,const int* pValue,const char* name,int numArrayElements, int prec) {
    IM_ASSERT(ft==ImGui::FT_INT || ft==ImGui::FT_BOOL || ft==ImGui::FT_ENUM);
    if (prec==0) prec=-1;
    return SaveTemplate<int>(f,ft,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const unsigned* pValue,const char* name,int numArrayElements, int prec) {
    if (prec==0) prec=-1;
    return SaveTemplate<unsigned>(f,ImGui::FT_UNSIGNED,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const char* pValue,const char* name,int pValueSize)    {
    FieldType ft = ImGui::FT_STRING;
    int numArrayElements = pValueSize;
    if (!f || ft==ImGui::FT_COUNT || !pValue || !name || name[0]=='\0') return false;
    numArrayElements = pValueSize;
    pValueSize=(int)strlen(pValue);if (numArrayElements>pValueSize || numArrayElements<=0) numArrayElements=pValueSize;
    if (numArrayElements<0) numArrayElements=0;

    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);
    // value
    f->print("%s\n\n",pValue);
    return true;
}
bool Serializer::saveTextLines(const char* pValue,const char* name)   {
    FieldType ft = ImGui::FT_TEXTLINE;
    if (!f || ft==ImGui::FT_COUNT || !pValue || !name || name[0]=='\0') return false;
    const char *tmp;const char *start = pValue;
    int left = strlen(pValue);int numArrayElements =0;  // numLines
    bool endsWithNewLine = pValue[left-1]=='\n';
    while ((tmp=strchr(start, '\n'))) {
        ++numArrayElements;
        left-=tmp-start-1;
        start = ++tmp;  // to skip '\n'
    }
    if (left>0) ++numArrayElements;
    if (numArrayElements==0) return false;

    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);
    // value
    f->print("%s",pValue);
    if (!endsWithNewLine)  f->print("\n");
    f->print("\n");
    return true;
}
bool Serializer::saveTextLines(int numValues,bool (*items_getter)(void* data, int idx, const char** out_text),void* data,const char* name)  {
    FieldType ft = ImGui::FT_TEXTLINE;
    if (!items_getter || !f || ft==ImGui::FT_COUNT || numValues<=0 || !name || name[0]=='\0') return false;
    int numArrayElements =numValues;  // numLines

    // name
    f->print( "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) f->print( "-%d",numArrayElements);
    f->print( ":%s]\n",name);

    // value
    const char* text=NULL;int len=0;
    for (int i=0;i<numArrayElements;i++)    {
        if (items_getter(data,i,&text)) {
            f->print("%s",text);
            if (len<=0 || text[len-1]!='\n')  f->print("\n");
        }
        else f->print("\n");
    }
    f->print("\n");
    return true;
}
bool Serializer::saveCustomFieldTypeHeader(const char* name, int numTextLines) {
    // name
    f->print( "[%s",FieldTypeNames[ImGui::FT_CUSTOM]);
    if (numTextLines==0) numTextLines=1;
    if (numTextLines>1) f->print( "-%d",numTextLines);
    f->print( ":%s]\n",name);
    return true;
}

#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE

void StringSet(char *&destText, const char *text, bool allowNullDestText) {
    if (destText) {ImGui::MemFree(destText);destText=NULL;}
    const char e = '\0';
    if (!text && !allowNullDestText) text=&e;
    if (text)  {
        const int sz = strlen(text);
        destText = (char*) ImGui::MemAlloc(sz+1);strcpy(destText,text);
    }
}
void StringAppend(char *&destText, const char *textToAppend, bool allowNullDestText, bool prependLineFeedIfDestTextIsNotEmpty, bool mustAppendLineFeed) {
    const int textToAppendSz = textToAppend ? strlen(textToAppend) : 0;
    if (textToAppendSz==0) {
        if (!destText && !allowNullDestText) {destText = (char*) ImGui::MemAlloc(1);strcpy(destText,"");}
        return;
    }
    const int destTextSz = destText ? strlen(destText) : 0;
    const bool mustPrependLF = prependLineFeedIfDestTextIsNotEmpty && (destTextSz>0);
    const bool mustAppendLF = mustAppendLineFeed;// && (destText);
    const int totalTextSz = textToAppendSz + destTextSz + (mustPrependLF?1:0) + (mustAppendLF?1:0);
    ImVector<char> totalText;totalText.resize(totalTextSz+1);
    totalText[0]='\0';
    if (destText) {
        strcpy(&totalText[0],destText);
        ImGui::MemFree(destText);destText=NULL;
    }
    if (mustPrependLF) strcat(&totalText[0],"\n");
    strcat(&totalText[0],textToAppend);
    if (mustAppendLF) strcat(&totalText[0],"\n");
    destText = (char*) ImGui::MemAlloc(totalTextSz+1);strcpy(destText,&totalText[0]);
}
int StringAppend(ImVector<char>& v,const char* fmt, ...) {
    IM_ASSERT(v.size()>0 && v[v.size()-1]=='\0');
    va_list args,args2;

    va_start(args, fmt);
    va_copy(args2,args);                                    // since C99 (MANDATORY! otherwise we must reuse va_start(args2,fmt): slow)
    const int additionalSize = vsnprintf(NULL,0,fmt,args);  // since C99
    va_end(args);

    const int startSz = v.size();
    v.resize(startSz+additionalSize);
    const int rv = vsprintf(&v[startSz-1],fmt,args2);
    va_end(args2);

    return rv;
}


} //namespace ImGuiHelper
#endif //NO_IMGUIHELPER_SERIALIZATION


#ifdef IMGUI_USE_ZLIB	// requires linking to library -lZlib
#include <zlib.h>

namespace ImGui {

#ifndef NO_IMGUIHELPER_SERIALIZATION
#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
bool GzDecompressFromFile(const char* filePath,ImVector<char>& rv,bool clearRvBeforeUsage)   {
    if (clearRvBeforeUsage) rv.clear();
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"rb",false)) return false;
    //----------------------------------------------------
    return GzDecompressFromMemory(&f_data[0],f_data.size(),rv,clearRvBeforeUsage);
    //----------------------------------------------------
}
#   ifdef YES_IMGUISTRINGIFIER
bool GzBase64DecompressFromFile(const char* filePath,ImVector<char>& rv)    {
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::GzBase64DecompressFromMemory(&f_data[0],rv);
}
bool GzBase85DecompressFromFile(const char* filePath,ImVector<char>& rv)    {
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::GzBase85DecompressFromMemory(&f_data[0],rv);
}
#   endif //#YES_IMGUISTRINGIFIER
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#endif //NO_IMGUIHELPER_SERIALIZATION

bool GzDecompressFromMemory(const char* memoryBuffer,int memoryBufferSize,ImVector<char>& rv,bool clearRvBeforeUsage)    {
    if (clearRvBeforeUsage) rv.clear();
    const int startRv = rv.size();

    if (memoryBufferSize == 0  || !memoryBuffer) return false;
    const int memoryChunk = memoryBufferSize > (16*1024) ? (16*1024) : memoryBufferSize;
    rv.resize(startRv+memoryChunk);  // we start using the memoryChunk length

    z_stream myZStream;
    myZStream.next_in = (Bytef *) memoryBuffer;
    myZStream.avail_in = memoryBufferSize;
    myZStream.total_out = 0;
    myZStream.zalloc = Z_NULL;
    myZStream.zfree = Z_NULL;

    bool done = false;
    if (inflateInit2(&myZStream, (16+MAX_WBITS)) == Z_OK) {
        int err = Z_OK;
        while (!done) {
            if (myZStream.total_out >= (uLong)(rv.size()-startRv)) rv.resize(rv.size()+memoryChunk);    // not enough space: we add the memoryChunk each step

            myZStream.next_out = (Bytef *) (&rv[startRv] + myZStream.total_out);
            myZStream.avail_out = rv.size() - startRv - myZStream.total_out;

            if ((err = inflate (&myZStream, Z_SYNC_FLUSH))==Z_STREAM_END) done = true;
            else if (err != Z_OK)  break;
        }
        if ((err=inflateEnd(&myZStream))!= Z_OK) done = false;
    }
    rv.resize(startRv+(done ? myZStream.total_out : 0));

    return done;
}
bool GzCompressFromMemory(const char* memoryBuffer,int memoryBufferSize,ImVector<char>& rv,bool clearRvBeforeUsage)  {
    if (clearRvBeforeUsage) rv.clear();
    const int startRv = rv.size();

    if (memoryBufferSize == 0  || !memoryBuffer) return false;
    const int memoryChunk = memoryBufferSize/3 > (16*1024) ? (16*1024) : memoryBufferSize/3;
    rv.resize(startRv+memoryChunk);  // we start using the memoryChunk length

    z_stream myZStream;
    myZStream.next_in =  (Bytef *) memoryBuffer;
    myZStream.avail_in = memoryBufferSize;
    myZStream.total_out = 0;
    myZStream.zalloc = Z_NULL;
    myZStream.zfree = Z_NULL;

    bool done = false;
    if (deflateInit2(&myZStream,Z_BEST_COMPRESSION,Z_DEFLATED,(16+MAX_WBITS),8,Z_DEFAULT_STRATEGY) == Z_OK) {
        int err = Z_OK;
        while (!done) {
            if (myZStream.total_out >= (uLong)(rv.size()-startRv)) rv.resize(rv.size()+memoryChunk);    // not enough space: we add the full memoryChunk each step

            myZStream.next_out = (Bytef *) (&rv[startRv] + myZStream.total_out);
            myZStream.avail_out = rv.size() - startRv - myZStream.total_out;

            if ((err = deflate (&myZStream, Z_FINISH))==Z_STREAM_END) done = true;
            else if (err != Z_OK)  break;
        }
        if ((err=deflateEnd(&myZStream))!= Z_OK) done=false;
    }
    rv.resize(startRv+(done ? myZStream.total_out : 0));

    return done;
}
#   ifdef YES_IMGUISTRINGIFIER
bool GzBase64DecompressFromMemory(const char* input,ImVector<char>& rv) {
    rv.clear();ImVector<char> v;
    if (ImGui::Base64Decode(input,v)) return false;
    if (v.size()==0) return false;
    return GzDecompressFromMemory(&v[0],v.size(),rv);
}
bool GzBase85DecompressFromMemory(const char* input,ImVector<char>& rv) {
    rv.clear();ImVector<char> v;
    if (ImGui::Base85Decode(input,v)) return false;
    if (v.size()==0) return false;
    return GzDecompressFromMemory(&v[0],v.size(),rv);
}
bool GzBase64CompressFromMemory(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode,int numCharsPerLineInStringifiedMode)   {
    output.clear();ImVector<char> output1;
    if (!ImGui::GzCompressFromMemory(input,inputSize,output1)) return false;
    return ImGui::Base64Encode(&output1[0],output1.size(),output,stringifiedMode,numCharsPerLineInStringifiedMode);
}
bool GzBase85CompressFromMemory(const char* input,int inputSize,ImVector<char>& output,bool stringifiedMode,int numCharsPerLineInStringifiedMode) {
    output.clear();ImVector<char> output1;
    if (!ImGui::GzCompressFromMemory(input,inputSize,output1)) return false;
    return ImGui::Base85Encode(&output1[0],output1.size(),output,stringifiedMode,numCharsPerLineInStringifiedMode);
}
#   endif //#YES_IMGUISTRINGIFIER

} // namespace ImGui
#endif //IMGUI_USE_ZLIB

#   ifdef YES_IMGUIBZ2
//#include "../imguiyesaddons/imguibz2.h"   // This should be already included
namespace ImGui {
// Two methods that fill rv and return true on success
#       ifndef NO_IMGUIHELPER_SERIALIZATION
#           ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
bool Bz2DecompressFromFile(const char* filePath,ImVector<char>& rv,bool clearRvBeforeUsage) {
    if (clearRvBeforeUsage) rv.clear();
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"rb",false)) return false;
    //----------------------------------------------------
    return ImGui::Bz2DecompressFromMemory(&f_data[0],f_data.size(),rv,clearRvBeforeUsage);
    //----------------------------------------------------
}
#   ifdef YES_IMGUISTRINGIFIER
bool Bz2Base64DecompressFromFile(const char* filePath,ImVector<char>& rv)   {
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::Bz2Base64Decode(&f_data[0],rv);
}
bool Bz2Base85DecompressFromFile(const char* filePath, ImVector<char>& rv)   {
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::Bz2Base85Decode(&f_data[0],rv);
}
#   endif //#YES_IMGUISTRINGIFIER
#           endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#       endif //NO_IMGUIHELPER_SERIALIZATION
} // namespace ImGui
#   endif //YES_IMGUIBZ2

#   ifdef YES_IMGUISTRINGIFIER
namespace ImGui {
// Two methods that fill rv and return true on success
#       ifndef NO_IMGUIHELPER_SERIALIZATION
#           ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
bool Base64DecodeFromFile(const char* filePath,ImVector<char>& rv)  {
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::Base64Decode(&f_data[0],rv);
}
bool Base85DecodeFromFile(const char* filePath,ImVector<char>& rv)  {
    ImVector<char> f_data;
    if (!ImGuiHelper::GetFileContent(filePath,f_data,true,"r",true)) return false;
    return ImGui::Base85Decode(&f_data[0],rv);
}
#           endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#       endif //NO_IMGUIHELPER_SERIALIZATION
} // namespace ImGui
#   endif //YES_IMGUISTRINGIFIER
