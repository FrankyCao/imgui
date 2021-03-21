#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <windows.h>
#include <tchar.h>
#include <vector>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include "application.h"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
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

static void * user_handle = nullptr;

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

# if defined(_UNICODE)
std::wstring widen(const std::string& str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), (wchar_t*)result.data(), size);
    return result;
}
# endif

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    const auto c_ClassName  = _T("Imgui Application Class");
# if defined(_UNICODE)
    const std::wstring c_WindowName = widen(Application_GetName(user_handle));
# else
    const std::string c_WindowName = Application_GetName(user_handle) + std::string(" Win32_GL3");
# endif

# if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
# endif
    // Create application window
    const auto wc = WNDCLASSEX{ sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), LoadIcon(GetModuleHandle(nullptr), IDI_APPLICATION),
        LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, c_ClassName, LoadIcon(GetModuleHandle(nullptr), IDI_APPLICATION) };
    RegisterClassEx(&wc);

    int window_width = 1440;
    int window_height = 960;
    float window_scale = 1;

    auto hwnd = CreateWindow(c_ClassName, c_WindowName.c_str(), WS_OVERLAPPEDWINDOW, 100, 100, window_width, window_height, nullptr, nullptr, wc.hInstance, nullptr);
    if (hwnd == nullptr)
    {
        fprintf(stderr, "Failed to Open window! %s\n", c_WindowName.c_str());
        return 1;
    }
    auto dc = GetDC(hwnd);
    if (dc == nullptr)
    {
        fprintf(stderr, "Failed to get window DC! %s\n", c_WindowName.c_str());
        return 1;
    }
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    auto visual = ChoosePixelFormat(dc, &pfd);
    if (0 == visual)
    {
        fprintf(stderr, "Failed to Choose PixelFormat! %s\n", c_WindowName.c_str());
        return 1;
    }
    if (FALSE == SetPixelFormat(dc, visual, &pfd))
    {
        fprintf(stderr, "Failed to Set PixelFormat! %s\n", c_WindowName.c_str());
        return 1;
    }
    auto rc = wglCreateContext(dc);
    if (rc == nullptr)
    {
        fprintf(stderr, "Failed to create context! %s\n", c_WindowName.c_str());
        return 1;
    }
    if (FALSE == wglMakeCurrent(dc, rc))
    {
        fprintf(stderr, "Failed to Make Current context! %s\n", c_WindowName.c_str());
        return 1;
    }
    
    const char* glsl_version = "#version 130";

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader! %s (%d)\n", c_WindowName.c_str(), err);
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = window_scale;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.f);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    Application_Initialize(&user_handle);

    // Main loop
    bool done = false;
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        ImGui_ImplWin32_WaitForEvent();
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        done = Application_Frame(user_handle);
        if (done)
            ::PostQuitMessage(0);

        ImGui::EndFrame();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(dc);
    }

    Application_Finalize(&user_handle);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(wglGetCurrentContext());
    ReleaseDC(hwnd, dc);
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
