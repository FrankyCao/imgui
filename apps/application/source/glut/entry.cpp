#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/freeglut.h>
#endif
#include "application.h"

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed
#endif

#define STB_IMAGE_IMPLEMENTATION
extern "C" {
#include "stb_image.h"
}

ImTextureID Application_LoadTexture(const char* path)
{
    int width = 0, height = 0, component = 0;
    if (auto data = stbi_load(path, &width, &height, &component, 4))
    {
        auto texture = Application_CreateTexture(data, width, height);
        stbi_image_free(data);
        return texture;
    }
    else
        return nullptr;
}

struct ImTexture
{
    GLuint TextureID = 0;
    int    Width     = 0;
    int    Height    = 0;
};

static std::vector<ImTexture> g_Textures;
static void * user_handle = nullptr;
static ImVec4 clear_color = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);

ImTextureID Application_CreateTexture(const void* data, int width, int height)
{
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

    return reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(texture.TextureID));
}

static std::vector<ImTexture>::iterator Application_FindTexture(ImTextureID texture)
{
    auto textureID = static_cast<GLuint>(reinterpret_cast<std::intptr_t>(texture));

    return std::find_if(g_Textures.begin(), g_Textures.end(), [textureID](ImTexture& texture)
    {
        return texture.TextureID == textureID;
    });
}

void Application_DestroyTexture(ImTextureID texture)
{
    auto textureIt = Application_FindTexture(texture);
    if (textureIt == g_Textures.end())
        return;

    glDeleteTextures(1, &textureIt->TextureID);

    g_Textures.erase(textureIt);
}

int Application_GetTextureWidth(ImTextureID texture)
{
    auto textureIt = Application_FindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Width;
    return 0;
}

int Application_GetTextureHeight(ImTextureID texture)
{
    auto textureIt = Application_FindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Height;
    return 0;
}

void glut_display_func()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Content", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

    Application_Frame(user_handle);

    ImGui::End();
    // Rendering
    ImGui::Render();
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
        // Create GLUT window
    glutInit(&argc, argv);
#ifdef __FREEGLUT_EXT_H__
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
    int window_width = 1440;
    int window_height = 960;
    float window_scale = 1;
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(window_width, window_height);
    std::string title = Application_GetName(user_handle);
    title += " GLUT";
    glutCreateWindow(title.c_str());
    // Setup GLUT display function
    // We will also call ImGui_ImplGLUT_InstallFuncs() to get all the other functions installed for us,
    // otherwise it is possible to install our own functions and call the imgui_impl_glut.h functions ourselves.
    glutDisplayFunc(glut_display_func);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    std::string ini_file = std::string(Application_GetName()) + ".ini";
    io.IniFilename = ini_file.c_str();
    io.FontGlobalScale = window_scale;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL2_Init();

    // init application
    Application_Initialize(&user_handle);

    glutMainLoop();

    // Cleanup
    Application_Finalize(&user_handle);

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    return 0;
}
