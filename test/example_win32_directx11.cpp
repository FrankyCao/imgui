// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <string>
#ifdef IMGUI_ADDONS
#include "implot.h"
#include "imgui_markdown.h"
#include "imgui_node_editor.h"
#include "imgui_memory_editor.h"
#include "imnodes.h"
#include "ImGuiNodeGraphEditor.h"
#include "TextEditor.h"
#include "ImGuiFileDialog.h"
#include "ImGuiFileSystem.h"
#include "HotKey.h"
#include "addon/addons_demo.h"
#endif
#include "Config.h"

// Data
ID3D11Device*                   g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef IMGUI_ADDONS
static std::string get_file_contents(const char *filename)
{
    std::ifstream infile(filename, std::ios::in | std::ios::binary);
    if (infile.is_open())
    {
        std::ostringstream contents;
        contents << infile.rdbuf();
        infile.close();
        return(contents.str());
    }
    throw(errno);
}

inline ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    // In your application you would load an image based on data_ input. Here we just use the imgui font texture.
    ImTextureID image = ImGui::GetIO().Fonts->TexID;
    // > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image, ImVec2( 40.0f, 20.0f ) };
    ImGui::MarkdownImageData imageData;
    imageData.isValid =         true;
    imageData.useLinkCallback = false;
    imageData.user_texture_id = image;
    imageData.size =            ImVec2( 40.0f, 20.0f );
    return imageData;
}

static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    std::string url( data_.link, data_.linkLength );
    std::string command = "open " + url;
    if( !data_.isImage )
    {
        system(command.c_str());
    }
}

static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ )
{
    // Call the default first so any settings can be overwritten by our implementation.
    // Alternatively could be called or not called in a switch statement on a case by case basis.
    // See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
    ImGui::defaultMarkdownFormatCallback( markdownFormatInfo_, start_ );        
    switch( markdownFormatInfo_.type )
    {
        // example: change the colour of heading level 2
        case ImGui::MarkdownFormatType::HEADING:
        {
            if( markdownFormatInfo_.level == 2 )
            {
                if( start_ )
                {
                    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] );
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
#endif

// Main code
int main(int, char**)
{
# if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
# endif
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX11 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1440, 1024, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
#ifdef IMGUI_ADDONS
    ImPlot::CreateContext();
#endif
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "win32_directx11.ini";
    io.IniFilename = ini_file.c_str();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

#ifdef IMGUI_ADDONS
    // load file dialog resource
    ImGuiFileDialog filedialog;
    std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
    prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

    // init sample file dialog
    ImGuiFs::Dialog dlg;

    // init memory edit
    MemoryEditor mem_edit;
    mem_edit.Open = false;
    mem_edit.OptShowDataPreview = true;
    size_t data_size = 0x1000;
    void* data = malloc(data_size);

    // Init Text Edit
	TextEditor editor;

    // Init MarkDown
    ImGui::MarkdownConfig mdConfig; 

    // Init imnodes
    std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.ini";
    std::string node_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.node";
    imnodes::CreateContext();
    imnodes_sample::NodeEditorInitialize(node_ini_path.c_str(), node_path.c_str());

    // Init NodeGraphEditor
    ImGui::NodeGraphEditor nge;
    std::string nge_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.nge.ini";
    std::string nge_style_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.style.ini";
    nge.save_node_path = nge_ini_path;
    nge.save_style_path = nge_style_path;

    // Init HotKey
    static std::vector<ImHotKey::HotKey> hotkeys = 
    { 
        {"Layout", "Reorder nodes in a simpler layout", 0xFFFF261D},
        {"Save", "Save the current graph", 0xFFFF1F1D},
        {"Load", "Load an existing graph file", 0xFFFF181D},
        {"Play/Stop", "Play or stop the animation from the current graph", 0xFFFFFF3F},
        {"SetKey", "Make a new animation key with the current parameters values at the current time", 0xFFFFFF1F}
    };
#endif

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
#ifdef IMGUI_ADDONS
    bool show_implot_window = false;
    bool show_file_dialog_window = false;
    bool show_sample_file_dialog = false;
    bool show_text_edit_window = false;
    bool show_markdown_window = false;
    bool show_dock_window = false;
    bool show_tab_window = false;
    bool show_node_window = false;
    bool show_node_edit_window = false;
    bool show_addon_widget = false;
    bool show_zmo_window = false;
#endif
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
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
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
#ifdef IMGUI_ADDONS
            ImGui::Checkbox("ImPlot Window", &show_implot_window);
            ImGui::Checkbox("File Dialog Window", &show_file_dialog_window);
            ImGui::Checkbox("Sample File Dialog", &show_sample_file_dialog);
            ImGui::Checkbox("Memory Edit Window", &mem_edit.Open);
            ImGui::Checkbox("Show Text Edit Window", &show_text_edit_window);
            ImGui::Checkbox("Show Markdown Window", &show_markdown_window);
            ImGui::Checkbox("Show Dock Window", &show_dock_window);
            ImGui::Checkbox("Show Tab Window", &show_tab_window);
            ImGui::Checkbox("Show Node Window", &show_node_window);
            ImGui::Checkbox("Show Node Edit Windows", &show_node_edit_window);
            ImGui::Checkbox("Show Addon Widgets", &show_addon_widget);
            ImGui::Checkbox("Show ImGuizmo Window", &show_zmo_window);

            // show hotkey window
            if (ImGui::Button("Edit Hotkeys"))
            {
                ImGui::OpenPopup("HotKeys Editor");
            }

            // Handle hotkey popup
            ImHotKey::Edit(hotkeys.data(), hotkeys.size(), "HotKeys Editor");
            int hotkey = ImHotKey::GetHotKey(hotkeys.data(), hotkeys.size());
            if (hotkey != -1)
            {
                // handle the hotkey index!
            }
#endif
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

#ifdef IMGUI_ADDONS
        // 4. Show ImPlot simple window
        if (show_implot_window)
        {
            ImPlot::ShowDemoWindow(&show_implot_window);
        }

        // 5. Show FileDialog demo window
        if (show_file_dialog_window)
        {
            show_file_dialog_demo_window(&filedialog, &show_file_dialog_window);
        }

        // 6. Show Sample FileDialog
        {
            // dlg.WrapMode = false;
            const char* filePath = dlg.chooseFileDialog(show_sample_file_dialog, dlg.getLastDirectory(), ".jpg;.jpeg;.png;.gif;.tga;.bmp", "Sample file dialog", ImVec2(400, 800), ImVec2(50, 50));
            if (strlen(filePath) > 0) 
            {
	            //fprintf(stderr,"Browsed..: %s\n",filePath);
            }
            show_sample_file_dialog = false;
        }

        // 7. Show Memory Edit window
        if (mem_edit.Open)
        {
            ImGui::Begin("Memory Window", &mem_edit.Open);
            mem_edit.DrawWindow("Memory Editor", data, data_size);
            ImGui::End();
        }

        // 8. Show Text Edit Window
        if (show_text_edit_window)
        {
            editor.text_edit_demo(&show_text_edit_window);
        }

        // 9. Show Markdown Window
        if (show_markdown_window)
        {
            std::string help_doc = get_file_contents("docs/imgui.md");
            mdConfig.linkCallback =         LinkCallback;
            mdConfig.tooltipCallback =      NULL;
            mdConfig.imageCallback =        ImageCallback;
            mdConfig.linkIcon =             ICON_FA5_LINK;
            mdConfig.headingFormats[0] =    { io.Fonts->Fonts[0], true };
            mdConfig.headingFormats[1] =    { nullptr, true };
            mdConfig.headingFormats[2] =    { nullptr, false };
            mdConfig.userData =             NULL;
            mdConfig.formatCallback =       ExampleMarkdownFormatCallback;
            ImGui::Markdown( help_doc.c_str(), help_doc.length(), mdConfig );
        }

        // 10. Show Dock Window
        if (show_dock_window)
        {
            ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
            if(ImGui::Begin("imguidock window (= lumix engine's dock system)",&show_dock_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowAddonsDuckWindow();
            }
            ImGui::End();
        }

        // 11. Show Tab Window
        if (show_tab_window)
        {
            ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: TabWindow", &show_tab_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowAddonsTabWindow();   // see its code for further info         
            }
            ImGui::End();
        }

        // 12. Show Node Window
        if (show_node_window)
        {
            imnodes_sample::NodeEditorShow();
        }

        // 13. Show Node Edit Window
        if (show_node_edit_window)
        {
            ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: Custom Node Graph",&show_node_edit_window, ImGuiWindowFlags_NoScrollbar))
            {
                std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.nge.ini";
                std::string node_style_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.style.ini";
                ImGui::TestNodeGraphEditor(&nge);   // see its code for further info         
            }
            ImGui::End();
        }

        // 14. Show Addon Widget.
        if (show_addon_widget)
        {
            ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Addon Widget", &show_addon_widget);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::ShowAddonsDemoWindowWidgets();
            ImGui::End();
        }

        // 15. Show Zmo Window
        if (show_zmo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
            ImGui::Begin("##ZMO", &show_zmo_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
            ImGui::ShowAddonsZMOWindow();
            ImGui::End();
        }
#endif
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

#ifdef IMGUI_ADDONS
    // Store file dialog bookmark
    end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

    // Cleanup memory edit resource
    if (data)
        free(data);

    // Clean Node Window
    imnodes_sample::NodeEditorShutdown(node_ini_path.c_str(), node_path.c_str());
    imnodes::DestroyContext();

    // Cleanup Demo
    ImGui::CleanupDemo();
    ImGui::CleanupZMODemo();
    nge.clear();
#endif
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
#ifdef IMGUI_ADDONS
    ImPlot::DestroyContext();
#endif
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
