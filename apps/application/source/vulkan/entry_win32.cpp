#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <mutex>
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"
#include "application.h"

#include "entry_vulkan.h"

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
                ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), g_MinImageCount);
                g_MainWindowData.FrameIndex = 0;
                g_SwapChainRebuild = false;
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
    const auto c_ClassName  = _T("Imgui Application Class");
# if defined(_UNICODE)
    const std::wstring c_WindowName = widen(Application_GetName(user_handle));
# else
    const std::string c_WindowName = Application_GetName(user_handle) + std::string(" Win32");
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

    // Setup Vulkan
    std::vector<const char *> instance_extensions;
    instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    uint32_t extensions_count = instance_extensions.size();
    const char** extensions = instance_extensions.data();
    SetupVulkan(extensions, extensions_count);
    
    // Create Window Surface
    VkSurfaceKHR surface;
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = wc.hInstance;
    surface_create_info.hwnd = hwnd;
    VkResult err = vkCreateWin32SurfaceKHR(g_Instance, &surface_create_info, nullptr, &surface);
    check_vk_result(err);

    // Create Framebuffers
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, window_width, window_height);

    // Setup ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.Allocator = g_Allocator;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.CheckVkResultFn = check_vk_result;
    // Setup ImGui binding
    ImGui_ImplVulkan_Init(std::make_shared<ImGui_ImplVulkan_InitInfo>(init_info), wd->RenderPass);

    UpdateVulkanFont(wd);

    Application_Initialize(&user_handle);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Main loop
    bool done = false;
    while (!done)
    {
        ImGui_ImplWin32_WaitForEvent();
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        done = Application_Frame(user_handle);
        if (done)
            ::PostQuitMessage(0);

        ImGui::EndFrame();
        // Rendering
        ImGui::Render();
        FrameRendering(wd);
    }

    Application_Finalize(&user_handle);

    // Cleanup
    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow();
    CleanupVulkan();

    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}